<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  onMount(async () => {
    // const device = await navigator.hid.requestDevice({ filters: [] });
    let devices = await navigator.hid.getDevices();
    console.log(devices);
    devices.forEach((device) => {
      console.log(`HID: ${device.productName}`);
    });
  });

  onDestroy(() => {});
</script>

<h3>HID</h3>
<section></section>

<style>
</style>
