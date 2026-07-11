# Demo Guide

This guide is written for recording a short portfolio walkthrough or explaining
the project in an interview.

## Setup

Build the app:

```powershell
cmake --build build-vs --config Debug --target AIPoweredDAW
```

Run tests:

```powershell
ctest --test-dir build-vs -C Debug --output-on-failure
```

Launch the app:

```powershell
.\build-vs\src\AIPoweredDAW_artefacts\Debug\"AI Powered DAW.exe"
```

For local AI, run Ollama and use:

- Provider: `Ollama`
- Endpoint: `http://127.0.0.1:11434/api/generate`
- Model: `gemma4:e4b`
- API key: blank

## 3-Minute Walkthrough

1. Start playback and show the realtime drum loop, transport, callback timing,
   and diagnostics.
2. Click the piano roll to add notes, then use quantize, transpose, and
   duplicate.
3. Change instrument mode and add low-pass, saturation, or delay.
4. Move mixer gain/pan and add gain automation from the automation lane.
5. Use the command console to preview and execute `summarize_project`.
6. Use the agent panel in Mock mode to create an `AI Bass` track, clip, and
   notes through a multi-step plan.
7. Switch to Ollama/Gemma 4 and run a simple prompt.
8. Save the project, export WAV, and point out the test coverage.

## Good Agent Prompts

Use prompts that map cleanly to supported tools:

- `Make a sad, dreamy indie song starter with soft drums, warm bass, wistful chords, and a simple lead.`
- `Create a melancholy bedroom-pop sketch around 82 BPM.`
- `Make a laid-back rainy-day indie loop with warm chords and a sparse lead.`
- `Create a MIDI bass track with a four beat clip and simple root-fifth notes.`
- `Add a delay effect to the first track.`
- `Create a lead MIDI track and add a one bar phrase.`
- `Summarize this project.`
- `Add a gain automation point on the first track at beat 4.`

## Example Command Plan

For sad/dreamy indie material, the preferred high-level command is:

```json
{
  "type": "create_melancholy_indie_seed",
  "name": "Rainy Window",
  "bpm": 82,
  "startBeat": 0,
  "lengthBeats": 16
}
```

Lower-level plans still work when you want exact control:

```json
{
  "commands": [
    {
      "type": "create_track",
      "trackType": "midi",
      "name": "AI Bass"
    },
    {
      "type": "create_midi_clip",
      "trackId": "$step1.id",
      "name": "Generated Bass",
      "startBeat": 0,
      "lengthBeats": 4
    },
    {
      "type": "add_midi_notes",
      "trackId": "$step1.id",
      "clipId": "$step2.id",
      "notes": [
        { "pitch": 36, "startBeat": 0, "lengthBeats": 0.5, "velocity": 0.95 },
        { "pitch": 43, "startBeat": 1, "lengthBeats": 0.5, "velocity": 0.85 },
        { "pitch": 48, "startBeat": 2, "lengthBeats": 0.5, "velocity": 0.9 },
        { "pitch": 43, "startBeat": 3, "lengthBeats": 0.5, "velocity": 0.85 }
      ]
    }
  ]
}
```

## Resume Bullets

- Built a C++20/JUCE AI-assisted DAW prototype with realtime audio playback,
  MIDI sequencing, built-in synth/effect processing, automation, project
  persistence, and offline WAV export.
- Designed a validated JSON command layer and MCP-style tool manifest for
  agent-callable DAW operations, including command previews and execution
  history.
- Implemented provider-agnostic LLM integration with deterministic mock tests,
  OpenAI-compatible BYOK endpoints, local Ollama/Gemma support, and multi-step
  command planning with ID resolution across generated actions.
- Developed deterministic tests for project serialization, audio graph
  rendering, automation, offline export, command execution, provider parsing,
  and agent command plans.

## Technical Talking Points

- The audio callback never calls the LLM; AI only mutates project state through
  validated commands.
- Audio clips are loaded outside the realtime callback and rendered through the
  same track graph as MIDI instruments.
- Agent output is treated as untrusted JSON and validated before mutation.
- Multi-step plans solve the ID problem by resolving `"$stepN.id"` references
  after each successful command.
- The mock provider keeps demos and tests deterministic without a network
  dependency.
