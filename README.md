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
