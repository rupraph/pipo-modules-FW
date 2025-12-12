export function rssiToSignalStrength(rssi: number): number {
  if (rssi >= -50) return 5;      // Excellent (5 bars)
  if (rssi >= -60) return 4;      // Good (4 bars)
  if (rssi >= -70) return 3;      // Fair (3 bars)
  if (rssi >= -80) return 2;      // Weak (2 bars)
  return 1;                        // Very weak (1 bar)
}
