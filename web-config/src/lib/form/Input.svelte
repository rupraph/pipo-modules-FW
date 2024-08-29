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
  .input-wrapper {
    display: flex;
    flex-direction: row;
    border-radius: 3px;
    background-color: var(--bg-secondary);
    border: 1px solid transparent;
    align-items: center;
  }
  .input-wrapper:hover {
    border: 1px solid var(--main);
  }
  .input-wrapper.invalid {
    border: 1px solid var(--red);
  }
</style>
