<script>
  import { createEventDispatcher } from "svelte";
  import { fade } from "svelte/transition";
  import { CheckCircle2, XCircle, Info, AlertTriangle, X } from "lucide-svelte";

  const dispatch = createEventDispatcher();

  export let type = "error";
  export let dismissible = true;
</script>

<article class={type} role="alert" transition:fade>
  <!-- {#if type === "success"}
    <CheckCircle2 size={20} />
  {:else if type === "error"}
    <XCircle size={20} />
  {:else if type === "warning"}
    <AlertTriangle size={20} />
  {:else}
    <Info size={20} />
  {/if} -->

  <div class="text">
    <slot />
  </div>

  {#if dismissible}
    <button class="close" on:click={() => dispatch("dismiss")}>
      <X size={16} />
    </button>
  {/if}
</article>

<style lang="postcss">
  article {
    color: white;
    padding: 0.75rem 1.5rem;
    border-radius: 0.5rem;
    display: flex;
    align-items: center;
    margin: 0 auto 0.5rem auto;
    width: 20rem;
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.5);
    /* opacity: 0.85; */
  }
  .error {
    background: #e33333dd;
  }
  .success {
    background: #f4ffb0;
    color: var(--bg-primary);
  }
  .info {
    background: #4a9eeadd;
  }
  .warning {
    background: #ff8c00dd;
  }
  .text {
    margin-left: 1rem;
  }
  button {
    color: white;
    background: transparent;
    border: 0 none;
    padding: 0;
    margin: 0 0 0 auto;
    line-height: 1;
    font-size: 1rem;
    cursor: pointer;
  }
</style>
