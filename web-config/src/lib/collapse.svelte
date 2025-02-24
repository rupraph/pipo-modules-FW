<script lang="ts">
  import { slide } from "svelte/transition";
  import { cubicOut } from "svelte/easing";
  import Switch from "./form/Switch.svelte";
  export let title: string;
  export let open = false;
  export let value: boolean | undefined = undefined;

  const hasState = value !== undefined;
</script>

<div class="title-container">
  <h3 on:click={() => (open = !open)}>
    <i class="arrow {open ? 'down' : 'right'}"></i>
    {title}
  </h3>
  {#if hasState}
    <!-- <input
      class="input-checkbox"
      type="checkbox"
      on:click|stopPropagation
      bind:checked={value}
    /> -->
    <Switch bind:value design="slider" label="" />
  {/if}
</div>

{#if open}
  <section
    class={$$restProps.class}
    transition:slide={{
      duration: 300,
      easing: cubicOut,
      axis: "y",
    }}
  >
    <slot></slot>
  </section>
{/if}

<style>
  .title-container {
    display: flex;
    align-items: center;
    height: 3em;
    justify-content: space-between;
    width: 100%;
  }
  h3 {
    cursor: pointer;
    display: flex;
    align-items: center;
    flex: 1;
  }
  h3:hover {
    text-decoration: underline;
  }
  .arrow {
    margin-right: 1em;
    border-radius: 3px;
    border: solid rgb(0, 0, 0);
    border-width: 0 3px 3px 0;
    display: inline-block;
    padding: 3px;
  }

  .right {
    transform: rotate(-45deg);
    -webkit-transform: rotate(-45deg);
  }

  .left {
    transform: rotate(135deg);
    -webkit-transform: rotate(135deg);
  }

  .up {
    transform: rotate(-135deg);
    -webkit-transform: rotate(-135deg);
  }

  .down {
    transform: rotate(45deg);
    -webkit-transform: rotate(45deg);
  }

  section {
    overflow-x: auto;
  }
</style>
