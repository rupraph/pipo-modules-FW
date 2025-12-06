<script lang="ts">
  import { teleport } from "./index";
  export let title = "";
  export let enabled = true;
  export let followCursor = false;
  let isHovered = false;
  let x: number;
  let y: number;

  function refresh(event: MouseEvent) {
    isHovered = true;
    if (followCursor) {
      x = event.pageX + 5;
      y = event.pageY + 5;
      return;
    }
    const {
      x: bx,
      y: by,
      height,
    } = (event.currentTarget as HTMLDivElement).getBoundingClientRect();

    x = bx;
    y = by + height + 5;
  }
  function mouseOver(event: MouseEvent) {
    isHovered = true;
    refresh(event);
  }
  function mouseMove(event: MouseEvent) {
    refresh(event);
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
    background-color: var(--bg-primary);
    border-radius: 4px;
    padding: 4px;
    position: fixed;
    top: 0;
    left: 0;
    z-index: 1000;
  }
</style>
