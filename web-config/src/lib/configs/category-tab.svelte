<script lang="ts">
  export let onClick: (name: string) => void;
  export let items: { value: string; label: string }[];
  export let active: string;
  function itemClick(item: string) {
    onClick(item);
  }
</script>

<div class="cat-tabs">
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
  .cat-tabs {
    width: 100%;
    display: grid;
    grid-template-columns: 33% 33% 33%;
    grid-template-rows: auto;
    margin: 1em 0;
  }
  .cat-tabs > .tab {
    grid-template-columns: 8px 8px auto 8px 8px;
    grid-template-rows: 100%;
    border: 0;
    line-height: 2em;
  }

  .cat-tabs > .tab.active {
    cursor: text;
    color: rgb(2, 141, 176);
  }
</style>
