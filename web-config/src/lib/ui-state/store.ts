import { writable, get } from "svelte/store";
import type { PipoTypes } from "../../types";
import type { UIState, BoardUIState } from "./types";
import { pipoio } from "../../pipoio";

const STORAGE_KEY = "pipo-ui-state";

/**
 * Load UI state from localStorage
 */
function loadState(): UIState {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) {
      return JSON.parse(stored);
    }
  } catch (error) {
    console.warn("Failed to load UI state from localStorage:", error);
  }
  return {};
}

/**
 * Save UI state to localStorage
 */
function saveState(state: UIState): void {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
  } catch (error) {
    console.warn("Failed to save UI state to localStorage:", error);
  }
}

/**
 * Get default state for a board type
 */
function getDefaultBoardState(): BoardUIState {
  return {
    collapses: {},
    selectedChannel: undefined,
  };
}

/**
 * Create the UI state store
 */
function createUIStateStore() {
  const { subscribe, set, update } = writable<UIState>(loadState());

  // Subscribe to changes and persist to localStorage
  subscribe((state) => {
    saveState(state);
  });

  return {
    subscribe,
    set,
    update,

    /**
     * Get the collapse state for a specific board type and collapse ID
     * Returns undefined if no persisted state exists (let component use its default)
     */
    getCollapseState: (
      boardType: PipoTypes,
      collapseId: string
    ): boolean | undefined => {
      const state = get({ subscribe });
      const boardState = state[boardType];

      if (boardState && collapseId in boardState.collapses) {
        return boardState.collapses[collapseId];
      }

      // Return undefined if not found - component will use its own default
      return undefined;
    },

    /**
     * Set the collapse state for a specific board type and collapse ID
     */
    setCollapseState: (
      boardType: PipoTypes,
      collapseId: string,
      isOpen: boolean
    ): void => {
      update((state) => {
        if (!state[boardType]) {
          state[boardType] = getDefaultBoardState();
        }
        state[boardType]!.collapses[collapseId] = isOpen;
        return state;
      });
    },

    /**
     * Get the selected channel for a specific board type
     */
    getSelectedChannel: (boardType: PipoTypes): string | undefined => {
      const state = get({ subscribe });
      return state[boardType]?.selectedChannel;
    },

    /**
     * Set the selected channel for a specific board type
     */
    setSelectedChannel: (
      boardType: PipoTypes,
      channel: string | undefined
    ): void => {
      update((state) => {
        if (!state[boardType]) {
          state[boardType] = getDefaultBoardState();
        }
        state[boardType]!.selectedChannel = channel;
        pipoio.monitorAxis(channel);
        return state;
        
      });
    },

    /**
     * Get the channel type for a specific board type
     */
    getChannelType: (boardType: PipoTypes): string | undefined => {
      const state = get({ subscribe });
      return state[boardType]?.channelType;
    },

    /**
     * Set the channel type for a specific board type
     */
    setChannelType: (
      boardType: PipoTypes,
      channelType: string | undefined
    ): void => {
      update((state) => {
        if (!state[boardType]) {
          state[boardType] = getDefaultBoardState();
        }
        state[boardType]!.channelType = channelType;
        return state;
      });
    },

    /**
     * Reset state for a specific board type
     */
    resetBoardState: (boardType: PipoTypes): void => {
      update((state) => {
        state[boardType] = getDefaultBoardState();
        return state;
      });
    },

    /**
     * Clear all state
     */
    clearAll: (): void => {
      set({});
    },
  };
}

export const uiState = createUIStateStore();
