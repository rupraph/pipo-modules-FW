export function rssiToSignalStrength(rssi: number) {
  const min = -100;
  const max = -40;
  rssi = Math.min(Math.max(rssi, min), max);
  return Math.round(((rssi - min) / (max - min)) * 5);
}
