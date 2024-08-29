<script lang="ts">
  import { createEventDispatcher } from "svelte";
  const dispatch = createEventDispatcher();
  let id = Math.random().toString(36).substring(2);
  export let label: string = "";
  export let options: { label: string; value: string | number }[] = [];
  export let value: string | number;
  const slugify = (str = "") =>
    str.toLowerCase().replace(/ /g, "-").replace(/\./g, "") + id;
</script>

<span class="input columns">
  <span>{label}</span>
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
    grid-template-columns: 20px auto;
    text-align: start;
    align-items: center;
    gap: 15px;
  }
  .radio-wrapper input {
    width: max-content;
  }

  .radio-wrapper label {
    width: 100%;
    cursor: pointer;
  }
  .radio-wrapper:hover > label {
    text-decoration: underline;
  }
</style>
