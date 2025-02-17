<script lang="ts">
  import { scale } from "svelte/transition";
  import { cubicOut } from "svelte/easing";
  export let open = false;

  function close(e: KeyboardEvent) {
    if ((e.keyCode || e.which) == 27) {
      e.stopImmediatePropagation();
      e.stopPropagation();
      e.preventDefault();
      open = false;
    }
  }
</script>

{#if open}
  <div
    class="overlay"
    on:click={() => (open = false)}
    on:keyup={close}
    tabindex="0"
  ></div>
  <div
    class="modal section-borders"
    transition:scale={{
      duration: 300,
      easing: cubicOut,
    }}
  >
    <slot></slot>
    <div class="close" on:click={() => (open = false)}>✖</div>
  </div>
{/if}

<style>
  .modal {
    top: 50%;
    left: 50%;
    z-index: 21;
    width: calc(100vw - 8px);
    max-width: 600px;
    background-color: var(--bg-tertiary);
    position: fixed;
    overflow: hidden;
    transform: translate(-50%, -50%);
  }
  .close {
    position: absolute;
    top: 0;
    right: 0;
    padding: 5px;
    font-size: 1.6em;
    cursor: pointer;
  }
  .overlay {
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: 20;
    position: fixed;
    overflow: hidden;
    background: rgba(0, 0, 0, 0.5);
    backdrop-filter: blur(5px);
  }
</style>
