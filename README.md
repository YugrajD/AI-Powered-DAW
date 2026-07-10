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
