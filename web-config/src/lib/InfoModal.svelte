<script lang="ts">
  import Modal from "./modal.svelte";
  import {Info} from "lucide-svelte";
  let open = false;
</script>


<button class="info" on:click={() => open = !open}>
 <Info color="var(--grey)"/>
</button>

<Modal bind:open={open}>
  <slot></slot>
</Modal>

<style scoped>
  .info {
    all: unset;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    width: 24px;
    height: 24px;
    border-radius: 50%;
    transition: background-color 0.2s;
  }
  .info:hover {
    background-color: var(--bg-secondary);
  }
</style>
