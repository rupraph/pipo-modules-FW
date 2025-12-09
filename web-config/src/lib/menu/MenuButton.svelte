<script lang="ts">
  export let border = false;
  export let secondary = false;
  export let connected = false;
</script>

<button class:border class:secondary class:connected on:click>
  <slot></slot>
</button>

<style>
  button {
    width: 30px;
    height: 30px;
    border: none;
    border-radius: 50%;
    padding: 0;
    background-color: transparent;
  }
  button.secondary {
    background-color: var(--grey);
    color: var(--bg-primary);
  }
  button.connected {
    background-color: var(--main);
    color: var(--bg-primary);
  }
  button.border {
    border: 1px solid var(--main);
  }
  button:hover {
    background-color: var(--bg-secondary);
    cursor: pointer;
  }
  button.secondary:hover {
    background-color: var(--bg-tertiary);
  }
  button.connected:hover {
    background-color: var(--main-lighter);
  }
  button > * {
    width: 100%;
    height: 100%;
  }
</style>
