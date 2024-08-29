<script lang="ts">
  import Input from './Input.svelte';
  export let label: string;
  export let value: string = '';
  let id = Math.random().toString(36).substring(2);
</script>

<Input label={label} id={id}>
  <input type="text" id={id} name={label}  bind:value={value} />
</Input>

<style>

</style>