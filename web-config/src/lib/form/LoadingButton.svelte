<script lang="ts">
  export let loading: boolean = false;
  export let disabled: boolean = false;
  export let onClick: () => void;
  export let width: string = "100px"; // New prop for adjustable width
</script>

<button
  on:click={onClick}
  class={($$restProps.class || "") +
    " loading-button contract " +
    (loading ? " loading" : "") +
    (disabled ? " disabled" : "")}
  style="width: {width}; box-sizing: border-box;"
>
  {#if !loading}
    <span class="label"><slot></slot></span>
  {:else}
    <span class="spinner loader"> </span>
  {/if}
</button>

<style>
  .loading-button {
    /* height: 38px; */
    max-height: 38px;
  }
  .loading-button.loading {
    cursor: wait;
  }
  .loading-button .spinner {
    top: 50%;
    opacity: 0;
    width: 2em;
    height: 2em;
    border: 5px solid #fff;
    border-bottom-color: transparent;
    border-radius: 50%;
    display: inline-block;
    box-sizing: border-box;
    animation: rotation 1s linear infinite;
  }
  .loading-button.contract {
    overflow: hidden;
    width: 100px;
  }
  .loading-button.contract.loading .label {
    opacity: 0;
  }
  .loading-button.contract.loading .spinner {
    opacity: 1;
  }

  @keyframes rotation {
    0% {
      transform: rotate(0deg);
    }
    100% {
      transform: rotate(360deg);
    }
  }
</style>
