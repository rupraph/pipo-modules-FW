<script lang="ts">
  import { onMount, onDestroy } from "svelte";

  export let targetSelector: string | null = null;
  export let active = false;

  // bind:this requires the element to always be in the DOM (no {#if})
  let holeEl: HTMLElement;
  let visible = false;
  let targetEl: HTMLElement | null = null;
  let resizeObserver: ResizeObserver | null = null;
  let rafPending = false;
  let animateMove = false; // true only on target change, false during scroll

  function findTarget(): HTMLElement | null {
    if (!targetSelector) return null;
    return document.querySelector(`[data-tutorial="${targetSelector}"]`);
  }

  function updatePosition() {
    targetEl = findTarget();
    if (!targetEl) {
      visible = false;
      return;
    }

    const rect = targetEl.getBoundingClientRect();
    const padding = 4;

    // Direct DOM write inside rAF — bypasses Svelte's microtask scheduler
    // so the position update lands in the same paint frame as the scroll event
    holeEl.style.top = `${rect.top - padding}px`;
    holeEl.style.left = `${rect.left - padding}px`;
    holeEl.style.width = `${rect.width + padding * 2}px`;
    holeEl.style.height = `${rect.height + padding * 2}px`;

    if (!visible) visible = true;
    if (animateMove) setTimeout(() => (animateMove = false), 280);
  }

  function scheduleUpdate() {
    if (rafPending) return;
    rafPending = true;
    requestAnimationFrame(() => {
      rafPending = false;
      updatePosition();
    });
  }

  onMount(() => {
    updatePosition();
    resizeObserver = new ResizeObserver(() => updatePosition());
    if (targetEl) resizeObserver.observe(targetEl);
    window.addEventListener("resize", scheduleUpdate);
    // capture: true catches scroll on any nested scrollable container
    window.addEventListener("scroll", scheduleUpdate, { passive: true, capture: true });
    document.addEventListener("scroll", scheduleUpdate, { passive: true, capture: true });
  });

  onDestroy(() => {
    resizeObserver?.disconnect();
    window.removeEventListener("resize", scheduleUpdate);
    window.removeEventListener("scroll", scheduleUpdate, { capture: true } as EventListenerOptions);
    document.removeEventListener("scroll", scheduleUpdate, { capture: true } as EventListenerOptions);
  });

  // Re-observe when target changes — enable transition animation
  $: if (targetSelector && active) {
    resizeObserver?.disconnect();
    animateMove = true;
    updatePosition();
    targetEl = findTarget();
    if (targetEl) resizeObserver?.observe(targetEl);
    else resizeObserver?.observe(document.body);
  }

  $: if (!active) {
    visible = false;
  }
</script>

<!-- Always in DOM so bind:this works; visibility controlled via CSS opacity -->
<div class="spotlight-overlay" class:visible={active && visible} />
<div
  class="spotlight-hole"
  class:visible={active && visible}
  class:animating={animateMove}
  bind:this={holeEl}
/>

<style>
  .spotlight-overlay {
    position: fixed;
    inset: 0;
    z-index: 100;
    background: rgba(0, 0, 0, 0.55);
    pointer-events: none;
    opacity: 0;
    transition: opacity 0.15s;
  }
  .spotlight-overlay.visible {
    opacity: 1;
  }

  .spotlight-hole {
    position: fixed;
    z-index: 101;
    pointer-events: none;
    border-radius: 8px;
    box-shadow:
      0 0 0 9999px rgba(0, 0, 0, 0.55),
      0 0 24px 4px var(--main, #f2fd97);
    opacity: 0;
    transition: opacity 0.15s;
  }
  .spotlight-hole.visible {
    opacity: 1;
  }
  /* On target change: animate position. During scroll: position updates are instant (no transition). */
  .spotlight-hole.animating {
    transition:
      opacity 0.15s,
      top 0.25s ease,
      left 0.25s ease,
      width 0.25s ease,
      height 0.25s ease;
  }
</style>
