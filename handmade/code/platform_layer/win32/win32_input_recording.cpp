/*  =====================================================================
    $File: platform_layerwin32win32_input_recording.cpp
    $Creation Date: 2015-06-15
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_INPUT_RECORDING_CPP

internal void
Win32BeginRecordingInput(win32_state* win32State)
{
  win32State->snapshotRecordingIndex = 0;
  win32State->snapshotRecording = true;

  CopyMemory(win32State->permanentSnapshot,
             win32State->permanentStorage,
             win32State->permanentSnapshotSize);
}

internal void
Win32EndRecordingInput(win32_state* win32State)
{
  win32State->snapshotRecording = false;
}

internal void
Win32RecordInput(win32_state* win32State, game_input* gameInput)
{
  if(win32State->snapshotRecordingIndex == win32State->snapshotMax)
  {
    win32State->snapshotRecording = false;
  }
  win32_frame_snapshot snapshot = {};
  snapshot.gameInput = *gameInput;

  *(win32State->snapshots +
    win32State->snapshotRecordingIndex) = snapshot;
  win32State->snapshotRecordingIndex++;
}

internal void
Win32BeginPlaybackInput(win32_state* win32State)
{
  win32State->snapshotPlaybackIndex = 0;
  win32State->snapshotPlayback = true;

  CopyMemory(win32State->permanentStorage,
             win32State->permanentSnapshot,
             win32State->permanentSnapshotSize);
}

internal void
Win32EndPlaybackInput(win32_state* win32State)
{
  win32State->snapshotPlayback = false;

  // NOTE(Cristián): We need to zero the input, otherwise the
  // state of the controller when it was playbacking would be preserved
  game_controller_input* c = win32State->gameInput->controllers;
  for(int i = 0;
      i < ARRAY_COUNT(win32State->gameInput->controllers);
      ++i, *c = {});
}


internal void
Win32PlaybackInput(win32_state* win32State, game_input* gameInput)
{
  *gameInput = (win32State->snapshots +
                win32State->snapshotPlaybackIndex++)->gameInput;
  // LOOP
  if(win32State->snapshotPlaybackIndex == win32State->snapshotRecordingIndex)
  {
    Win32BeginPlaybackInput(win32State);
  }
}

#define _WIN32_INPUT_RECORDING_CPP
#endif
