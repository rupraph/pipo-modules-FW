<script lang="ts">
  import { uid } from "../../utils";

  export let label: string;
  export let value: boolean;
  let id = uid();
</script>

<span class="input checkbox column {$$restProps.class || ''}">
  <label for={id || label}>{label} </label>
  <span class="input-wrapper">
    <input type="checkbox" {id} name={label} bind:checked={value} />
  </span>
</span>

<style>
  .input.checkbox {
    grid-auto-flow: column;
    grid-template-rows: auto;
    grid-template-columns: auto auto;
    width: 100%;
  }
</style>
