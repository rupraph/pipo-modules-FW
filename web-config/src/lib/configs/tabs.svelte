<script lang="ts">
  import LoadingButton from "../form/LoadingButton.svelte";

  export let onClick: (name: string) => void;
  export let onCreate: () => void;
  export let onRename: (oldName: string, newName: string) => void;
  export let onDelete: (name: string) => Promise<void>;
  export let items: string[];
  export let active: string;
  let newname = "";
  let renaming = false;
  let deleting = false;
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
  function onDeleteClick(toDelete: string) {
    deleting = true;
    Promise.all([onDelete(toDelete)]).finally(() => (deleting = false));
  }
</script>

<div class="configs">
  <ul class="tabs">
    {#each items as item}
      <li
        class="tab"
        class:active={item === active}
        on:click={() => itemClick(item)}
      >
        {#if item === active}
          {#if renaming}
            <span contenteditable bind:textContent={newname} on:keyup={onkeyup}>
              {newname}
            </span>
          {:else}
            <span>
              {item}
              <LoadingButton
                onClick={() => onDeleteClick(item)}
                class="delete"
                loading={deleting}
                title="Delete"
                width="1em">x</LoadingButton
              >
            </span>
          {/if}
        {:else}
          <span>
            {item}
          </span>
        {/if}
      </li>
    {/each}
    <li class="tab new" on:click={() => onCreate()}><span>+</span></li>
  </ul>
  <div class="content section-borders">
    <slot />
  </div>
</div>

<style>
  .configs {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
  }
  .tabs {
    max-width: calc(100% - 30px);
  }
  .tab {
    display: grid;
    grid-template-columns: 8px 8px auto 8px 8px;
    grid-template-rows: 100%;
    border: 0;
    line-height: 2em;
    height: 2em;
  }

  .tab.active {
    z-index: 12;
    cursor: text;
    padding-bottom: 4px;
    color: var(--text-color);
  }

  .tab.active > span[contenteditable] {
    outline: 0px solid transparent;
  }
  .tab.active > span {
    padding-bottom: 2px;
  }
  .tab > span {
    background-color: var(--bg-color);
    grid-area: 1 / 3 / 4 / 5;
    font-size: 1.2em;
    border-top: 2px solid rgb(80, 80, 80);
    min-width: 50px;
    padding: 0 10px;
    z-index: 10;
  }
  .tab::before,
  .tab::after {
    content: "";
    height: 100%;
    border-top: 2px solid rgb(80, 80, 80);
    min-width: 16px;
    background: var(--bg-color);
  }
  .tab.active::before,
  .tab.active::after {
    padding-bottom: 2px;
  }
  .tab:after {
    grid-area: 1 / 4 / 2 / 6;
    content: "";
    transform: skew(25deg);
    border-radius: 0 8px 0 0;
    border-right: 2px solid rgb(80, 80, 80);
    padding-left: 6px;
  }
  .tab:before {
    grid-area: 1 / 1 / 2 / 3;
    border-radius: 8px 0 0 0;
    transform: skew(-25deg);
    border-radius: 8px 0 0 0;
    border-left: 2px solid rgb(80, 80, 80);
    padding-right: 6px;
  }
  .tab.new > span {
    min-width: 16px;
  }
  .content {
    z-index: 10;
    padding-top: 1.5em;
    background: var(--bg-color);
    transform: translate(0, -3px);
    width: 100%;
    max-width: 100%;
  }
  :global(.tab .delete) {
    background: transparent;
    margin: 0;
    padding: 0;
    width: 1em;
    height: 1.2em;
    cursor: pointer;
    border-radius: 10%;
    /* transform: translate(-0.9em, -0.3em); */
  }
  :global(.tab .delete:hover) {
    background: var(--bg-color);
  }
</style>
