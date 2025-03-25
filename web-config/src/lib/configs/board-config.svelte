<script lang="ts">
  import { pipoio } from "../../pipoio";
  import type { GeneralConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Select from "../form/Select.svelte";
  import { schema } from "../../schema";

  export let generalconfig: GeneralConfig;

  const options = [
    { label: "Acces Point", value: "AP" },
    { label: "Station", value: "STA" },
  ];
</script>

<!-- <Select label="Wifi Mode" {options} bind:value={generalconfig.Wifi_mode} /> -->
<Text
  label="Pipo Name"
  bind:value={generalconfig.PipoName}
  maxlength={schema.name.max}
  minlength={schema.name.min}
/>
<!-- <button class="primary" on:click={reboot} style="width: fit-content"
  >Reboot</button
> -->
