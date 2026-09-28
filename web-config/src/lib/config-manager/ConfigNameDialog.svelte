<script lang="ts">
  import Modal from "../modal.svelte";
  import { createEventDispatcher } from "svelte";

  export let open = false;
  export let title = "New Config";
  export let initialValue = "";
  export let existingNames: string[] = [];
  export let confirmLabel = "Create";

  const dispatch = createEventDispatcher<{
    confirm: string;
    cancel: void;
  }>();

  let value = "";
  let error = "";

  const NAME_REGEX = /^[a-zA-Z0-9_-]{1,16}$/;

  $: if (open) {
    value = initialValue;
    error = "";
  }

  function validate(name: string): string {
    if (!name) return "Name is required";
    if (name.length > 16) return "Max 16 characters";
    if (!/^[a-zA-Z0-9_-]+$/.test(name))
      return "Only letters, numbers, - and _ allowed";
    if (
      existingNames.some(
        (n) => n.toLowerCase() === name.toLowerCase() && n !== initialValue,
      )
    )
      return "Name already exists";
    return "";
  }

  function handleInput(e: Event) {
    const input = e.target as HTMLInputElement;
    // Filter disallowed chars in real-time
    value = input.value.replace(/[^a-zA-Z0-9_-]/g, "").slice(0, 16);
    input.value = value;
    error = validate(value);
  }

  function handleConfirm() {
    error = validate(value);
    if (error) return;
    dispatch("confirm", value);
    open = false;
  }

  function handleCancel() {
    dispatch("cancel");
    open = false;
  }

  function handleKeydown(e: KeyboardEvent) {
    if (e.key === "Enter" && !error && value) {
      handleConfirm();
    }
  }
</script>

<Modal
  bind:open
  --modal-overlay-bg="rgba(0,0,0,0.7)"
  --modal-overlay-blur="blur(5px)"
  --modal-z="25"
>
  <div class="dialog">
    <h3>{title}</h3>
    <div class="input-group">
      <input
        type="text"
        bind:value
        on:input={handleInput}
        on:keydown={handleKeydown}
        placeholder="Config name"
        maxlength="16"
      />
      {#if error}
        <span class="error">{error}</span>
      {/if}
      <span class="hint">a-z, 0-9, dash, underscore — max 16 chars</span>
    </div>
    <div class="actions">
      <button class="secondary" on:click={handleCancel}>Cancel</button>
      <button
        class="primary"
        on:click={handleConfirm}
        disabled={!!error || !value}>{confirmLabel}</button
      >
    </div>
  </div>
</Modal>

<style>
  .dialog {
    padding: 16px;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }
  h3 {
    margin: 0;
    font-size: 16px;
    color: var(--main);
  }
  .input-group {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }
  input[type="text"] {
    background-color: var(--bg-secondary);
    border: 1px solid var(--main);
    border-radius: 4px;
    height: 32px;
    padding: 0 8px;
    color: var(--text-color);
    font-size: 14px;
    width: 100%;
    box-sizing: border-box;
  }
  .error {
    color: var(--red);
    font-size: 12px;
  }
  .hint {
    color: var(--text-color-secondary);
    font-size: 11px;
  }
  .actions {
    display: flex;
    gap: 8px;
    justify-content: flex-end;
  }
  button:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
</style>
