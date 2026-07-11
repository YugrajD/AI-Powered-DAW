# AI Powered DAW

A compact AI-assisted digital audio workstation built in modern C++.

## Stage 0

The first milestone establishes the project shell, core project model,
serialization, diagnostics, and unit tests before realtime audio work begins.

## Stage 1

The realtime audio foundation adds JUCE audio device setup, transport state,
an audio callback, metronome generation, a prepared track processing graph,
gain/pan processing, and callback performance diagnostics.

## Stage 2

The MIDI sequencer milestone adds clip editing helpers, MIDI note scheduling
through the audio graph, a simple built-in sine synth voice, demo pattern
playback, quantize/transpose/duplicate controls, and tests for sequencing
behavior.

## Stage 3

The core UI milestone introduces a DAW-style shell with an arrangement
timeline, track headers, clip blocks, piano roll note display/editing,
transport bar, inspector panel, and diagnostics area.

## Stage 4

The built-in instruments/effects milestone adds explicit track device state,
three internal instruments (sine, subtractive, and drum synth), built-in
low-pass, saturation, and delay effects, device serialization, inspector
display, device controls, and render-path tests.

## Stage 5

The audio clips milestone adds audio clip metadata, WAV/AIFF import, clip
gain/fade fields, audio file loading outside the realtime callback, audio
clip playback through the track graph, waveform-style arrangement previews,
and render-path coverage for imported audio clips.

## Stage 6

The mixer and automation milestone adds track automation lanes, gain/pan
automation interpolation, automation-aware audio rendering, a mixer panel
with per-track gain/pan controls, a clickable gain automation lane, and
tests for automation playback behavior.

## Stage 7

The project system and export milestone adds `.aidaw` project save/load,
timestamped backup writes, offline WAV rendering through the track graph,
Save/Load/Export controls, and tests for project persistence and render
output.

## Stage 8

The AI command layer milestone adds JSON-based project mutation commands,
validation/previews, command execution history, project summarization, an
in-app command console, and an MCP-oriented tool manifest for agent-callable
DAW operations.

## Stage 9

The LLM integration milestone adds a provider-agnostic agent boundary, a
deterministic mock provider for offline demos/tests, OpenAI-compatible and
Ollama request builders, an agent command service that turns model output into
validated DAW mutations, and an in-app prompt path that exercises the full
agent-to-command pipeline without requiring an API key.

The current integration is deliberately split into two layers:

- `ILLMProvider` owns model-specific command generation.
- `AgentCommandService` owns project summarization, tool-manifest context,
  preview, execution, and command-history recording.

This keeps BYOK cloud providers and local model backends swappable while the
DAW core stays testable and deterministic.
