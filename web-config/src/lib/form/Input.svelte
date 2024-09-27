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
  .input.disabled,
  .input.disabled > :global(*) {
    opacity: 0.5;
    cursor: not-allowed;
  }
  .input.disabled > .input-wrapper :global(input),
  .input.disabled > .input-wrapper :global(div) {
    pointer-events: none;
  }
  .minmax-input {
    grid-auto-flow: column;
    grid-template-rows: auto auto;
    grid-template-columns: auto;
    width: 100%;
  }
  .minmax-input > .input-wrapper {
    width: 100%;
    display: flex;
    flex-direction: column;
    padding-top: 0.5em;
  }

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
