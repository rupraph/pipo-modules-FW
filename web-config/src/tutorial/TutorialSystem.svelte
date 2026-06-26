<script lang="ts">
  import { get } from "svelte/store";
  import SpotlightOverlay from "./SpotlightOverlay.svelte";
  import TutorialPanel from "./TutorialPanel.svelte";
  import {
    tutorialActive,
    tutorialSteps,
    tutorialChapters,
    tutorialStep,
    tutorialMode,
    currentStep,
    stepNumberInChapter,
    totalStepsInChapter,
    isLastStepInChapter,
    isLastStepOverall,
    currentChapterIndex,
    totalChapters,
    isLastChapter,
    tutorialService,
  } from "./store";
  import { pipoType } from "../services/config";
  import type { TutorialStep, TutorialChapter } from "./types";

  // ── Filter steps by current board variant ─────────────────────────
  function filterByVariant(steps: TutorialStep[]): TutorialStep[] {
    const type = get(pipoType);
    return steps.filter((s) => !s.variant || s.variant.includes(type));
  }

  // ── Filter chapters to only those with at least one valid step ────
  function filterChapters(
    chapters: TutorialChapter[],
    validStepIds: Set<string>,
  ): TutorialChapter[] {
    return chapters.filter((ch) =>
      ch.stepIds.some((id) => validStepIds.has(id)),
    );
  }

  // ── Apply variant filtering on mount ──────────────────────────────
  $: if ($tutorialActive) {
    const rawSteps = get(tutorialSteps);
    const rawChapters = get(tutorialChapters);

    const filteredSteps = filterByVariant(rawSteps);
    const validIds = new Set(filteredSteps.map((s) => s.id));
    const filteredChapters = filterChapters(rawChapters, validIds);

    if (filteredSteps.length === 0) {
      // No valid steps for this variant — dismiss
      tutorialService.dismiss();
    } else if (
      filteredSteps.length !== rawSteps.length ||
      filteredChapters.length !== rawChapters.length
    ) {
      tutorialSteps.set(filteredSteps);
      tutorialChapters.set(filteredChapters);
      tutorialStep.set(0);
    }
  }

  // ── Derived values ────────────────────────────────────────────────
  $: step = $currentStep;
  $: target = step?.target ?? null;

  // ── Navigation handlers ───────────────────────────────────────────
  function nextStep() {
    const steps = get(tutorialSteps);
    const idx = get(tutorialStep);
    if (idx < steps.length - 1) {
      tutorialStep.set(idx + 1);
    }
  }

  function prevStep() {
    const idx = get(tutorialStep);
    if (idx > 0) {
      tutorialStep.set(idx - 1);
    }
  }

  function skipChapter() {
    const steps = get(tutorialSteps);
    const idx = get(tutorialStep);
    const currentCh = steps[idx]?.chapterId;

    // Find first step of next chapter
    let nextIdx = idx;
    while (nextIdx < steps.length && steps[nextIdx].chapterId === currentCh) {
      nextIdx++;
    }

    if (nextIdx < steps.length) {
      tutorialStep.set(nextIdx);
    } else {
      // No next chapter — complete
      handleComplete();
    }
  }

  function handleSkipAll() {
    const mode = get(tutorialMode);
    if (mode === "replay") {
      tutorialService.dismiss();
    } else {
      tutorialService.skip();
    }
  }

  function handleComplete() {
    const mode = get(tutorialMode);
    if (mode === "replay") {
      tutorialService.dismiss();
    } else {
      tutorialService.complete();
    }
  }
</script>

{#if $tutorialActive && step}
  <SpotlightOverlay targetSelector={target} active={$tutorialActive} />

  <TutorialPanel
    {step}
    chapters={$tutorialChapters}
    currentChapterIndex={$currentChapterIndex}
    stepNumberInChapter={$stepNumberInChapter}
    totalStepsInChapter={$totalStepsInChapter}
    isLastStepInChapter={$isLastStepInChapter}
    isLastChapter={$isLastChapter}
    isLastStepOverall={$isLastStepOverall}
    onNext={nextStep}
    onPrev={prevStep}
    onSkipChapter={skipChapter}
    onSkipAll={handleSkipAll}
    onComplete={handleComplete}
  />
{/if}
