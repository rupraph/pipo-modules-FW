Heap test on analog. BLE enabled + connected +save config + scan networks

After websocket input buffer and translator pre allocation:
[113417][D][debug.cpp:50] pipoDebugHeapFull(): Free Heap: 58188 bytes (56.82 KB)
[113418][D][debug.cpp:53] pipoDebugHeapFull(): Fragmentation Ratio: 43.71%

removed HID objects:
[263689][D][debug.cpp:50] pipoDebugHeapFull(): Free Heap: 59708 bytes (58.31 KB)
[263690][D][debug.cpp:53] pipoDebugHeapFull(): Fragmentation Ratio: 38.28%
very similar

after optimising mid translator
[ 77780][D][debug.cpp:50] pipoDebugHeapFull(): Free Heap: 59876 bytes (58.47 KB)
[ 77781][D][debug.cpp:53] pipoDebugHeapFull(): Fragmentation Ratio: 38.45%
very similar
