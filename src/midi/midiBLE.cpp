#include "midiBLE.h"

#ifdef INCLUDE_BLE

#include <NimBLEDevice.h>
#include <atomic>
#include "midi/ble_midi_codec.h"

// BLE-MIDI peripheral on NimBLE-Arduino 2.x. See docs/ble-midi.md.
// - Producers (engine, Core 1) only push into s_outQ under a spinlock.
// - bleMidiTask (Core 0, next to the NimBLE host) drains it once per
//   connection interval, as long as the host has buffers to spare.
// - Inbound packets are parsed in the NimBLE host task; Program Changes go
//   to a small static queue read with midiBLEReadProgramChange().

extern "C" int os_msys_num_free(void);

#ifdef PIPO_MOTION
static const char* kDeviceName = "PipoMotionBLE";
#elif PIPO_RANGE
static const char* kDeviceName = "PipoRangeBLE";
#elif PIPO_ANALOG
static const char* kDeviceName = "PipoAnalogBLE";
#endif

static const char* kMidiServiceUuid = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
static const char* kMidiCharUuid = "7772E5DB-3868-4112-A1A9-F2669D106BF3";

// ATT MTU 128 -> 125-byte packets, each fits in one 256-byte NimBLE mbuf
static const uint16_t kPreferredMtu = kBleMidiMaxPacket + 3;
static const uint16_t kDefaultPayload = 20;  // ATT MTU 23 until the exchange

// Connection parameters, interval in 1.25 ms units, timeout in 10 ms units
static const uint16_t kItvlMin = 6;              // 7.5 ms
static const uint16_t kItvlMax = 12;             // 15 ms
static const uint16_t kAppleItvl = 12;           // 15/15 ms is Apple compliant
static const uint16_t kSupervisionTimeout = 300; // 3 s
static const uint32_t kFallbackAfterMs = 3000;

static const int kMinFreeMbufs = 8;  // headroom left for the host's own traffic
static const int kMaxPacketsPerTick = 3;
static const size_t kMaxBatchMsgs = kBleMidiMaxPacket / 3;  // 2-byte msg + ts

static const uint32_t kSenderStack = 3072;
static const UBaseType_t kSenderPriority = 2;
static const BaseType_t kSenderCore = 0;  // NimBLE host runs on core 0
static const UBaseType_t kInQueueLen = 8;

struct BleMidiProgramChange {
  uint8_t channel;  // 1-16
  uint8_t program;
};

static NimBLEServer* s_server = nullptr;
static NimBLECharacteristic* s_chr = nullptr;
static TaskHandle_t s_senderTask = nullptr;

static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
static BleMidiOutQueue s_outQ;  // guarded by s_mux

// Written by the NimBLE host task, read by producers and the sender task
static volatile bool s_ready = false;  // connected and subscribed to notifications
static std::atomic<uint32_t> s_session{0};  // bumped on connect/subscribe/disconnect
static volatile uint16_t s_connHandle = BLE_HS_CONN_HANDLE_NONE;
static volatile uint16_t s_connItvl = kItvlMax;
static volatile uint16_t s_maxPayload = kDefaultPayload;
static volatile uint32_t s_connectedAt = 0;

// Stats for the current connection (logged on disconnect)
static std::atomic<uint32_t> s_sentPackets{0};
static std::atomic<uint32_t> s_notifyFails{0};
static std::atomic<uint32_t> s_busyTicks{0};
static std::atomic<uint32_t> s_droppedIn{0};

static StaticQueue_t s_inQStruct;
static uint8_t s_inQStorage[kInQueueLen * sizeof(BleMidiProgramChange)];
static QueueHandle_t s_inQ = nullptr;

static void resetOutQueue() {
  portENTER_CRITICAL(&s_mux);
  s_outQ.clear();
  portEXIT_CRITICAL(&s_mux);
}

