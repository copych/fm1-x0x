# One performance mode

Design brief from the performer. This is the next interface architecture, not a
description of controls already implemented. Live4 improves drum composition;
it still uses the temporary X0X ARP, EDIT, FX and MIX screens.

The performer stays on one performance screen during playback. Target selection,
sound, composition, mix and temporary effects are direct gestures on that screen.
Holding a modifier temporarily changes the controls; releasing it restores their
normal meaning. No next-page action is needed for the eight live control groups.
Scene/file management and detailed step editing may live outside this screen.

| Live group | Required behavior |
| --- | --- |
| Participation | Direct part and individual drum mute/unmute. Show both levels clearly. Manual choices always win over the Director. Keep solo/restore available in this layer. |
| Director | One accessible conducting gesture, with current intent, pending action and transition boundary visible. Discovery/development and section direction belong here, not inside a 303-only generator menu. |
| Mutate / undo | Develop the selected ensemble members, undo the last musical change, and LIKE/RETURN an established motif. Musical undo must preserve later manual sound and mute choices. Cancel an unapplied mutation before rolling back applied material. |
| 303 sound | Selected 303's cutoff, resonance, envelope modulation and decay under the four main knobs. Accent, waveform and per-303 drive must also be directly reachable by held controls. |
| 303 effects / placement | Independent pan, level and send controls for A/B; per-303 drive remains in its synthesis chain. Targeted effects must state which 303 they affect. Shared sends remain distinct from inserts. |
| Drum sound | Select an individual instrument directly, then expose pitch, snap/attack, DJ filtering and distortion/drive. Labels follow the instrument's actual synthesis controls. Drum-bus versus instrument targeting must be explicit. |
| Momentary effects | Hold for stutter/repeat/chop, tape stop and filter gestures. Clearly indicate target (selected members or master), division and held state. Release restores live playback smoothly. |
| Mix | Levels, pan and sends without leaving performance. Instrument participation remains immediately available while mixing. |

## Screen hierarchy

Keep musical context legible rather than filling the display with a permanent
step grid. The screen should continuously show:

- Tempo, bar/phrase position and a clear beat indicator.
- Current section/Director intent and any queued transition.
- Selected parts, their colors, mute states and activity.
- On drums, the selected instrument and a compact mute/activity strip.
- Four large parameter names and values matching the knobs right now.
- A transient gesture label for LIKE, undo, development or a held effect.

The current prototype has four selectable engine targets: 909, 808, 303 A, 303 B.
How those become the eventual single drum-machine track/kit model remains a
separate architecture decision; this UI work must not silently merge their data.

## Interaction rules

Target selection and musical action are separate. Selecting a part must not
mute it or regenerate its phrase. Support simultaneous selection where an action
can sensibly affect several members; show the complete selection on screen.
Use a deliberate mute modifier rather than overloading an ordinary selection tap.

Knob changes should avoid sudden parameter jumps when the selected part changes.
The detailed takeover/relative-control rule must be chosen during the physical
mapping pass. Held layers must work while playback continues and must not strand
the performer in a menu. Momentary FX always release cleanly, including when the
selected target changes while the gesture is held.

Physical assignments are intentionally not finalized here. The current
ENV/LFO/OCT combinations are prototype access paths and do not define the future
performance keyboard. Map the complete set together, checking simultaneous
gestures and one-handed reach, instead of accumulating more ad-hoc shortcuts.

## Implementation sequence and capability gaps

1. Add the performance screen and a unified target/selection state, initially
   connecting existing mute, synthesis, mix, LIKE/RETURN and Director operations.
2. Give composition/undo one clear route, including ensemble actions and visible
   pending boundaries. Remove dependence on ARP being the current screen.
3. Add missing instrument/bus processing and momentary FX after an explicit
   buffer/CPU budget. A drum DJ filter and targeted audio repeat are requirements,
   not existing capabilities implied by drawing a control.
4. Validate simultaneous gestures, pickup behavior, effect release, mute priority
   and long playback on the device. Host/emulator tests can cover state and
   routing; hardware testing establishes timing and tactile usefulness.

Reuse existing UI/undo resources where possible. Keep symbolic generation out
of audio rendering and keep the existing RAM reserve. A new screen alone must
not introduce additional audio buffers or pretend missing DSP is implemented.
