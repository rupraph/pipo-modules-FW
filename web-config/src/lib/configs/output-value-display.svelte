<script lang="ts">
  export let value: number | undefined;
  export let type: "osc" | "midi-cc" | "midi-note" | "midi-pb" = "osc";
  export let label: string = "Output";

  // Convert MIDI note number to note name (e.g., 60 -> "C4")
  function noteToName(note: number): string {
    const names = [
      "C",
      "C#",
      "D",
      "D#",
      "E",
      "F",
      "F#",
      "G",
      "G#",
      "A",
      "A#",
      "B",
    ];
    const octave = Math.floor(note / 12) - 1;
    const noteName = names[note % 12];
    return `${noteName}${octave}`;
  }

  $: displayValue =
    value !== undefined
      ? type === "midi-note"
        ? `${noteToName(Math.round(value))} (${Math.round(value)})`
        : value.toFixed(2)
      : "--";
</script>

<div class="output-display">
  <span class="label">{label}:</span>
  <span class="value">{displayValue}</span>
</div>

<style>
  .output-display {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 8px 12px;
    background-color: var(--bg-tertiary);
    border: 1px solid var(--grey);
    border-radius: 6px;
    font-size: 13px;
    margin-top: 8px;
  }

  .label {
    color: var(--grey);
    font-weight: 500;
  }

  .value {
    font-family: "Monaco", "Courier New", monospace;
    color: var(--main);
    font-weight: 600;
    min-width: 60px;
    text-align: right;
  }
</style>
