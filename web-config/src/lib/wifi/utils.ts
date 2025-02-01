export function rssiToSignalStrength(rssi: number) {
  const input = rssi;
  // if (isNaN(rssi)) {
  //   rssi = -100;
  // }
  // // TODO: Check this magic numbers from AI,
  // rssi = Math.max(-100, Math.min(rssi, -50));
  // rssi = (-rssi - 50) / 50;
  // const res = Math.round(rssi * 5);
  // return res;
  const min = -100;
  const max = -40;
  rssi = Math.min(Math.max(rssi, min), max);
  const res = Math.round(((rssi - min) / (max - min)) * 5);

  console.log("rssiToSignalStrength", input, res);
  return res;
}
