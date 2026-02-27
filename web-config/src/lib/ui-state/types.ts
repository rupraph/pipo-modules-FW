import type { PipoTypes } from "../../types";

/**
 * UI state for a specific board type
 * Stores which collapse sections are open and which channel is selected
 */
export type BoardUIState = {
  collapses: {
    [collapseId: string]: boolean; // true = open, false = closed
  };
  selectedChannel?: string; // currently selected channel in Channel settings
  channelType?: string; // current channel type category (e.g., 'euler', 'analog', 'touch')
};

/**
 * All UI states organized by board type
 */
export type UIState = {
  [K in PipoTypes]?: BoardUIState;
  advancedMode?: boolean; // Global advanced mode toggle
};
