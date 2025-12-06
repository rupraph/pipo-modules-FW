<script lang="ts">
  import type { OscConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import InfoModal from "../InfoModal.svelte";
  import Select from "../form/Select.svelte";
  import Number from "../form/Number.svelte";
  export let osc: OscConfig;
</script>

<section>
  <h3>
    OSC Output Settings
    <InfoModal>
      <p>
        Configure the Open Sound Control (OSC) output settings for your Pipo
        device. OSC is a protocol used for communication among computers, sound
        synthesizers, and other multimedia devices.
      </p>
      <ul>
        <li>
          <strong>Enable OSC Output:</strong> Toggle to enable or disable OSC output
          functionality.
        </li>
        <li>
          <strong>Destination IP:</strong> Specify the IP address of the device that
          will receive the OSC messages.
        </li>
        <li>
          <strong>Destination Port:</strong> Set the port number on which the receiving
          device is listening for OSC messages.
        </li>
        <li>
          <strong>Message Prefix:</strong> Define a prefix for all OSC messages sent
          from the Pipo device. This helps in organizing and identifying messages
          on the receiving end.
        </li>
      </ul>
      <p>
        Ensure that the destination device is configured to receive OSC messages
        on the specified IP and port.
      </p>
    </InfoModal>
  </h3>
  <Number label="Out min"></Number>
  <Number label="Out max"></Number>
</section>

<style scoped>
  section {
    width: 100%;
  }
  h3 {
    display: flex;
    align-items: center;
    gap: 8px;
  }
</style>
