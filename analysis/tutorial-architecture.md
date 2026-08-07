# Tutorial System — Architecture & Implementation Plan

## Overview

Add a first-boot intro tutorial to the Pipo web config UI with two tiers:

1. **Core Tutorial** — Generic intro to the device (one-time, backend-flagged)
2. **What's New** — Version-specific feature highlights (per-version, localStorage-tracked)

Visual approach: **Hybrid — spotlight highlight on target elements + fixed bottom panel** with step content and navigation.

---

## 1. Backend (Firmware) Changes

### 1.1 Add NVS Initialization Flag

Add an NVS key `"initialized"` (bool, defaults to `false`/absent).

**Why NVS over config file**: Survives config resets, separate concern from config, already used for WiFi credentials.

```
NVS namespace: "pipo"
Key: "initialized" → uint8_t (0 = not initialized, 1 = initialized)
```

### 1.2 Extend `/info` Response

Add `"initialized"` boolean to the JSON:

```json
{
  "name": "unnamed Pipo",
  "version": "v1.0.0",
  "type": "pipo_motion",
  "ip": "192.168.4.1",
  "mac": "AA:BB:CC:DD:EE:FF",
  "hw_rev": "11",
  "batt_type": "1",
  "initialized": false // ← NEW
}
```

Firmware reads NVS key on boot, caches in a `bool`, exposes in `/info`.

### 1.3 Add `POST /tutorial-complete` Endpoint

Sets `initialized = true` in NVS. Called by frontend when user completes or skips the core tutorial.

```cpp
server.on("/tutorial-complete", HTTP_POST, [&](AsyncWebServerRequest* request) {
    preferences.putUChar("initialized", 1);
    request->send(200);
});
```

### 1.4 Add `POST /tutorial-reset` Endpoint (Optional)

For debugging / support — resets the flag so tutorial auto-shows again:

```cpp
server.on("/tutorial-reset", HTTP_POST, [&](AsyncWebServerRequest* request) {
    preferences.putUChar("initialized", 0);
    request->send(200);
});
```

### Files to modify

- `src/server/server.cpp` — add `/tutorial-complete`, `/tutorial-reset`, extend `/info`
- `src/server/server.h` — no changes needed (no new members)
- Possibly `src/main.cpp` — if NVS init needs adjustment

---

## 2. Frontend — Component Architecture

### 2.1 New File Tree

```
web-config/src/
  tutorial/
    TutorialSystem.svelte      — Orchestrator: renders overlay + panel, manages step state
    SpotlightOverlay.svelte    — Full-screen dimming overlay with cutout "hole"
    TutorialPanel.svelte        — Fixed bottom panel with content + navigation
    store.ts                    — Tutorial Svelte stores + localStorage persistence
    steps.ts                    — Step definitions (typed data arrays)
    types.ts                    — TutorialStep, TutorialState, etc.
```

### 2.2 Component Responsibilities

#### `TutorialSystem.svelte` (Orchestrator)