static void wakeSender() {
  if (s_senderTask != nullptr)
    xTaskNotifyGive(s_senderTask);
}

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& info) override {
    s_connHandle = info.getConnHandle();
    s_connItvl = info.getConnInterval();
    s_maxPayload = kDefaultPayload;
    s_connectedAt = millis();
    s_sentPackets = 0;
    s_notifyFails = 0;
    s_busyTicks = 0;
    s_droppedIn = 0;
    portENTER_CRITICAL(&s_mux);
    s_outQ.resetStats();
    portEXIT_CRITICAL(&s_mux);
    s_session++;
    BTconnected = true;

    server->updateConnParams(info.getConnHandle(), kItvlMin, kItvlMax, 0,
                             kSupervisionTimeout);
    log_i("MIDI BLE connected, interval %.2f ms", s_connItvl * 1.25f);
    if (DEBUG_HEAP)
      pipoDebugHeap();
  }

  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& info,
                    int reason) override {
    s_ready = false;
    s_connHandle = BLE_HS_CONN_HANDLE_NONE;
    s_session++;
    resetOutQueue();
    BTconnected = false;
    wakeSender();

    portENTER_CRITICAL(&s_mux);
    BleMidiOutQueue::Stats q = s_outQ.stats();
    portEXIT_CRITICAL(&s_mux);
    log_i("MIDI BLE disconnected (reason 0x%x)", reason);
    log_i(
        "BLE MIDI stats: packets %u, notify fails %u, busy ticks %u, dropped "
        "notes %u, dropped cc %u, evicted note-on %u, dropped in %u, task "
        "stack free %u",
        (unsigned)s_sentPackets, (unsigned)s_notifyFails,
        (unsigned)s_busyTicks, (unsigned)q.droppedOrdered,
        (unsigned)q.droppedLatest, (unsigned)q.evictedNoteOn,
        (unsigned)s_droppedIn,
        s_senderTask ? (unsigned)uxTaskGetStackHighWaterMark(s_senderTask) : 0);
    if (DEBUG_HEAP)
      pipoDebugHeap();
  }

  void onMTUChange(uint16_t mtu, NimBLEConnInfo& info) override {
    uint16_t payload = mtu > 3 ? mtu - 3 : kDefaultPayload;
    s_maxPayload = payload < kBleMidiMaxPacket ? payload : kBleMidiMaxPacket;
    log_i("MIDI BLE MTU %u, packet payload %u", mtu, s_maxPayload);
  }

  void onConnParamsUpdate(NimBLEConnInfo& info) override {
    s_connItvl = info.getConnInterval();
    log_i("MIDI BLE interval %.2f ms, latency %u", s_connItvl * 1.25f,
          info.getConnLatency());
  }
};

class MidiCharCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& info) override {
    NimBLEAttValue value = chr->getValue();
    bleMidiParse(value.data(), value.size(),
                 [](uint8_t status, uint8_t d1, uint8_t d2) {
                   if ((status & 0xF0) != 0xC0)
                     return;
                   BleMidiProgramChange pc = {
                       (uint8_t)((status & 0x0F) + 1), d1};
                   if (xQueueSend(s_inQ, &pc, 0) != pdTRUE)
                     s_droppedIn++;
                 });
    // BLE-MIDI spec: reads return an empty payload
    static const uint8_t empty = 0;
    chr->setValue(&empty, 0);
  }

  void onSubscribe(NimBLECharacteristic* chr, NimBLEConnInfo& info,
                   uint16_t subValue) override {
    bool notify = subValue & 0x0001;
    if (notify)
      resetOutQueue();  // start the session with nothing stale
    s_session++;
    s_ready = notify;
    wakeSender();
    log_i("MIDI BLE notifications %s", notify ? "on" : "off");
  }
};

static ServerCallbacks s_serverCallbacks;
static MidiCharCallbacks s_chrCallbacks;

// Interval in ms, rounded up (1.25 ms units)
static uint32_t connIntervalMs() {
  uint32_t ms = (s_connItvl * 5 + 3) / 4;
  return ms > 0 ? ms : 1;
}

