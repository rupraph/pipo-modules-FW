<script lang="ts">
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let value: string = "";
  let id = uid();
</script>

<Input {label} {id}>
  <input type="text" {id} name={label} bind:value />
</Input>

<style>
</style>
