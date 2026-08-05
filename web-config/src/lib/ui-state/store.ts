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
 * Get default channel for a board type
 * Returns the default channel ID that should be selected when no channel is chosen
 */
function getDefaultChannel(boardType: PipoTypes): string | undefined {
  switch (boardType) {
    case "range":
      return "dist"; // Range board has only one channel
    case "analog":
      return "A01"; // Default to first analog channel
    case "motion":
      return "yaw"; // Default to yaw (most commonly used rotation channel)
    case "max30102":
      return "ir_ac"; // Default to IR pulsatile (primary PPG signal)
    case "unknown":
      return undefined;
    default:
      return undefined;
  }
}

/**
 * Get default state for a board type
 */
function getDefaultBoardState(boardType?: PipoTypes): BoardUIState {
  return {
    collapses: {},
    selectedChannel: boardType ? getDefaultChannel(boardType) : undefined,
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

  // When WebSocket connects, monitor all currently selected channels
  pipoio.on("connect", () => {
    const state = get({ subscribe });
    Object.entries(state).forEach(([boardType, boardState]) => {
      if (boardState?.selectedChannel) {
        pipoio.monitorAxis(boardState.selectedChannel);
      }
    });
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
          state[boardType] = getDefaultBoardState(boardType);
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
     * Initialize board state with defaults if not already set
     * This ensures that boards like 'range' have their default channel selected
     */
    initializeBoardState: (boardType: PipoTypes): void => {
      const state = get({ subscribe });
      const defaultChannel = getDefaultChannel(boardType);
      
      if (!defaultChannel) {
        return;
      }
      
      // Initialize if board state doesn't exist or selectedChannel is not set
      const needsInitialization = !state[boardType] || state[boardType]?.selectedChannel === undefined;
      
      if (needsInitialization) {
        update((currentState) => {
          if (!currentState[boardType]) {
            currentState[boardType] = getDefaultBoardState(boardType);
          }
          currentState[boardType]!.selectedChannel = defaultChannel;
          return currentState;
        });
      }
      
      // Always monitor the default channel for range board (it only has one channel)
      // For other boards, only monitor if we just initialized
      if (boardType === "range" || needsInitialization) {
        pipoio.monitorAxis(defaultChannel);
      }
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
          state[boardType] = getDefaultBoardState(boardType);
        }
        state[boardType]!.selectedChannel = channel;
        if (channel) {
          pipoio.monitorAxis(channel);
        }
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
          state[boardType] = getDefaultBoardState(boardType);
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
        state[boardType] = getDefaultBoardState(boardType);
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
