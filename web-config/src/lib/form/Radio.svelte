<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { uid } from "../../utils";
  const dispatch = createEventDispatcher();
  let id = uid();
  export let label: string = "";
  export let options: { label: string; value: string | number }[] = [];
  export let value: string | number;
  const slugify = (str = "") =>
    str.toLowerCase().replace(/ /g, "-").replace(/\./g, "") + id;
</script>

<span class="input columns">
  <label>{label}</label>
  {#each options as { label, value: v }, i}
    <span class="radio-wrapper">
      <input
        type="radio"
        id={slugify(label)}
        name={id}
        value={v}
        checked={+v === value}
        on:change={() => dispatch("change", +v)}
      />
      <label for={slugify(label)}>{label}</label>
    </span>
  {/each}
</span>

<style>
  .radio {
    display: grid;
    grid-auto-flow: column;
    grid-template-columns: auto;
  }
  .radio-wrapper {
    display: grid;
    grid-template-rows: auto;
    grid-template-columns: 20px auto;
    text-align: start;
    align-items: center;
    gap: 15px;
  }
  .radio-wrapper input {
    width: max-content;
    grid-area: 1 / 1 / 2 / 2;
  }

  .radio-wrapper label {
    width: 100%;
    cursor: pointer;
    grid-area: 1 / 2 / 2 / 3;
  }
  .radio-wrapper:hover > label {
    text-decoration: underline;
  }
</style>