static void bleMidiTask(void*) {
  BleMidiMsg pending[kMaxBatchMsgs];  // batch taken from the queue, not sent yet
  size_t pendingCount = 0;
  uint32_t pendingSession = 0;
  uint32_t fallbackCheckedFor = 0;  // s_connectedAt of the last fallback check
  uint8_t packet[kBleMidiMaxPacket];

  for (;;) {
    if (!s_ready) {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      continue;
    }
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(connIntervalMs()));
    if (!s_ready)
      continue;

    if (pendingSession != s_session) {  // never send a previous session's batch
      pendingCount = 0;
      pendingSession = s_session;
    }

    // Apple rejects 7.5 ms requests: ask for 15/15 ms once if still slower
    uint32_t connectedAt = s_connectedAt;
    if (fallbackCheckedFor != connectedAt &&
        millis() - connectedAt > kFallbackAfterMs) {
      fallbackCheckedFor = connectedAt;
      if (s_connItvl > kItvlMax) {
        // integer formatting: float printf is heavy for this small stack
        log_i("MIDI BLE interval %u units of 1.25 ms, requesting 15 ms",
              (unsigned)s_connItvl);
        s_server->updateConnParams(s_connHandle, kAppleItvl, kAppleItvl, 0,
                                   kSupervisionTimeout);
      }
    }

    for (int p = 0; p < kMaxPacketsPerTick; p++) {
      if (pendingCount == 0) {
        portENTER_CRITICAL(&s_mux);
        pendingCount = s_outQ.takeBatch(pending, kMaxBatchMsgs, s_maxPayload);
        portEXIT_CRITICAL(&s_mux);
        if (pendingCount == 0)
          break;
      }
      // Host queue backing up: the link is slower than we produce. Keep the
      // batch, let the queue coalesce, retry next interval.
      if (os_msys_num_free() < kMinFreeMbufs) {
        s_busyTicks++;
        break;
      }
      size_t len = bleMidiEncode(pending, pendingCount, millis() & 0x1FFF,
                                 packet, sizeof(packet));
      if (len == 0) {  // cannot happen: batch was sized for the packet
        pendingCount = 0;
        continue;
      }
      if (!s_chr->notify(packet, len, s_connHandle)) {
        s_notifyFails++;
        break;  // keep the batch for the next interval
      }
      pendingCount = 0;
      s_sentPackets++;
    }
  }
}

void midiBLESetup() {
  s_inQ = xQueueCreateStatic(kInQueueLen, sizeof(BleMidiProgramChange),
                             s_inQStorage, &s_inQStruct);

  NimBLEDevice::init(kDeviceName);
  NimBLEDevice::setMTU(kPreferredMtu);
  NimBLEDevice::setSecurityAuth(true, false, false);  // bonding allowed, as before

  s_server = NimBLEDevice::createServer();
  s_server->setCallbacks(&s_serverCallbacks, false);
  s_server->advertiseOnDisconnect(true);

  NimBLEService* service = s_server->createService(kMidiServiceUuid);
  s_chr = service->createCharacteristic(
      kMidiCharUuid,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY,
      kBleMidiMaxPacket);
  s_chr->setCallbacks(&s_chrCallbacks);

  // 128-bit service UUID in the advertisement, name in the scan response
  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(kMidiServiceUuid);
  adv->enableScanResponse(true);
  adv->setName(kDeviceName);
  adv->start();

  if (xTaskCreatePinnedToCore(bleMidiTask, "bleMidiTask", kSenderStack, nullptr,
                              kSenderPriority, &s_senderTask,
                              kSenderCore) != pdPASS) {
    log_e("Failed to create bleMidiTask");
  }

  // Start BT LED blinking to indicate BLE is enabled and waiting for connection
  hwui.start_blink(BT_LED, WIFI_AP_PULSE_TIME, 0.2);
}

static uint8_t statusFor(uint8_t type, int channel) {
  int c = channel - 1;
  if (c < 0)
    c = 0;
  if (c > 15)
    c = 15;
  return type | (uint8_t)c;
}

static void enqueue(uint8_t status, int d1, int d2) {
  if (!s_ready)
    return;
  portENTER_CRITICAL(&s_mux);
  s_outQ.push(status, d1 & 0x7F, d2 & 0x7F);
  portEXIT_CRITICAL(&s_mux);
}

void MidiBLEsendCC(int control, int value, int channel) {
  enqueue(statusFor(0xB0, channel), control, value);
}

void MidiBLEsendNoteOn(int note, int velocity, int channel) {
  enqueue(statusFor(0x90, channel), note, velocity);
}

void MidiBLEsendNoteOff(int note, int velocity, int channel) {
  enqueue(statusFor(0x80, channel), note, velocity);
}

void MidiBLEsendPitchBend(int value, int channel) {
  int v = value + 8192;
  if (v < 0)
    v = 0;
  if (v > 16383)
    v = 16383;
  enqueue(statusFor(0xE0, channel), v & 0x7F, (v >> 7) & 0x7F);
}

bool midiBLEReadProgramChange(uint8_t* channel, uint8_t* program) {
  BleMidiProgramChange pc;
  if (s_inQ == nullptr || xQueueReceive(s_inQ, &pc, 0) != pdTRUE)
    return false;
  *channel = pc.channel;
  *program = pc.program;
  return true;
}

#endif  //INCLUDE_BLE
