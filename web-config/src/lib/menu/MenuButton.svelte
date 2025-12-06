<script lang="ts">
  export let border = false;
  export let secondary = false;
</script>

<button
 class:border={border}
 class:secondary={secondary}
on:click>
    <slot></slot>
</button>

<style>
button {
 width: 20px;
 height: 20px;
 border: none;
 border-radius: 50%;
 padding: 0;
 background-color: transparent;
}
button.secondary {
  background-color: var(--grey);
  color: var(--bg-primary);
}
button.border {
  border: 1px solid var(--main);
}
button:hover {
  background-color: var(--bg-secondary);
  cursor: pointer;
}
button.secondary:hover {
  background-color: var(--bg-tertiary);
}
button>*{
  width: 100%;
  height: 100%;
}
</style>