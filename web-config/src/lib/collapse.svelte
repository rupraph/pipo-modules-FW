<script lang="ts">
  import { slide } from "svelte/transition";
  import { cubicOut } from "svelte/easing";
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
    <div class="input-checkbox">
      <input type="checkbox" on:click|stopPropagation bind:checked={value} />
      <!-- <Switch bind:value={state} design="slider" label="" /> -->
    </div>
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
  h3 {
    cursor: pointer;
    display: flex;
    align-items: center;
    width: auto;
    margin: 2%;
  }
  h3:hover {
    text-decoration: underline;
  }
  .arrow {
    margin-right: 1em;
    border-radius: 3px;
    border: solid white;
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

  .title-container {
    display: flex;
    align-items: center;
    height: auto;
    justify-content: space-between;
  }

  .input.checkbox {
    /* margin-left: 20%; */
    display: flex;
    /* margin-left: 20%; */
    /* margin-right:80%; */
  }

  .input-checkbox {
    padding-right: 50%;
    /* padding: 1%; */
    /* padding: 1vm; */
    /* justify-content: center;   */
  }
</style>
