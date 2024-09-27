<script lang="ts">
  export let title = "";
  let isHovered = false;
  let x: number;
  let y: number;

  function mouseOver(event: MouseEvent) {
    isHovered = true;
    const {
      x: bx,
      y: by,
      width,
      height,
    } = event.target!.getBoundingClientRect();
    x = event.clientX; //+ width / 2 + 5;
    y = event.clientY; // + height / 2 + 5;
    console.log("x", x, "y", y);
  }
  function mouseMove(event: MouseEvent) {
    const {
      x: bx,
      y: by,
      width,
      height,
    } = event.target!.getBoundingClientRect();
    x = event.clientX; //+ width / 2 + 5;
    y = event.clientY; // + height / 2 + 5;
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

{#if isHovered}
  <div style="transform: translate({x}px, {y}px)" class="tooltip">{title}</div>
{/if}

<style>
  .tooltip {
    /* transform: translate(-50%, 0); */
    border: 1px solid #ddd;
    box-shadow: 1px 1px 1px #ddd;
    background: white;
    border-radius: 4px;
    padding: 4px;
    position: absolute;
    top: 0;
    left: 0;
  }
</style>
