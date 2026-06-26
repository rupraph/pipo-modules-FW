<script lang="ts">
  import { onMount, onDestroy } from "svelte";

  export let targetSelector: string | null = null;
  export let active = false;

  let holeStyle: Record<string, string> = {};
  let visible = false;
  let targetEl: HTMLElement | null = null;
  let resizeObserver: ResizeObserver | null = null;
  let scrollTimer: number | undefined;

  function findTarget(): HTMLElement | null {
    if (!targetSelector) return null;
    return document.querySelector(`[data-tutorial="${targetSelector}"]`);
  }

  function updatePosition() {
    targetEl = findTarget();
    if (!targetEl) {
      visible = false;
      holeStyle = {};
      return;
    }

    const rect = targetEl.getBoundingClientRect();
    const padding = 4; // extra padding around the spotlight hole

    holeStyle = {
      top: `${rect.top - padding}px`,
      left: `${rect.left - padding}px`,
      width: `${rect.width + padding * 2}px`,
      height: `${rect.height + padding * 2}px`,
      "border-radius": "8px",
    };
    visible = true;
  }

  function onResizeOrScroll() {
    if (scrollTimer) clearTimeout(scrollTimer);
    scrollTimer = window.setTimeout(updatePosition, 50);
  }

  onMount(() => {
    updatePosition();
    resizeObserver = new ResizeObserver(() => updatePosition());
    if (targetEl) resizeObserver.observe(targetEl);
    window.addEventListener("resize", onResizeOrScroll);
    window.addEventListener("scroll", onResizeOrScroll, { passive: true });
  });

  onDestroy(() => {
    resizeObserver?.disconnect();
    window.removeEventListener("resize", onResizeOrScroll);
    window.removeEventListener("scroll", onResizeOrScroll);
    if (scrollTimer) clearTimeout(scrollTimer);
  });

  // Re-observe when target changes
  $: if (targetSelector && active) {
    resizeObserver?.disconnect();
    updatePosition();
    targetEl = findTarget();
    if (targetEl) resizeObserver?.observe(targetEl);
    else resizeObserver?.observe(document.body);
  }

  $: if (!active) {
    visible = false;
  }
</script>

{#if active && visible}
  <!-- svelte-ignore a11y-no-static-element-interactions -->
  <div class="spotlight-overlay" />
  <div
    class="spotlight-hole"
    style={Object.entries(holeStyle)
      .map(([k, v]) => `${k}:${v}`)
      .join(";")}
  />
{/if}

<style>
  .spotlight-overlay {
    position: fixed;
    inset: 0;
    z-index: 100;
    background: rgba(0, 0, 0, 0.55);
    pointer-events: none;
  }

  .spotlight-hole {
    position: fixed;
    z-index: 101;
    pointer-events: none;
    box-shadow:
      0 0 0 9999px rgba(0, 0, 0, 0.55),
      0 0 24px 4px var(--main, #f2fd97);
    transition: all 0.25s ease;
  }
</style>
