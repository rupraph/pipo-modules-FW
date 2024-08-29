<script lang="ts">
  export let label: string;
  export let value: boolean;
  let id = Math.random().toString(36).substring(2);
</script>

<span class="input checkbox column {$$restProps.class || ''}">
  <label for={id || label}>{label} </label>
  <span class="input-wrapper">
    <input type="checkbox" {id} name={label} bind:checked={value} />
  </span>
</span>

<style>
  .input.checkbox {
    width: 100%;
  }
</style>
