<script lang="ts">
  import { teleport } from "./index";
  export let title = "";
  export let enabled = true;
  let isHovered = false;
  let x: number;
  let y: number;

  function mouseOver(event: MouseEvent) {
    isHovered = true;
    const {
      x: bx,
      y: by,
      height,
    } = (event.currentTarget as HTMLDivElement).getBoundingClientRect();
    x = bx;
    y = by + height + 5;
  }
  function mouseMove(event: MouseEvent) {
    const {
      x: bx,
      y: by,
      width,
      height,
    } = (event.currentTarget as HTMLDivElement).getBoundingClientRect();
    x = bx;
    y = by + height + 5;
  }
  function mouseLeave() {
    isHovered = false;
  }
</script>

<div
  on:mouseover={mouseOver}
  on:mouseleave={mouseLeave}
  on:mousemove={mouseMove}
>
  <slot />
</div>
{#if isHovered && enabled}
  <div use:teleport style="transform: translate({x}px, {y}px)" class="tooltip">
    {title}
  </div>
{/if}

<style>
  .tooltip {
    /* transform: translate(-50%, 0); */
    border: 1px solid #ddd;
    box-shadow: 1px 1px 1px #ddd;
    background-color: var(--bg-color);
    border-radius: 4px;
    padding: 4px;
    position: absolute;
    top: 0;
    left: 0;
    z-index: 1000;
  }
</style>
