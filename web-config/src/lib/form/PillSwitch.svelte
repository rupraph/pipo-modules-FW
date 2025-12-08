<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { uid } from "../../utils";

  const dispatch = createEventDispatcher();

  export let label: string;
  export let value: boolean;
  export let offLabel: string | undefined = undefined;
  export let onLabel: string | undefined = undefined;

  let id = uid();
  $: hasLabels = offLabel !== undefined || onLabel !== undefined;
</script>

<div class="pill-switch-container">
  <label class="main-label">{label}</label>
  <div class="pill-switch" class:minimal={!hasLabels}>
    {#if hasLabels}
      <div class="pill-indicator" class:on={value}></div>
      <input
        type="checkbox"
        {id}
        name={label}
        bind:checked={value}
        on:change={() => dispatch("change", value)}
      />
      <label for={id} class:active={!value}>{offLabel || "OFF"}</label>
      <label for={id} class:active={value}>{onLabel || "ON"}</label>
    {:else}
      <input
        type="checkbox"
        {id}
        name={label}
        bind:checked={value}
        on:change={() => dispatch("change", value)}
      />
      <label for={id} class="minimal-track" class:on={value}>
        <div class="minimal-pill"></div>
      </label>
    {/if}
  </div>
</div>

<style>
  .pill-switch-container {
    display: flex;
    align-items: center;
    justify-content: space-between;
    width: 100%;
  }

  .main-label {
    font-size: 14px;
    color: var(--color);
  }

  .pill-switch {
    border: 2px solid var(--main);
    position: relative;
    display: inline-flex;
    background-color: var(--bg-primary);
    border-radius: 20px;
    padding: 2px;
    gap: 2px;
  }

  /* Minimal mode styles */
  .pill-switch.minimal {
    border: none;
    /* background-color: transparent; */
    background-color: var(--bg-secondary);
    padding: 0;
  }

  .minimal-track {
    position: relative;
    width: 50px;
    height: 20px;
    background-color: red;
    border-radius: 20px;
    border: 2px solid var(--main);
    padding: 2px;
    transition: background-color 0.3s ease;
    display: flex;
    align-items: center;
  }

  .minimal-track.on {
    background-color: var(--main);
  }

  .minimal-pill {
    width: 16px;
    height: 16px;
    background-color: var(--main);
    border-radius: 50%;
    transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    margin: 4px;
  }

  .minimal-track.on .minimal-pill {
    transform: translateX(26px);
    background-color: var(--bg-primary);
  }

  /* Labeled mode styles */
  .pill-indicator {
    position: absolute;
    top: 2px;
    left: 2px;
    height: calc(100% - 4px);
    width: calc(50% - 3px);
    background-color: var(--main);
    border-radius: 18px;
    transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    z-index: 0;
  }

  .pill-indicator.on {
    transform: translateX(calc(100% + 2px));
  }

  .pill-switch input[type="checkbox"] {
    display: none;
  }

  .pill-switch label {
    position: relative;
    z-index: 1;
    padding: 0;
    border-radius: 18px;
    cursor: pointer;
    transition: color 0.3s ease;
    background-color: transparent;
    color: var(--color);
    font-size: 14px;
    text-align: center;
    user-select: none;
    flex: 1;
  }

  .pill-switch label.active {
    color: var(--bg-primary);
  }

  .pill-switch label:hover {
    opacity: 0.8;
  }

  .minimal-track {
    padding: 0;
    margin: 0;
  }
</style>
