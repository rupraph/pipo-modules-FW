<script lang="ts">
  export let onClick: (name: string) => void;
  const items = [
    { value: "MIDI", label: "MIDI" },
    { value: "OSC", label: "OSC" },
    { value: "HID", label: "HID" },
  ];
  export let active: string;
  function itemClick(item: string) {
    onClick(item);
  }
</script>

<!-- <h4 class="title">Output conversion settings</h4> -->
<div class="cat-tabs hero">
  {#each items as { value, label }}
    <span
      class="tab {value === active ? 'active' : ''}"
      on:click={() => itemClick(value)}
    >
      {label}
    </span>
  {/each}
</div>

<style>
  .title {
    text-align: center;
    height: 0em;
    background-color: var(--bg-tertiary);
  }

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

  /* .cat-tabs > .tab::after {
    content: "";
    position: absolute;
    top: 0;
    right: 0;
    width: 1px;
    height: 100%;
    background-color: black;
  }

  .cat-tabs > .tab:last-child::after {
    display: none;
  } */

  .cat-tabs > .tab.active {
    cursor: text;
    color: var(--main-darker);
    background-color: var(--bg-tabs);
  }
</style>
