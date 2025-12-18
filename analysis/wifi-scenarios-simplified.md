# WiFi Management - Simplified Scenarios

## Design Philosophy

**Simplified approach**: To switch networks, user must disconnect first, then connect to the new one.
This eliminates complex AP-switching state machine logic and reduces crash potential.

## Complete Scenarios

### **Boot Scenarios**

**Scenario 1: Boot with no stored networks**

- No credentials in storage
- Start in AP-only mode
- User can connect via AP and configure WiFi

**Scenario 2: Boot with stored networks + auto-connect enabled**

- `userWantsDisconnected = false`
- Automatically connect to highest-scored (most recent successful) network
- If network not available: Try 3 times, then give up but KEEP credentials
- Result: Either connected or in AP-only mode

**Scenario 3: Boot with stored networks + auto-connect disabled**

- `userWantsDisconnected = true` (user previously disconnected)
- Stay in AP-only mode
- Do NOT attempt any auto-connect
- Credentials remain stored for manual connection later

### **Connection Scenarios**

**Scenario 4: First-time connection (new network)**

- User enters SSID + password
- Attempt connection
- If successful: Save credentials, clear `userWantsDisconnected`, set as highest score
- If failed (3 attempts): Do NOT save credentials, allow retry

**Scenario 5: Connection with stored credentials**

- User selects known network
- Connect using stored password
- If successful: Promote network score, clear `userWantsDisconnected`
- If failed: After 3 attempts, remove credentials (password changed?)

**Scenario 6: Network switch (2-step process)**

- Currently connected to Network A, want to connect to Network B
- **Step 1**: User clicks "Disconnect" → disconnects from A, sets `userWantsDisconnected=true`
- **Step 2**: User clicks "Connect to B" → connects to B, clears `userWantsDisconnected=false`
- **Note to user**: "To switch networks, disconnect first, then connect to the new one"

### **Network Loss & Reconnection**

**Scenario 7: Temporary network loss (brief outage)**

- Connected to Network A, signal drops temporarily
- Auto-reconnect attempts: Try up to 3 times with 2-second intervals
- If successful: Resume connection
- If all attempts fail: Go to Scenario 8

**Scenario 8: Persistent network loss (permanent outage)**

- Network unavailable for extended period or moved out of range
- After 3 failed reconnection attempts: Give up, enter AP-only mode
- **KEEP credentials stored** (user didn't forget, network just unavailable)
- On next boot: Will try again (Scenario 2)
- **NO infinite retry loop** - stop after initial attempts

**Scenario 9: Network loss at boot**

- Boot, attempt to connect to stored network
- Network not available
- Try 3 times, then give up
- Enter AP-only mode
- Keep credentials for later

### **Manual Control Scenarios**

**Scenario 10: Manual disconnect**

- User clicks "Disconnect" button
- Disconnect from current network
- Set `userWantsDisconnected = true`
- Save state
- **Keep all credentials** (just preventing auto-connect)
- On reboot: Stay disconnected (Scenario 3)

**Scenario 11: Forget network**

- User clicks "Forget" on a network
- Remove credentials from storage
- If currently connected to that network: Disconnect
- Network won't appear in saved list anymore

**Scenario 12: Reconnect after manual disconnect**

- Previously disconnected manually (`userWantsDisconnected = true`)
- User manually connects to any network
- Clear `userWantsDisconnected = false`
- On next boot: Auto-connect to this network (Scenario 2)

## Key Parameters

- **Initial connection attempts**: 3 (for wrong password detection)
- **Reconnection attempts (lost connection)**: 3 (prevent infinite loops)
- **Reconnection delay**: 2 seconds between attempts
- **Boot connection attempts**: 3 (network not available at boot)
- **Max stored networks**: 5 (or current MAX_NETWORKS setting)

## State Variables

- `userWantsDisconnected`: Boolean, persisted to NVS
- `reconnectAttempts`: Counter, reset on success or after max reached
- `status`: DISCONNECTED, CONNECTING, CONNECTED
- `next.ssid/password`: Pending connection credentials

## Removed Features

- ~~`isChangingAP` flag~~ - No longer needed
- ~~Automatic AP switching~~ - User must disconnect first
- ~~Complex state transitions for switching~~ - Simplified flow

## Implementation Notes

- All WiFi API calls happen in wifiTask context only
- Event handlers only set flags
- Stack size: 4096 bytes (monitor with high watermark)
- Save to NVS only from task context, never from interrupt handlers
