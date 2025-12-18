# WiFi Management - Expected Behaviors

## Complete Scenarios

### **Network Management**

**Scenario 1: Multiple successful connections**
- Connect to Home → saved, score: 100
- Connect to Office → saved, score: 105 (most recent)
- Reboot → Auto-connects to Office ✅

**Scenario 2: Disconnect from current network**
- Connected to Office, Home also stored
- Click "Disconnect" → `userWantsDisconnected = true`, board disconnects
- Reboot → Stays disconnected (doesn't try Office OR Home) ✅

**Scenario 3: Reconnect after disconnect**
- (Board in disconnected state)
- Manually connect to Home → `userWantsDisconnected = false`
- Reboot → Auto-connects to Home ✅

**Scenario 4: Forget a network**
- Connected to Office, Home also stored
- Click "Forget Office" → Office removed, still connected to Office until next scan/reboot
- Reboot → Auto-connects to Home (highest remaining) ✅

**Scenario 5: Disconnect then forget**
- Connected to Office, Home also stored  
- Click "Disconnect" → disconnected, both networks still stored
- Click "Forget Office" → Office removed from storage
- Reboot → Stays disconnected (doesn't try Home) ✅

**Scenario 6: Switch networks**
- Connected to Office
- Click "Connect to Home" → `userWantsDisconnected = false`, switches to Home
- Reboot → Auto-connects to Home ✅

**Scenario 7: Wrong password**
- Try CoffeeShop with wrong password → fails 3 times
- Credentials auto-removed
- User can retry with correct password ✅

**Scenario 8: Boot with unreachable network**
- Last connected to Office (not in range)
- Reboot → Tries Office 3 times, gives up but KEEPS credentials
- Next reboot → Tries Office again if in range ✅

## Implementation Details

### Key Features
1. **Disconnect**: Sets flag to prevent auto-connect at boot, keeps all credentials
2. **Forget**: Removes credentials completely from storage
3. **Manual Connect**: Clears disconnect flag, enables auto-connect
4. **Failed Connection**: Removes credentials only after 3 attempts on NEW connection
5. **Boot Reconnect**: Keeps credentials even if network unavailable

### State Management
- `userWantsDisconnected` boolean flag (persisted to preferences)
- Cleared on any manual connection attempt
- Set on explicit disconnect request
- Controls boot auto-connect behavior
