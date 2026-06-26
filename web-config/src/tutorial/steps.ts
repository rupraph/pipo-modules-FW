import type { TutorialStep, TutorialChapter } from './types';

/**
 * data-tutorial ID registry — every targetable element in the UI:
 *
 *   menu-wifi        — WiFi icon button in Menu bar
 *   menu-settings    — Settings gear icon in Menu bar
 *   section-output   — Output settings section (MIDI/OSC toggle)
 *   section-presets  — Presets section
 *   channel-list     — Channel list (AnalogChannels / MotionChannels)
 *   channel-settings — Per-channel configuration panel (deadband, curve, range)
 *   save-button      — FloatingSaveButton
 *   relative-mode    — Relative mode toggle (motion only)
 *   hold-mode        — Hold mode toggle (range only)
 *   battery-status   — Battery indicator in top bar
 */

// ── Core tutorial chapters ──────────────────────────────────────────

export const coreChapters: TutorialChapter[] = [
  {
    id: 'welcome',
    title: 'Welcome',
    stepIds: ['welcome-intro'],
  },
  {
    id: 'wifi',
    title: 'WiFi & Connection',
    stepIds: ['wifi-open', 'wifi-connect'],
  },
  {
    id: 'channels',
    title: 'Your Sensor',
    stepIds: ['pick-channel', 'adjust-deadband', 'range-curve'],
  },
  {
    id: 'output',
    title: 'Output Modes',
    stepIds: ['choose-output', 'per-channel'],
  },
  {
    id: 'presets',
    title: 'Presets & Save',
    stepIds: ['load-preset', 'save-config'],
  },
];

// ── Core tutorial steps ─────────────────────────────────────────────

export const coreSteps: TutorialStep[] = [
  // ── Chapter: Welcome ──────────────────────────────────────────
  {
    id: 'welcome-intro',
    chapterId: 'welcome',
    target: null,
    title: 'Welcome to Pipo!',
    icon: '👋',
    description:
      'Pipo is a wireless sensor-to-MIDI/OSC controller. ' +
      'It turns physical motion, distance, or touch into musical control signals. ' +
      'This quick tour will show you the basics — tap "Next" to get started.',
  },

  // ── Chapter: WiFi & Connection ─────────────────────────────────
  {
    id: 'wifi-open',
    chapterId: 'wifi',
    target: 'menu-wifi',
    title: 'Connect to WiFi',
    icon: '📶',
    description:
      'Tap the WiFi icon in the top menu to open network settings. ' +
      'You can scan for nearby networks and connect Pipo to your local WiFi ' +
      'so it can communicate with your computer or mobile device.',
  },
  {
    id: 'wifi-connect',
    chapterId: 'wifi',
    target: 'menu-wifi',
    title: 'Stay Connected',
    icon: '🔗',
    description:
      'Once connected to WiFi, Pipo can send OSC messages over your network ' +
      'and serve this configuration page. ' +
      'The WiFi icon shows your current signal strength.',
  },

  // ── Chapter: Your Sensor ───────────────────────────────────────
  {
    id: 'pick-channel',
    chapterId: 'channels',
    target: 'channel-list',
    title: 'Pick a Channel',
    icon: '🎚️',
    description:
      'Each sensor axis is a "channel" — like pitch, roll, distance, or touch. ' +
      'Select a channel from this list to see and adjust its settings below.',
  },
  {
    id: 'adjust-deadband',
    chapterId: 'channels',
    target: 'channel-settings',
    title: 'Adjust the Deadband',
    icon: '🎯',
    description:
      'The deadband sets how much the sensor must move before triggering output. ' +
      'A small deadband = more responsive but more noise. ' +
      'Adjust this per channel to match your playing style.',
  },
  {
    id: 'range-curve',
    chapterId: 'channels',
    target: 'channel-settings',
    title: 'Range & Curve',
    icon: '📐',
    description:
      'Set the input range (min/max) to map your physical movement to MIDI or OSC values. ' +
      'You can also invert the signal or enable cyclic wrapping for continuous controllers.',
  },

  // ── Chapter: Output Modes ──────────────────────────────────────
  {
    id: 'choose-output',
    chapterId: 'output',
    target: 'section-output',
    title: 'MIDI or OSC?',
    icon: '🔊',
    description:
      'Pipo supports two output protocols: MIDI (USB + BLE) and OSC (WiFi). ' +
      'Choose your mode here. MIDI is great for DAWs and synths, ' +
      'OSC is perfect for creative coding and wireless setups.',
  },
  {
    id: 'per-channel',
    chapterId: 'output',
    target: 'section-output',
    title: 'Per-Channel Output',
    icon: '⚙️',
    description:
      'Each channel can be configured independently — set MIDI CC numbers, ' +
      'note mappings, or OSC addresses. ' +
      'Enable only the channels you need to keep things clean.',
  },

  // ── Chapter: Presets & Save ────────────────────────────────────
  {
    id: 'load-preset',
    chapterId: 'presets',
    target: 'section-presets',
    title: 'Load a Preset',
    icon: '📦',
    description:
      'Presets are ready-to-use configurations for common setups. ' +
      'Load one to quickly try different musical mappings ' +
      'without configuring everything manually.',
  },
  {
    id: 'save-config',
    chapterId: 'presets',
    target: 'save-button',
    title: 'Save Your Config',
    icon: '💾',
    description:
      'Whenever you make changes, the Save button appears here. ' +
      'Tap it to persist your settings to the device. ' +
      'Pipo remembers your config even after power cycling.',
  },
];

// ── What's New steps (per firmware version) ────────────────────────

export const whatsNewSteps: Record<string, TutorialStep[]> = {
  // Example — add entries for future versions:
  // 'v2.0.0': [
  //   {
  //     id: 'whatsnew-v2-ble',
  //     chapterId: 'whats-new',
  //     target: 'toggle-ble',
  //     title: 'New: BLE MIDI',
  //     icon: '🆕',
  //     description: 'You can now use Bluetooth MIDI to connect wirelessly...',
  //   },
  // ],
};
