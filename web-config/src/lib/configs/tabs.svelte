<script lang="ts">
  export let onClick: (name: string) => void;
  export let onCreate: () => void;
  export let onRename: (oldName: string, newName: string) => void;
  export let items: string[];
  export let active: string;
  let newname = "";
  let renaming = false;
  function itemClick(item: string) {
    if (item === active) {
      newname = item;
      renaming = true;
      return;
    }
    onClick(item);
  }
  function onkeyup(e: KeyboardEvent) {
    if (e.key === "Enter") {
      onRename(active, newname.trim());
      renaming = false;
    }
  }
</script>

<div class="configs">
  <ul class="tabs">
    {#each items as item}
      <li
        class="tab {item === active ? 'active' : ''}"
        on:click={() => itemClick(item)}
      >
        {#if item === active && renaming}
          <span contenteditable bind:textContent={newname} on:keyup={onkeyup}>
            {newname}
          </span>
        {:else}
          <span>
            {item}
          </span>
        {/if}
      </li>
    {/each}
    <li class="tab new" on:click={() => onCreate()}><span>+</span></li>
  </ul>
  <div class="content">
    <slot />
  </div>
</div>

<style>
  .configs {
    width: 100%;
    display: grid;
    grid-template-rows: 2em auto;
  }
  .tabs {
    gap: 10px;
    margin-left: 25px;
    max-height: 2em;
  }
  .tab {
    display: grid;
    grid-template-columns: 8px 8px auto 8px 8px;
    grid-template-rows: 100%;
    border: 0;
    line-height: 2em;
  }

  .tab.active {
    z-index: 12;
    cursor: text;
  }
  .tab > input {
    padding: 0;
    border: none;
    border-radius: 0;
    height: unset;
    width: 0;
  }

  .tab.active > span[contenteditable] {
    outline: 0px solid transparent;
  }
  .tab.active > span {
    padding-bottom: 3px;
  }
  .tab > span {
    background-color: var(--bg-color);
    grid-area: 1 / 3 / 4 / 5;
    font-size: 1.2em;
    border-top: 2px solid white;
    min-width: 50px;
    padding: 0 10px;
    z-index: 10;
  }
  .tab::before,
  .tab::after {
    content: "";
    height: 100%;
    border-top: 2px solid white;
    min-width: 16px;
    background: var(--bg-color);
  }
  .tab:after {
    grid-area: 1 / 4 / 2 / 6;
    content: "";
    transform: skew(25deg);
    border-radius: 0 8px 0 0;
    border-right: 2px solid white;
    padding-left: 6px;
  }
  .tab:before {
    grid-area: 1 / 1 / 2 / 3;
    border-radius: 8px 0 0 0;
    transform: skew(-25deg);
    border-radius: 8px 0 0 0;
    border-left: 2px solid white;
    padding-right: 6px;
  }
  .tab.new > span {
    min-width: 16px;
  }
  .content {
    z-index: 10;
    border-radius: 10px;
    border: 2px solid white;
    margin-top: 4px;
    background: var(--bg-color);
    transform: translate(0, -1px);
    padding: 1em;
  }
</style>
