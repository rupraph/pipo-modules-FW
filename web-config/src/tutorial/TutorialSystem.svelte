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
    tutorialPanelOpen,
  } from "./store";
  import { pipoType } from "../services/config";
  import type { TutorialStep, TutorialChapter } from "./types";

  function filterByVariant(steps: TutorialStep[]): TutorialStep[] {
    const type = get(pipoType);
    return steps.filter((s) => !s.variant || s.variant.includes(type));
  }
  function filterChapters(
    chapters: TutorialChapter[],
    validStepIds: Set<string>,
  ): TutorialChapter[] {
    return chapters.filter((ch) =>
      ch.stepIds.some((id) => validStepIds.has(id)),
    );
  }

  // Apply variant filtering
  $: if ($tutorialActive) {
    const rawSteps = get(tutorialSteps);
    const rawChapters = get(tutorialChapters);
    const filteredSteps = filterByVariant(rawSteps);
    const validIds = new Set(filteredSteps.map((s) => s.id));
    const filteredChapters = filterChapters(rawChapters, validIds);
    if (filteredSteps.length === 0) {
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

  $: step = $currentStep;
  $: target = step?.target ?? null;

  // ── Auto-open panels on step enter / close on leave ───────────────
  let previousStep: TutorialStep | null = null;

  $: if (step && step !== previousStep) {
    const prev = previousStep;
    const cur = step;
    previousStep = step;

    const prevAutoTarget = prev?.autoOpen ? prev.target : null;
    const curAutoTarget = cur.autoOpen ? cur.target : null;

    // Only write the store when the desired open target actually changes
    if (prevAutoTarget !== curAutoTarget) {
      tutorialPanelOpen.set(curAutoTarget ?? null);
    }
  }

  // ── Dynamic panel position (top when target is in lower half) ─────
  let panelPosition: "top" | "bottom" = "bottom";

  function computePanelPosition(): "top" | "bottom" {
    if (!target) return "bottom";
    const el = document.querySelector(`[data-tutorial="${target}"]`);
    if (!el) return "bottom";
    const rect = el.getBoundingClientRect();
    return rect.top + rect.height / 2 > window.innerHeight / 2
      ? "top"
      : "bottom";
  }

  $: if (step) panelPosition = computePanelPosition();

  // ── Navigation ────────────────────────────────────────────────────
  function nextStep() {
    const idx = get(tutorialStep);
    if (idx < get(tutorialSteps).length - 1) tutorialStep.set(idx + 1);
  }
  function prevStep() {
    const idx = get(tutorialStep);
    if (idx > 0) tutorialStep.set(idx - 1);
  }
  function skipChapter() {
    const steps = get(tutorialSteps);
    const idx = get(tutorialStep);
    const currentCh = steps[idx]?.chapterId;
    let nextIdx = idx;
    while (nextIdx < steps.length && steps[nextIdx].chapterId === currentCh)
      nextIdx++;
    if (nextIdx < steps.length) tutorialStep.set(nextIdx);
    else handleComplete();
  }
  function handleSkipAll() {
    tutorialPanelOpen.set(null);
    get(tutorialMode) === "replay"
      ? tutorialService.dismiss()
      : tutorialService.skip();
  }
  function handleComplete() {
    tutorialPanelOpen.set(null);
    get(tutorialMode) === "replay"
      ? tutorialService.dismiss()
      : tutorialService.complete();
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
    position={panelPosition}
    onNext={nextStep}
    onPrev={prevStep}
    onSkipChapter={skipChapter}
    onSkipAll={handleSkipAll}
    onComplete={handleComplete}
  />
{/if}
