<script lang="ts">
  import { scale } from "svelte/transition";
  import { cubicOut } from "svelte/easing";
  import { CircleX } from "lucide-svelte";
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
    class="modal modal-style"
    transition:scale={{
      duration: 300,
      easing: cubicOut,
    }}
  >
    <div class="close" on:click={() => (open = false)}>
      <CircleX size={28} />
    </div>
    <slot></slot>
  </div>
{/if}

<style>
  .modal {
    top: 50%;
    left: 50%;
    z-index: 21;
    width: calc(100vw - 8px);
    max-width: 400px;
    background-color: var(--bg-network);
    position: fixed;
    overflow: hidden;
    transform: translate(-50%, -50%);
    padding: 0 8px 8px 8px;
  }
  .modal :global(p) {
    text-align: left;
    margin: 0.5em 0;
  }
  .close {
    cursor: pointer;
    display: flex;
    justify-content: flex-end;
    padding: 8px 0 0;
    color: var(--text-color);
  }
  .close:hover {
    color: var(--main);
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
    backdrop-filter: blur(7px);
  }
</style>