- Receives `active: boolean` prop (bound from App.svelte)
- On activate: determines which steps to show (core vs what's-new vs both)
- Manages `currentStepIndex`, renders `SpotlightOverlay` + `TutorialPanel`
- Handles: next, previous, skip, complete
- On complete/skip: POSTs to `/tutorial-complete`, updates localStorage
- Calculates spotlight target position, passes to SpotlightOverlay
- Listens to scroll/resize to reposition spotlight

#### `SpotlightOverlay.svelte`

- Props: `targetSelector: string | null`, `active: boolean`
- Renders a full-screen fixed overlay with `pointer-events: none`
- Uses the **box-shadow punch-through** technique:
  - Semi-transparent background
  - A "hole" positioned over the target element
  - Glowing border/ring around the target
- Handles edge cases: target not found, target off-screen, target too small
- Smooth transition when moving between steps

#### `TutorialPanel.svelte`

- Props: `step: TutorialStep`, `stepIndex: number`, `totalSteps: number`, `isLast: boolean`
- Fixed at bottom of viewport (z-index above spotlight overlay)
- Shows: step title, description, optional emoji/icon
- Navigation: "Previous" (hidden on step 0), "Next" / "Got it!" (last step), "Skip All"
- Progress dots (●●○○○)
- Enter animation: slide-up + fade
- Exit animation: slide-down + fade
- Responsive: scrollable if content overflows

### 2.3 Store (`store.ts`)

```typescript
// Which tutorial mode is active
export const tutorialMode = writable<"core" | "whats-new" | "both" | null>(
  null,
);

// Whether the tutorial overlay is currently showing
export const tutorialActive = writable<boolean>(false);

// Current step index
export const tutorialStep = writable<number>(0);

// Steps being shown in this session
export const tutorialSteps = writable<TutorialStep[]>([]);

// Derived: current step object
export const currentTutorialStep = derived(
  [tutorialSteps, tutorialStep],
  ([$steps, $step]) => $steps[$step] ?? null,
);
```

**localStorage keys** (follows `pipo-ui-state` pattern):

```
pipo-tutorial-core-completed   → "true" | absent
pipo-tutorial-last-version     → "v1.0.0"  (version string)
```

### 2.4 Step Definitions (`steps.ts`)

```typescript
interface TutorialStep {
  id: string; // Unique step ID, e.g. "intro-welcome"
  target: string; // CSS selector for spotlight, null for center-modal
  title: string; // Short title (shown in panel)
  description: string; // Explanation text (supports basic markdown)
  icon?: string; // Optional emoji icon
  variant?: PipoTypes[]; // Only show for these board types (omit = all)
  position?: "top" | "bottom"; // Panel position preference (default: bottom)
}

interface TutorialChapter {
  id: string; // "core" | "whats-new-v2.0"
  title: string; // "Getting Started" | "What's New in v2.0"
  steps: TutorialStep[];
}
```

**Core tutorial steps** (example — to be refined):

1. Welcome + device name (target: none / centered)
2. WiFi & connection (target: `.menu-bar` WiFi icon)
3. Output mode — MIDI vs OSC (target: `.output-settings` section)
4. Selecting a channel (target: `.channel-list` first item)
5. Configuring a channel — deadband, curve, range (target: `.channel-settings`)
6. Presets — quick configuration (target: `.presets-section`)
7. Saving changes (target: `.floating-save-button`)

**What's New steps** — defined per version, keyed by version string:

```typescript
const whatsNewSteps: Record<string, TutorialStep[]> = {
  "v1.1.0": [
    { id: "new-feature-ble", target: ".ble-toggle", ... },
    { id: "new-feature-hires", target: ".hires-toggle", ... },
  ],
  "v2.0.0": [ ... ],
};
```

### 2.5 Types (`types.ts`)

```typescript
export interface TutorialStep {
  id: string;
  target: string | null; // null = center spotlight only
  title: string;
  description: string;
  icon?: string;
  variant?: PipoTypes[]; // filter by board type
}
```

---

## 3. Data Flow

### 3.1 First-Ever Boot

```
App.svelte {#await fetch()}
  → GET /info → { initialized: false, version: "v1.0.0" }
  → localStorage "pipo-tutorial-core-completed" absent
  → localStorage "pipo-tutorial-last-version" absent
  → tutorialMode.set('core')
  → tutorialActive.set(true)
  → render TutorialSystem (core steps only)
```

### 3.2 User Completes Core Tutorial

```
TutorialSystem: on last step "Got it!"
  → POST /tutorial-complete    (sets NVS initialized=true)
  → localStorage "pipo-tutorial-core-completed" = "true"
  → localStorage "pipo-tutorial-last-version" = "v1.0.0"
  → tutorialActive.set(false)
```

### 3.3 User Skips Tutorial

```
TutorialPanel: "Skip All" clicked
  → POST /tutorial-complete    (sets NVS initialized=true)
  → localStorage "pipo-tutorial-core-completed" = "true"
  → localStorage "pipo-tutorial-last-version" = "v1.0.0"
  → tutorialActive.set(false)
```

### 3.4 Firmware Upgrade (v1.0 → v2.0)

```
App.svelte {#await fetch()}
  → GET /info → { initialized: true, version: "v2.0.0" }
  → localStorage "pipo-tutorial-core-completed" = "true"    → skip core
  → localStorage "pipo-tutorial-last-version" = "v1.0.0"
  → "v2.0.0" > "v1.0.0"  →  tutorialMode.set('whats-new')
  → tutorialActive.set(true)
  → render TutorialSystem (whats-new steps for v2.0.0 only)
```

### 3.5 User Replays Tutorial from Menu

```
Menu: "Help → Tutorial" clicked
  → tutorialSteps.set(allCoreSteps)   // always core steps
  → tutorialActive.set(true)
  → Does NOT reset initialized flag or localStorage
  → Does NOT POST /tutorial-complete at end
```

### 3.6 Subsequent Boots (Already Initialized, Same Version)

```
→ initialized=true, core-completed=true, last-version matches current
→ tutorialActive remains false
→ No tutorial shown
```

---

## 4. Spotlight Implementation (CSS Technique)

The box-shadow "punch-through" technique:

```css
.spotlight-overlay {
  position: fixed;
  inset: 0;
  z-index: 100;
  pointer-events: none; /* clicks pass through to page */
}

.spotlight-overlay::before {
  content: "";
  position: fixed;
  /* Position = target element's bounding rect */
  top: var(--spotlight-top);
  left: var(--spotlight-left);
  width: var(--spotlight-width);
  height: var(--spotlight-height);
  border-radius: 8px;
  box-shadow:
    0 0 0 9999px rgba(0, 0, 0, 0.6),
    /* dim everything outside */ 0 0 20px 4px var(--main); /* glow around target */
  transition: all 0.3s ease;
}
```

JavaScript updates CSS custom properties based on `targetElement.getBoundingClientRect()`.

**Edge cases handled in SpotlightOverlay**:

- Target not found: fall back to centered spotlight
- Target scrolled off-screen: scroll it into view
- Resize/orientation change: recalculate on `ResizeObserver`
- Rapid step changes: debounce reposition to 100ms

---

## 5. Integration Points

### 5.1 `App.svelte` Changes

```svelte
<script>
  import TutorialSystem from './tutorial/TutorialSystem.svelte';
  import { tutorialActive, tutorialMode } from './tutorial/store';
  import { checkTutorial } from './tutorial/store';

  // After fetch(), before rendering main content:
  async function fetch() {
    const { data } = await pipoio.get<PipoInfo>("/info");
    // ... existing type detection ...
    pipoInfo.set(data);

    // Check if tutorial should auto-show
    await checkTutorial(data);
  }
</script>

<main>
  <!-- ... existing content ... -->

  {#if $tutorialActive}
    <TutorialSystem />
  {/if}
</main>
```

### 5.2 Menu Changes (`lib/menu/menu.svelte`)

Add a "Tutorial" entry somewhere accessible (e.g., under a "Help" sub-menu or as a dedicated button):

```svelte
<button on:click={startTutorial} title="Tutorial">
  <HelpCircle />  <!-- lucide-svelte icon -->
</button>
```

`startTutorial()` sets `tutorialSteps` to core steps and `tutorialActive = true`.

### 5.3 No changes needed for:

- `pipoio.ts` — existing HTTP/WS client works fine
- `services/config.ts` — stores unaffected
- Existing modals — tutorial has own z-index layer above modals (z-index: 150)
- Form components — no changes

---

## 6. Z-Index Layering

```
100  — SpotlightOverlay (pointer-events: none)
110  — TutorialPanel (bottom panel, interactable)
20-25 — Existing modals (lower, hidden behind spotlight when tutorial active)
1000 — OfflineOverlay (still on top if connection drops during tutorial)
```

When tutorial is active:

- `TutorialPanel` captures pointer events (navigation buttons)
- `SpotlightOverlay` is `pointer-events: none` so the user can still interact with the highlighted element if they wish
- Optionally: add a "spotlight ring" that's `pointer-events: auto` only on the target element

---

## 7. Variant-Specific Steps

Some steps only make sense for certain board types. The `variant` filter in `TutorialStep` handles this:

```typescript
// Only shown on motion boards
{
  id: "motion-relative-mode",
  target: ".relative-mode-row",
  title: "Relative Mode",
  description: "Toggle between absolute and relative orientation tracking...",
  variant: ["motion"],
}
```

The orchestrator filters steps based on `$pipoType` before starting the tutorial.

---

## 8. Deliverables & Implementation Order

| Phase  | Deliverable                               | Files                                                      |
| ------ | ----------------------------------------- | ---------------------------------------------------------- |
| **P1** | NVS flag + `/info` extension + endpoints  | `src/server/server.cpp`, `src/main.cpp`                    |
| **P2** | Tutorial types + store + step definitions | `web-config/src/tutorial/types.ts`, `store.ts`, `steps.ts` |
| **P3** | SpotlightOverlay component                | `web-config/src/tutorial/SpotlightOverlay.svelte`          |
| **P4** | TutorialPanel component                   | `web-config/src/tutorial/TutorialPanel.svelte`             |
| **P5** | TutorialSystem orchestrator               | `web-config/src/tutorial/TutorialSystem.svelte`            |
| **P6** | Integration: App.svelte + Menu            | `web-config/src/App.svelte`, `lib/menu/menu.svelte`        |
| **P7** | Write tutorial step content               | `web-config/src/tutorial/steps.ts`                         |
| **P8** | Test all 3 variants + edge cases          | manual testing                                             |

---

## 9. Future Considerations

- **Step images/GIFs**: `TutorialStep` can gain an `image?: string` field for screenshots
- **Analytics**: Track which steps users skip most → improve UX
- **i18n**: Step definitions could move to JSON files keyed by locale
- **A/B testing**: Different tutorial orders for different users
- **Interactive steps**: "Click here to try it" — detect click on target to advance
- **Accessibility**: ARIA labels, keyboard navigation, reduced-motion support
