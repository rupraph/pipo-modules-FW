import { writable, get, derived } from 'svelte/store';
import { pipoio } from '../pipoio';
import { pipoInfo } from '../services';
import type { PipoInfo } from '../types';
import type { TutorialStep, TutorialChapter, TutorialMode } from './types';
import { coreChapters, coreSteps, whatsNewSteps } from './steps';

// ── localStorage keys (follows ui-state/store.ts pattern) ──────────
const STORAGE_CORE = 'pipo-tutorial-core-completed';
const STORAGE_VERSION = 'pipo-tutorial-last-version';

function loadCoreCompleted(): boolean {
  try {
    return localStorage.getItem(STORAGE_CORE) === 'true';
  } catch {
    return false;
  }
}
function loadLastVersion(): string | null {
  try {
    return localStorage.getItem(STORAGE_VERSION);
  } catch {
    return null;
  }
}
function saveCoreCompleted(): void {
  try {
    localStorage.setItem(STORAGE_CORE, 'true');
  } catch {
    /* noop */
  }
}
function saveLastVersion(version: string): void {
  try {
    localStorage.setItem(STORAGE_VERSION, version);
  } catch {
    /* noop */
  }
}

// ── Svelte stores ──────────────────────────────────────────────────

/** Is the tutorial overlay currently showing? */
export const tutorialActive = writable<boolean>(false);

/** Which tutorial mode is active (null = not active). */
export const tutorialMode = writable<TutorialMode>(null);

/** Current step index (0-based, relative to tutorialSteps). */
export const tutorialStep = writable<number>(0);

/** ID of the panel the tutorial wants open (null = close all). */
export const tutorialPanelOpen = writable<string | null>(null);

/** Steps for the current tutorial session. */
export const tutorialSteps = writable<TutorialStep[]>([]);

/** Chapters for the current session (filtered to those with valid steps). */
export const tutorialChapters = writable<TutorialChapter[]>([]);

// ── Derived stores ─────────────────────────────────────────────────

/** The current step object. */
export const currentStep = derived(
  [tutorialSteps, tutorialStep],
  ([$steps, $step]) => $steps[$step] ?? null,
);

/** 1-based step number within the current chapter. */
export const stepNumberInChapter = derived(
  [tutorialSteps, tutorialStep, currentStep],
  ([$steps, $step, $current]) => {
    if (!$current) return 1;
    const chapterSteps = $steps.filter((s) => s.chapterId === $current.chapterId);
    return chapterSteps.findIndex((s) => s.id === $current.id) + 1;
  },
);

/** Total steps in the current chapter. */
export const totalStepsInChapter = derived(
  [tutorialSteps, currentStep],
  ([$steps, $current]) => {
    if (!$current) return 1;
    return $steps.filter((s) => s.chapterId === $current.chapterId).length;
  },
);

/** Whether the current step is the last in its chapter. */
export const isLastStepInChapter = derived(
  [stepNumberInChapter, totalStepsInChapter],
  ([$num, $total]) => $num === $total,
);

/** Whether the current step is the last overall. */
export const isLastStepOverall = derived(
  [tutorialSteps, tutorialStep],
  ([$steps, $step]) => $step === $steps.length - 1,
);

/** 1-based chapter index. */
export const currentChapterIndex = derived(
  [tutorialChapters, currentStep],
  ([$chapters, $current]) => {
    if (!$current || $chapters.length === 0) return 0;
    return $chapters.findIndex((ch) => ch.id === $current.chapterId);
  },
);

/** Total number of chapters in this session. */
export const totalChapters = derived(tutorialChapters, ($ch) => $ch.length);

/** Is the current chapter the last? */
export const isLastChapter = derived(
  [currentChapterIndex, totalChapters],
  ([$idx, $total]) => $idx === $total - 1,
);

// ── TutorialService (follows ConfigService pattern) ─────────────────

function filterValidSteps(steps: TutorialStep[]): TutorialStep[] {
  // In a browser context we can't access $pipoType easily in a non-component.
  // Variant filtering is done at activation time in TutorialSystem.
  return steps;
}

class TutorialService {
  /**
   * Check if tutorial should auto-show after /info fetch.
   * Called from App.svelte's fetch() .then() block.
   */
  checkAutoShow(info: PipoInfo): void {
    const coreCompleted = loadCoreCompleted();
    const lastVersion = loadLastVersion();

    if (!info.initialized && !coreCompleted) {
      // First ever boot → show core tutorial
      const steps = filterValidSteps(coreSteps);
      if (steps.length === 0) return;

      tutorialMode.set('core');
      tutorialSteps.set(steps);
      tutorialChapters.set(coreChapters);
      tutorialStep.set(0);
      tutorialActive.set(true);
    } else if (lastVersion && info.version !== lastVersion) {
      // Firmware upgraded → show what's new for this version
      const newSteps = whatsNewSteps[info.version];
      if (newSteps && newSteps.length > 0) {
        const steps = filterValidSteps(newSteps);
        if (steps.length === 0) return;

        tutorialMode.set('whats-new');
        tutorialSteps.set(steps);
        // What's new has no chapters — treat as single chapter
        tutorialChapters.set([{ id: 'whats-new', title: "What's New", stepIds: steps.map((s) => s.id) }]);
        tutorialStep.set(0);
        tutorialActive.set(true);
      }
      // Update last-seen version even if no what's-new steps
    }
  }

  /** Mark tutorial as complete (called on "Got it!" or "Skip All"). */
  async complete(): Promise<void> {
    tutorialPanelOpen.set(null);
    try {
      await pipoio.request({ method: 'post', url: '/tutorial-complete' });
    } catch (e) {
      console.warn('Failed to POST /tutorial-complete:', e);
      // Non-fatal — localStorage is the fallback
    }

    saveCoreCompleted();
    const info = get(pipoInfo);
    if (info?.version) {
      saveLastVersion(info.version);
    }

    tutorialActive.set(false);
    tutorialMode.set(null);
  }

  /** Skip tutorial (same backend effect as complete). */
  async skip(): Promise<void> {
    await this.complete();
  }

  /** Replay tutorial from menu — does NOT reset backend flag. */
  replay(): void {
    const steps = filterValidSteps(coreSteps);
    tutorialMode.set('replay');
    tutorialSteps.set(steps);
    tutorialChapters.set(coreChapters);
    tutorialStep.set(0);
    tutorialActive.set(true);
  }

  /** Dismiss without marking complete (used during replay). */
  dismiss(): void {
    tutorialPanelOpen.set(null);
    tutorialActive.set(false);
    tutorialMode.set(null);
  }
}

export const tutorialService = new TutorialService();
