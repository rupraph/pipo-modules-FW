<script lang="ts">
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let value: string = "";
  export let readonly: boolean = false;
  export let maxlength: number = 127;
  export let minlength: number = 0;
  export let validate: (value: string) => string = () => "";
  let validationError: string = "";
  let id = uid();
  // Reactive statement to validate the name length
  $: {
    if (value.length < minlength) {
      validationError = `The ${label} must be at least ${minlength} characters long.`;
    } else if (value.length > maxlength) {
      validationError = `The ${label} must be no more than ${maxlength} characters long.`;
    } else {
      validationError = validate(value);
    }
  }
</script>

<Input {label} {id} {validationError}>
  <input
    type="text"
    {id}
    name={label}
    bind:value
    {readonly}
    {maxlength}
    {minlength}
  />
</Input>

<style>
</style>
