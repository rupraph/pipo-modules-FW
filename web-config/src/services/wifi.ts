import { setNetworks, setSignal, setSSID } from "../lib/wifi/store";
import { rssiToSignalStrength } from "../lib/wifi/utils";
import { pipoio } from "../pipoio";



export async function fetchNetworks() {
    for (let i = 0; i < 5; i++) {
      const { data } = await pipoio.get<string>("/wifi-networks");
      if (!data || data === "Scanning") {
        await new Promise((resolve) => setTimeout(resolve, 1000));
        continue;
      }
      const [_, ...lines] = data.trim().split("\n");
      const networks = lines
        .map((line) => {
          const [_, ssid, signal, connected, known] = line.match(
            /"(.*)" (-?\d+) (\d+) (\d+)/
          )!;

          const quality = rssiToSignalStrength(parseInt(signal));
          return {
            ssid,
            known: known === "1",
            connected: connected === "1",
            quality,
          };
        })
        .filter((e) => e.ssid)
        .sort((a, b) => {
          if (a.connected) return -1;
          if (b.connected) return 1;
          if (a.known && !b.known) return -1;
          if (!a.known && b.known) return 1;
          return b.quality - a.quality;
        });
      const connected = networks.find((n) => n.connected);
      setNetworks(networks);
      if (connected) {
        setSignal(connected.quality);
        setSSID(connected.ssid);
      } else {
        setSignal(0);
        setSSID("Disconected");
      }
      break;
    }
  }