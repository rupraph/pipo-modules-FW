<script lang="ts">
  export let onClick: (name: string) => void;
  const items = [
    { value: "MIDI", label: "MIDI" },
    { value: "OSC", label: "OSC" },
    // { value: "HID", label: "HID" },
  ];
  export let active: string;
  function itemClick(item: string) {
    onClick(item);
  }
</script>

<div class="cat-tabs hero">
  {#each items as { value, label }}
    <span
      class="tab"
      class:active={value === active}
      on:click={() => itemClick(value)}
    >
      {label}
    </span>
  {/each}
</div>

<style>
  .cat-tabs {
    width: 100%;
    display: grid;
    grid-template-columns: 33% 33% 33%;
    grid-template-rows: auto;
    /* background-color: var(--bg-tertiary); */
    font-size: 1.2em;
    font-weight: bold;
    /* padding: 0.8em 0; */
    margin-top: 1em;
  }
  .cat-tabs > .tab {
    display: flex;
    align-items: center;
    justify-content: center;
    position: relative;
    border: 0;
    line-height: 1em;
    padding: 0.8em 0;
    border-top-left-radius: 0.8em;
    border-top-right-radius: 0.8em;
  }
  .cat-tabs > .tab.active {
    cursor: text;
    color: var(--main);
    background-color: var(--bg-tabs);
  }
</style>
