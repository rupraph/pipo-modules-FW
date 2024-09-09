<script lang="ts">
  export let label: string;
  export let id: string;
  export let valid: "valid" | "invalid" | string | undefined;
  export let validationError: string | undefined;
</script>

<span class="input {$$restProps.class || ''} ${valid}">
  <label for={id || label}>{label} </label>

  <span class="input-wrapper">
    <slot />
  </span>
  {#if validationError}
    <p class="validation">{validationError}</p>
  {/if}
</span>

<style>
</style>
