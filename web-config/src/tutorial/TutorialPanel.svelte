<script lang="ts">
  import { fly, fade } from "svelte/transition";
  import { ChevronLeft, ChevronRight, X, ChevronsRight } from "lucide-svelte";
  import type { TutorialStep, TutorialChapter } from "./types";

  export let step: TutorialStep | null = null;
  export let chapters: TutorialChapter[] = [];
  export let currentChapterIndex: number = 0;
  export let stepNumberInChapter: number = 1;
  export let totalStepsInChapter: number = 1;
  export let isLastStepInChapter: boolean = false;
  export let isLastChapter: boolean = false;
  export let isLastStepOverall: boolean = false;

  export let onNext: () => void = () => {};
  export let onPrev: () => void = () => {};
  export let onSkipChapter: () => void = () => {};
  export let onSkipAll: () => void = () => {};
  export let onComplete: () => void = () => {};

  /** Panel position: 'top' when spotlight target is in the bottom half of the viewport. */
  export let position: "top" | "bottom" = "bottom";
</script>

{#if step}
  <div
    class="tutorial-panel"
    class:panel-top={position === "top"}
    class:panel-bottom={position === "bottom"}
    in:fly={{ y: position === "bottom" ? 80 : -80, duration: 300 }}
    out:fade={{ duration: 200 }}
  >
    <!-- Chapter progress dots -->
    <div class="chapter-dots">
      {#each chapters as chapter, ci}
        <div class="chapter-dot-group">
          {#each chapter.stepIds as _sid, si}
            {@const isPast = ci < currentChapterIndex}
            {@const isCurrent = ci === currentChapterIndex}
            {@const isFuture = ci > currentChapterIndex}
            <span
              class="dot"
              class:filled={isPast ||
                (isCurrent && si < stepNumberInChapter - 1)}
              class:current={isCurrent && si === stepNumberInChapter - 1}
              class:future={isFuture}
            />
          {/each}
        </div>
        {#if ci < chapters.length - 1}
          <span class="chapter-sep" />
        {/if}
      {/each}
    </div>

    <!-- Chapter title -->
    <div class="chapter-title">
      {chapters[currentChapterIndex]?.title ?? ""}
    </div>

    <!-- Step content -->
    <div class="step-body">
      {#if step.icon}
        <span class="step-icon">{step.icon}</span>
      {/if}
      <div class="step-number">
        Step {stepNumberInChapter} of {totalStepsInChapter}
      </div>
      <h3 class="step-title">{step.title}</h3>
      <p class="step-desc">{step.description}</p>
    </div>

    <!-- Navigation -->
    <div class="nav-row">
      <button class="nav-btn skip-all" on:click={onSkipAll}>
        <X size={14} /> Skip All
      </button>

      {#if !isLastChapter}
        <button class="nav-btn skip-chapter" on:click={onSkipChapter}>
          <ChevronsRight size={14} /> Skip Chapter
        </button>
      {:else}
        <span class="nav-spacer" />
      {/if}

      <div class="nav-arrows">
        {#if stepNumberInChapter > 1 || currentChapterIndex > 0}
          <button class="nav-btn prev" on:click={onPrev}>
            <ChevronLeft size={16} /> Back
          </button>
        {/if}

        {#if isLastStepOverall}
          <button class="nav-btn complete" on:click={onComplete}>
            Got it! ✓
          </button>
        {:else}
          <button class="nav-btn next" on:click={onNext}>
            {isLastStepInChapter && !isLastChapter ? "Next Chapter" : "Next"}
            <ChevronRight size={16} />
          </button>
        {/if}
      </div>
    </div>
  </div>
{/if}

<style>
  .tutorial-panel {
    position: fixed;
    left: 50%;
    transform: translateX(-50%);
    z-index: 110;
    width: calc(100vw - 16px);
    max-width: 560px;
    background: var(--bg-primary, #232024);
    border: 1px solid var(--main, #f2fd97);
    padding: 16px 20px 12px;
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  /* ── Chapter dots ──────────────────────────────── */
  .chapter-dots {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0;
  }
  .chapter-dot-group {
    display: flex;
    gap: 4px;
  }
  .chapter-sep {
    width: 12px;
    height: 1px;
    background: var(--bg-tertiary, #6e6e6e);
    margin: 0 6px;
  }
  .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: var(--bg-tertiary, #6e6e6e);
    transition: background 0.2s;
  }
  .dot.filled {
    background: var(--main, #f2fd97);
  }
  .dot.current {
    background: var(--main, #f2fd97);
    box-shadow: 0 0 6px var(--main, #f2fd97);
  }
  .dot.future {
    background: var(--bg-secondary, #353535);
  }

  /* ── Chapter title ─────────────────────────────── */
  .chapter-title {
    text-align: center;
    font-size: 0.75rem;
    text-transform: uppercase;
    letter-spacing: 0.08em;
    color: var(--text-color-secondary, #5b585c);
  }

  /* ── Step body ─────────────────────────────────── */
  .step-body {
    text-align: center;
  }
  .step-icon {
    font-size: 1.8rem;
    display: block;
    margin-bottom: 4px;
  }
  .step-number {
    font-size: 0.7rem;
    color: var(--text-color-secondary, #5b585c);
    margin-bottom: 2px;
  }
  .step-title {
    margin: 0 0 6px;
    font-size: 1.1rem;
    color: var(--text-color, #faf8fe);
  }
  .step-desc {
    margin: 0;
    font-size: 0.85rem;
    color: var(--text-color-secondary, #5b585c);
    line-height: 1.45;
    max-height: 4.5em;
    overflow-y: auto;
  }

  /* ── Navigation ────────────────────────────────── */
  .nav-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
  }
  .nav-btn {
    all: unset;
    cursor: pointer;
    display: flex;
    align-items: center;
    gap: 4px;
    font-size: 0.8rem;
    padding: 6px 10px;
    border-radius: 8px;
    transition: background 0.15s;
    color: var(--text-color-secondary, #5b585c);
    white-space: nowrap;
  }
  .nav-btn:hover {
    background: var(--bg-secondary, #353535);
    color: var(--text-color, #faf8fe);
  }
  .skip-all {
    color: var(--red, #e33333);
  }
  .skip-chapter {
    color: var(--text-color-secondary, #5b585c);
  }
  .nav-spacer {
    flex: 1;
  }
  .nav-arrows {
    display: flex;
    gap: 4px;
    margin-left: auto;
  }
  .prev {
    color: var(--text-color-secondary, #5b585c);
  }
  .next,
  .complete {
    color: var(--main, #f2fd97);
    font-weight: 600;
  }
  .complete {
    background: var(--bg-secondary, #353535);
    padding: 6px 14px;
  }

  /* ── Position variants ──────────────────────────── */
  .panel-bottom {
    bottom: 0;
    border-radius: 16px 16px 0 0;
    box-shadow: 0 -4px 24px rgba(0, 0, 0, 0.5);
  }
  .panel-top {
    top: 0;
    border-radius: 0 0 16px 16px;
    box-shadow: 0 4px 24px rgba(0, 0, 0, 0.5);
  }
</style>
