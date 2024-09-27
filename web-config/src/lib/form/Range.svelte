<script lang="ts">
  import { throttle, uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let min: number = -Infinity;
  export let max: number = Infinity;
  export let step: number = 1;
  export let value: number = 0;
  export let tooltip: string | undefined;
  let id = uid();
  let interval = 0;
  let timeout = 0;
  let keyupTimeout = 0;

  function onmousedown(delta: number) {
    if (timeout || interval) {
      clearTimeout(timeout);
      timeout = 0;
    }
    if (interval) {
      clearInterval(interval);
      interval = 0;
    }
    timeout = setTimeout(() => {
      interval = setInterval(() => {
        if (value < min || value > max) {
          clearInterval(interval);
          interval = 0;
          check();
          return;
        }
        value += delta;
      }, 100);
    }, 200);
  }
  function check() {
    if (value < min) {
      value = min;
    }
    if (value > max) {
      value = max;
    }
  }
  function onmouseup() {
    clearTimeout(timeout);
    clearInterval(interval);
    interval = 0;
    timeout = 0;
    check();
  }
  const onkeyup = throttle(() => {
    check();
  }, 100);
</script>

<Input class={$$restProps.class || ""} {label} {id} {tooltip}>
  <input type="number" {id} name={label} {min} {max} {step} bind:value />

  <button
    on:click={() => {
      value -= step;
      onmouseup();
    }}
    on:mousedown={() => onmousedown(-step)}
    on:touchstart={() => onmousedown(-step)}
    on:touchcancel={onmouseup}
    on:touchend={onmouseup}
    on:mouseleave={onmouseup}
    on:mouseup={onmouseup}
    on:focusout={onmouseup}>-</button
  >
  <button
    on:click={() => {
      value += step;
      onmouseup();
    }}
    on:mousedown={() => onmousedown(step)}
    on:touchstart={() => onmousedown(step)}
    on:touchcancel={onmouseup}
    on:touchend={onmouseup}
    on:mouseleave={onmouseup}
    on:mouseup={onmouseup}
    on:keyup={onkeyup}
    on:focusout={onmouseup}>+</button
  >
</Input>

<style>
  input::-webkit-outer-spin-button,
  input::-webkit-inner-spin-button {
    -webkit-appearance: none;
    margin: 0;
  }

  input[type="number"] {
    -moz-appearance: textfield;
  }
  button {
    margin-right: 2px;
    background-color: transparent;
    color: var(--color);
  }
  button:hover {
    color: var(--main);
  }
</style>
