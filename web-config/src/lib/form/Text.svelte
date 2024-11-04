<script lang="ts">
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let value: string = "";
  export let readonly: boolean = false;
  export let maxlength: number = 127;
  let id = uid();
</script>

<Input {label} {id}>
  <input type="text" {id} name={label} bind:value {readonly} {maxlength} />
</Input>

<style>
</style>
