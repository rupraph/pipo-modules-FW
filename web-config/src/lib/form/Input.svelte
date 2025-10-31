<script lang="ts">
  export let label: string;
  export let id: string;
  export let validationError: string | undefined = undefined;
  $: valid = validationError ? "invalid" : "valid";
</script>

<span class="input {$$restProps.class || ''}{valid}">
  <label for={id || label}>{label} </label>

  <span class="input-wrapper">
    <slot />
  </span>
  {#if validationError}
    <p class="validation">{validationError}</p>
  {/if}
</span>

<style>
  .input.disabled,
  .input.disabled > :global(*) {
    opacity: 0.5;
    cursor: not-allowed;
  }
  .input.disabled > .input-wrapper :global(input),
  .input.disabled > .input-wrapper :global(div) {
    pointer-events: none;
  }

  .input-wrapper {
    display: flex;
    flex-direction: row;
    border-radius: 3px;
    background-color: var(--bg-secondary);
    border: 1px solid transparent;
    align-items: center;
    width: 100%;
    color: var(--text-color-secondary);
  }

  .input-wrapper:hover {
    border: 1px solid var(--main);
  }

  .validation {
    color: var(--red);
    font-size: 0.875rem;
    margin-top: 0.25rem;
  }
</style>
