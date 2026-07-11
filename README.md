# AI Powered DAW

A compact AI-assisted digital audio workstation built in modern C++.

## Highlights

- C++20/JUCE desktop app with realtime audio playback and transport control.
- Project model for MIDI/audio tracks, clips, notes, devices, automation, and
  persistence.
- Track processing graph with built-in synths, effects, gain/pan automation,
  imported audio clips, and offline WAV export.
- DAW UI with arrangement view, piano roll, automation lane, mixer, inspector,
  command console, diagnostics, and agent panel.
- JSON command layer with validation, previews, execution history, project
  summarization, and an MCP-style tool manifest.
- Provider-agnostic agent integration with deterministic mock mode,
  OpenAI-compatible BYOK endpoints, Ollama/Gemma local model support, and
  multi-step command planning.
- High-level musical seed tools for generating melancholy indie/dream-pop song
  starters with chords, bass, soft drums, lead notes, and effects.
- Deterministic tests for project serialization, audio rendering, automation,
  provider parsing, command execution, and agent plans.

## Documentation

- [Architecture](ARCHITECTURE.md): system boundaries and technical design.
- [Demo Guide](DEMO.md): walkthrough script, prompts, command-plan example, and
  resume bullets.

## Build

```powershell
cmake -S . -B build-vs
cmake --build build-vs --config Debug --target AIPoweredDAW
```

Run tests:

```powershell
cmake --build build-vs --config Debug --target AIPoweredDAWTests
ctest --test-dir build-vs -C Debug --output-on-failure
```

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

## Stage 10

The provider runtime milestone adds concrete OpenAI-compatible and Ollama LLM
providers, a JUCE-backed HTTP transport, response parsers that extract command
JSON from model output, fake-transport tests for provider behavior, and in-app
provider controls for switching between mock, BYOK cloud, and local model
execution.

Provider modes:

- Mock: deterministic offline demo path that creates an `AI Bass` MIDI track.
- OpenAI Compatible: POSTs a chat-completions request to the configured
  endpoint with an optional bearer API key.
- Ollama: POSTs a local `/api/generate` request to the configured endpoint.

Providers may return one JSON command matching the tool manifest from Stage 8.
The command is still previewed and validated before it mutates the project.

## Stage 11

The agent planning milestone lets providers return multi-step command plans
instead of a single command. `AgentCommandService` accepts either one command,
an array of commands, or `{ "commands": [...] }`, then validates and executes
each step in order while recording every step in command history.

Plans can reference IDs created by earlier commands:

- `"$lastId"` resolves to the most recent successful command result ID.
- `"$step1.id"`, `"$step2.id"`, and so on resolve to specific earlier step IDs.

That allows one prompt to create a track, create a clip on that track, and add
notes to that clip without the model knowing project IDs in advance.

## Stage 12

The portfolio readiness milestone adds architecture documentation, a demo guide,
resume bullets, build/test instructions, and final app polish so the project is
easy to review, run, and explain.

## Stage 13

The musical tools milestone adds a higher-level `create_melancholy_indie_seed`
command for generating sad, dreamy indie song starters. The tool creates a slow
four-track sketch with wistful chords, warm bass, soft drums, sparse lead notes,
built-in effects, and a laid-back tempo, giving the agent a musical operation
instead of forcing it to assemble every note manually.
