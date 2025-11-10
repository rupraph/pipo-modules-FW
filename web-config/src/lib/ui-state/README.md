# UI State Management

This module provides persistent UI state management for the Pipo Config web application. It stores UI preferences in localStorage, organized by board type (analog, motion, range).

## Features

- **Persistent state**: UI state is saved to localStorage and restored on page load
- **Per-board organization**: Each board type (analog, motion, range) has its own isolated state
- **Collapse tracking**: Remembers which collapse sections are open/closed
- **Channel selection**: Remembers the last selected channel in "Channel settings"

## What is NOT stored

The following data is intentionally **not** persisted:
- Configuration data fetched from the server (pipoio)
- Service data
- Any data that should be fresh from the server on each load

## Usage

### In Collapse Components

To enable state persistence for a `Collapse` component, add both the `collapseId` prop and your desired default `open` state:

```svelte
<!-- This collapse starts closed by default, but remembers user preference -->
<Collapse title="Quick settings" collapseId="quick-settings" open={false}>
  <QuickConfig bind:config />
</Collapse>

<!-- This collapse starts open by default -->
<Collapse title="Presets" collapseId="presets" open={true}>
  <Presets />
</Collapse>
```

**Important:** The `open` prop defines the **default state** that will be used on first load. Once the user interacts with the collapse, their preference is saved and will override the default on subsequent loads.

The `collapseId` should be:
- Unique within the application
- Descriptive and kebab-case formatted
- Consistent across component updates

### Standard Collapse IDs

The following collapse IDs are used throughout the app:

- `quick-settings` - Quick settings panel (default: closed)
- `presets` - Presets panel (default: open)
- `channel-settings` - Channel settings panel (default: open)
- `info` - Info panel (default: closed)
- `osc-settings` - OSC settings panel (default: closed)
- `board-settings` - Board settings panel (default: closed)

### Programmatic Access

You can also access the state store directly:

```typescript
import { uiState } from './lib/ui-state';
import { pipoType } from './services';

// Get collapse state
const isOpen = uiState.getCollapseState($pipoType, 'channel-settings');

// Set collapse state
uiState.setCollapseState($pipoType, 'channel-settings', true);

// Get selected channel
const channel = uiState.getSelectedChannel($pipoType);

// Set selected channel
uiState.setSelectedChannel($pipoType, 'A01');

// Reset state for a board type
uiState.resetBoardState($pipoType);

// Clear all state
uiState.clearAll();
```

## Storage Structure

The state is stored in localStorage under the key `pipo-ui-state` with the following structure:

```json
{
  "analog": {
    "collapses": {
      "quick-settings": false,
      "presets": true,
      "channel-settings": true,
      "board-settings": false
    },
    "selectedChannel": "A01"
  },
  "motion": {
    "collapses": {
      "presets": true,
      "channel-settings": true
    },
    "selectedChannel": "accX"
  },
  "range": {
    "collapses": {
      "channel-settings": true
    },
    "selectedChannel": "dist"
  }
}
```

## How Defaults Work

Default states are defined directly in the component props (e.g., `open={true}` or `open={false}`). The store only saves and retrieves user interactions - it doesn't define any defaults itself. This makes each component self-documenting about its initial behavior.

When a user first visits the app:
1. The `Collapse` component uses the `open` prop value as the initial state
2. When the user toggles the collapse, that preference is saved to localStorage
3. On subsequent visits, the saved preference overrides the prop default
