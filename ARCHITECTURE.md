# AI Powered DAW Architecture

This project is a compact DAW prototype built in C++20 with JUCE. The goal is
to show a real audio application architecture plus an AI-agent command layer,
not just a chat box next to a UI.

## System Overview

The app is split into four main subsystems:

- Project model: tracks, clips, MIDI notes, audio clip metadata, devices,
  automation, transport, and persistence.
- Audio engine: JUCE device setup, realtime callback, transport timing,
  metronome, track graph rendering, instruments, effects, and offline export.
- DAW UI: arrangement, piano roll, automation lane, mixer, inspector, transport,
  command console, diagnostics, and agent panel.
- Agent layer: JSON command tools, project summarization, command history,
  LLM provider abstraction, local/cloud provider implementations, and
  multi-step command plans.

## Core Project Model

`aidaw::Project` is the central mutable model. It owns:

- `Track`: audio or MIDI track state, instrument, gain, pan, effects, clips,
  and automation lanes.
- `Clip`: MIDI note data or audio-file metadata.
- `Transport`: play state, tempo, and beat position.
- Stable integer entity IDs for tracks and clips.

Serialization is handled by `ProjectSerializer`, while `ProjectFileService`
adds project save/load and timestamped backups.

## Audio Engine

`AudioEngine` owns JUCE audio-device lifecycle and calls into
`TrackProcessingGraph` from the realtime callback. The graph renders all active
tracks into the output buffer using beat-domain transport timing.

Important audio features:

- Realtime MIDI note scheduling from clips.
- Built-in sine, subtractive, and drum synth modes.
- Low-pass, saturation, and delay effect slots.
- Gain and pan automation interpolation during render.
- Audio clip playback from imported WAV/AIFF files.
- Callback timing diagnostics and overrun tracking.
- Offline WAV export through the same graph path.

## Command Layer

`CommandExecutor` exposes DAW operations as validated JSON commands. Each
command can be previewed before execution, then recorded in `CommandHistory`.

Supported tool commands include:

- `create_track`
- `create_midi_clip`
- `add_midi_notes`
- `set_track_gain`
- `add_effect`
- `add_automation_point`
- `summarize_project`
- `create_melancholy_indie_seed`

The command layer also emits a tool manifest that an agent can use as a compact
contract for project mutation.

## Agent And LLM Layer

The AI path is deliberately provider-agnostic:

- `ILLMProvider` defines the model boundary.
- `MockLLMProvider` gives deterministic offline demos and tests.
- `OpenAICompatibleProvider` supports BYOK chat-completions style endpoints.
- `OllamaProvider` supports local `/api/generate` models.
- `JuceHttpTransport` performs HTTP requests through JUCE.
- `AgentCommandService` builds project/tool context, calls the provider,
  extracts command plans, resolves created IDs, validates each step, executes
  each step, and records history.

Agent responses may be:

- A single command object.
- An array of command objects.
- `{ "commands": [...] }`.

Multi-step plans can reference earlier results with `"$lastId"` or
`"$stepN.id"`. This lets a model create a track, create a clip on that track,
and add notes to that clip in one prompt without knowing IDs ahead of time.

For musical prompts, the agent can also use higher-level tools such as
`create_melancholy_indie_seed`, which creates a coherent sketch directly:
tempo, chord track, bass track, soft drum track, sparse lead track, and effects.

## Test Strategy

The test suite focuses on deterministic model behavior and audio graph
correctness:

- Project model and serialization round trips.
- Project file save/load and backup creation.
- MIDI editing helpers.
- Track rendering, pan/gain, instruments, effects, audio clips, and automation.
- Offline render output.
- Command validation/execution/history.
- LLM request builders, provider response parsers, fake HTTP transport, and
  multi-step agent command plans.

## Why This Is Portfolio-Relevant

This is intentionally broader than a toy UI:

- Realtime C++ audio callback and graph rendering.
- Persistent DAW state model with serialization.
- UI panels tied to real project mutation paths.
- Local and BYOK model integration behind a testable interface.
- Agent tools with validation and execution history.
- Multi-step planning with ID resolution across generated commands.
