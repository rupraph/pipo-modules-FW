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

<h4 class="title">Output conversion settings</h4>
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
    background-color: var(--bg-tertiary);
    font-size: 1.2em;
    font-weight: bold;
    padding: 0.5em 0;
  }
  .cat-tabs > .tab {
    grid-template-columns: 8px 8px auto 8px 8px;
    grid-template-rows: 100%;
    border: 0;
    line-height: 1em;
  }

  .cat-tabs > .tab.active {
    cursor: text;
    color: var(--main);
  }
</style>
