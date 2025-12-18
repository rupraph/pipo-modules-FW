# WiFi Management Implementation Summary

## Changes Implemented

### 1. Password Manager (`pw-manager.hpp`)

**Added disconnect state management:**

- `userWantsDisconnected` boolean flag (persisted to preferences)
- `clearDisconnectRequest()` - Called when user manually connects
- `setDisconnectRequest()` - Called when user clicks disconnect
- `shouldAutoConnect()` - Returns whether to auto-connect at boot
- Updated `save()` and `load()` to persist the disconnect flag

### 2. WiFi Core Logic (`pipowifi.cpp`)

**Fixed AP Switching:**

- Modified `refresh()` to call `step()` after intentional disconnect
- Now properly triggers connection to new AP after disconnecting from old one

**Improved Disconnect Handler:**

- `onSTADisconnectedHandler()` now:
  - Distinguishes between intentional disconnect (AP switch) and failed connection
  - Preserves `next.ssid/password` during AP switch
  - Always calls `step()` to handle next action
  - Clears credentials only after 3 failed attempts to NEW AP

**Enhanced Connection Success:**

- `onSTAConnectedHandler()` now clears disconnect flag on successful connection
- Ensures auto-connect is re-enabled when user manually connects

**Smart Auto-Connect Logic:**

- Modified `step()` to respect `shouldAutoConnect()` flag
- Clears disconnect flag when user initiates manual connection
- Skips auto-connect at boot if user previously disconnected

**Disconnect Method:**

- Updated `disconnect()` to set disconnect flag via password manager

### 3. Server Endpoint (`server.cpp`)

**Added HTTP endpoint:**

- `/wifi-disconnect` (POST) - Allows UI to trigger disconnect

### 4. UI Component (`web-config/src/lib/wifi/connect.svelte`)

**Added disconnect functionality:**

- `onDisconnect()` function to call backend endpoint
- "Disconnect" button (shown only when connected to a network)
- Properly pauses websocket and reloads page after disconnect

## How It Works

### Network Switching Flow

1. User clicks "Connect to Network B" while connected to Network A
2. UI calls `/wifi-connect` with Network B credentials
3. `setSSID()` and `setPassword()` set `next.ssid/password` and `isChangingAP = true`
4. `refresh()` detects `isChangingAP`, disconnects from Network A
5. `onSTADisconnectedHandler()` sees intentional disconnect + `next.ssid` set
6. Handler calls `step()` which attempts connection to Network B
7. On success, `onSTAConnectedHandler()` saves credentials and clears disconnect flag

### Disconnect Flow

1. User clicks "Disconnect" button
2. UI calls `/wifi-disconnect`
3. `disconnect()` calls `pwm.setDisconnectRequest()` to set flag
4. WiFi disconnects from current network
5. Board stays in AP-only mode
6. At next boot, `step()` sees `!shouldAutoConnect()` and skips reconnection

### Reconnect Flow

1. User manually connects to any network
2. `step()` calls `pwm.clearDisconnectRequest()` before connection attempt
3. Flag cleared, connection proceeds
4. At next boot, board will auto-connect to this network

### Failed Connection Flow

1. User tries to connect to network with wrong password
2. Connection fails, `reconnectAttempts` increments (max 3)
3. After 3 failures, credentials are cleared from `next.ssid/password`
4. Credentials are NOT saved to password manager
5. User can try again with correct password

### Boot Reconnection Flow

1. Boot starts, STA mode activated
2. `step()` checks if `shouldAutoConnect()` is true
3. If true, calls `connect()` which tries highest-scored network
4. If network unavailable, fails after 3 attempts but KEEPS credentials
5. Next boot will try again

## Testing Checklist

- [ ] Connect to Network A → verify connection works
- [ ] Switch to Network B while connected to A → verify switch works
- [ ] Click Disconnect while connected → verify disconnects and stays disconnected
- [ ] Reboot after disconnect → verify doesn't auto-connect
- [ ] Manually connect after disconnect → verify reconnects and flag cleared
- [ ] Reboot after manual connect → verify auto-connects
- [ ] Try wrong password → verify fails and doesn't save credentials
- [ ] Boot with network out of range → verify keeps credentials
- [ ] Forget network → verify removed from storage
- [ ] Multiple networks stored → verify highest-scored is selected at boot

## Files Modified

1. `/src/wifi/pw-manager.hpp` - Disconnect flag management
2. `/src/wifi/pipowifi.cpp` - Core WiFi logic fixes
3. `/src/server/server.cpp` - HTTP endpoint for disconnect
4. `/web-config/src/lib/wifi/connect.svelte` - UI disconnect button
5. `/analysis/wifi-scenarios.md` - Expected behavior documentation

## Notes

- The `userWantsDisconnected` flag is the key to the whole system
- It's separate from network credentials (forget vs disconnect)
- Cleared automatically on any manual connection attempt
- Prevents auto-connect at boot when set
- Simple, standard approach used by most WiFi managers
