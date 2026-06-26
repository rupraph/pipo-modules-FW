import type { PipoTypes } from '../types';

/**
 * A single step in a tutorial chapter.
 *
 * `target` references a `data-tutorial="<id>"` attribute on a DOM element.
 * Set to `null` for center-screen steps with no spotlight.
 * `variant` filters which board types the step appears for (omit = all variants).
 */
export interface TutorialStep {
  id: string;
  chapterId: string;
  target: string | null;
  title: string;
  description: string;
  icon?: string;
  variant?: PipoTypes[];
}

/**
 * A named group of tutorial steps. Steps are defined separately and linked
 * via `stepIds` — this keeps the step array flat for simple index-based
 * navigation while still supporting chapter grouping.
 */
export interface TutorialChapter {
  id: string;
  title: string;
  stepIds: string[];
}

/** Which tutorial mode is currently active. */
export type TutorialMode = 'core' | 'whats-new' | 'replay' | null;
