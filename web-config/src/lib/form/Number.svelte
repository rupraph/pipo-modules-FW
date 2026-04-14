<script lang="ts">
  import { throttle, uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let min: number = -Infinity;
  export let max: number = Infinity;
  export let step: number = 1;
  export let value: number = 0;
  let id = uid();
  let interval: ReturnType<typeof setInterval> | 0 = 0;
  let timeout: ReturnType<typeof setTimeout> | 0 = 0;
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
    // Handle null, undefined, NaN, or empty string
    if (value == null || isNaN(value) || value === "") {
      value = min;
      return;
    }
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
  $: {
    // Handle null, undefined, NaN, or empty string
    if (value == null || isNaN(value) || value === "") {
      value = min;
    } else {
      if (value < min) value = min;
      if (value > max) value = max;
    }
  }
  // Compute input width based on the widest possible value (max digits or min digits if negative)
  $: inputChars = Math.max(
    String(max).length,
    String(min).length,
    2
  );
</script>

<Input class={"number-input-container "} {label} {id} --input-chars="{inputChars}">
  <button
    class:disabled={value <= min}
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
  <input
    type="number"
    class="number-input"
    {id}
    name={label}
    {min}
    {max}
    {step}
    bind:value
  />
  <button
    class:disabled={value >= max}
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
  :global(.input.number-input-container .input-wrapper) {
    background-color: var(--bg-primary);
    border-radius: 20px;
    padding: 0;
    overflow: hidden;
    width: fit-content;
    height: 29px;
    display: grid;
    grid-template-columns: 27px calc(var(--input-chars, 3) * 1ch + 8px) 27px;
    gap: 2px;
  }
  input.number-input {
    width: 100%;
    height: 100%;
    border: none;
    background-color: var(--grey);
    text-align: center;
    margin: 0;
    padding: 0 2px;
    border-radius: 0;
  }
  input::-webkit-outer-spin-button,
  input::-webkit-inner-spin-button {
    -webkit-appearance: none;
    margin: 0;
  }

  input[type="number"] {
    -moz-appearance: textfield;
  }
  button {
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 0;
    background-color: var(--grey);
    color: var(--color);
    border: none;
    cursor: pointer;
  }
  button:hover {
    background-color: var(--bg-secondary);
  }
  button.disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
</style>
