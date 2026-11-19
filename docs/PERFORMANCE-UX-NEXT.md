# Live7 / Live8 UX refinements — build only

Source changes on top of released live6 (5031e10), prepared for 1.2.1-live7.
Build results are recorded below. Tests were explicitly excluded by the user.
Published firmware and the public browser emulator remain live6. This local
live7 build contains the earlier behavior and has not been tested. Live8 adds
the display follow-ups below and direct SEQ=LIKE / GLO=RETURN controls, retaining
SEL+SEQ/GLO for classic editing/settings and the earlier capture combinations.
Live8 is built without tests by explicit user request; results follow below.

## Performance is home

- HOME returns from a classic editor/settings screen directly to LIVE, closing
  its overlay without accepting a pending confirmation.
- HOME in LIVE selects its native instrument controls and stays in performance.
  Linked selection, manual mix and musical material are retained.
- Plain SEQ/GLO taps in LIVE stay there and show the deliberate entry gesture.
- Hold SEL and tap SEQ for the classic step editor.
- Hold SEL and tap GLO for settings.
- Hold SEL and tap HOME for classic patterns. In classic views, SEL + HOME
  opens patterns, or switches patterns to song. Plain HOME returns to LIVE.
- HOME + EDIT remains an alternative return gesture. HOME + musical-key mutes,
  HOME + SEL mix restoration, and HOME + SEQ tap tempo are retained.
- Existing active diagnostic-test ownership of HOME/PLAY remains unchanged:
  those buttons first stop the diagnostic test rather than abandoning its state.

## Screen clarity

The performance surface still has exactly four displayed parameter controls,
mapped in reading order to K1–K4. Additional key-operated mute indicators are
allowed, but they must not imply additional knob controls.

The drum sound heading now includes the kit and selected instrument, using
the existing engine voice name (for example, 909 BD). It follows white-key and
PRESETS selection and remains visible on the extra sound controls.

## Mixer cycles and sliders

First LFO tap selects Mixer Level. Repeated LFO taps cycle Level -> Pan ->
Reverb -> Delay -> Level. In Mixer, SELECT cycles the same views in either
direction. HOME + SELECT changes tempo without leaving Mixer. Elsewhere in
LIVE, SELECT keeps its tempo function.

Each view shows exactly four vertical sliders left-to-right, corresponding to
K1=909, K2=808, K3=303 A, K4=303 B. Only the controlled quantity changes between
views; there is no hidden SEL-held pan/send layer. Pan is bipolar with a visible
center mark. Values retain the existing stored 0–127 units; pan displays its
offset from center (-64..63). Each slider can show M when its part is muted.
Musical-key part/instrument mutes and PRESETS mixer-kit selection are retained.

## Beat indication

The BPM number's background pulses for approximately 90 ms on each musical
beat. The first beat of the bar uses the stronger highlight. Text switches to
dark-on-bright during a pulse, retaining contrast. The pulse is triggered by
the existing musical step/bar position, not a free-running BPM timer; without
new beats it expires rather than continuing to flash. Stopped playback leaves
the area unhighlighted. This uses the same supported 909 bar reference as
performance automation and section timing.

## Continuous automation depth

Depth is now 0–127. The modulation radius is
`round(depth * target_max / 127)` in the target's native stored range (0..max).
Compute the requested movement around the captured center, then clip each value
at the target limits. Do not reduce the radius or recenter to make it fit.

For a 0–127 control at 5, depth 10 requests -5..15 and produces 0..15.
At 122, depth 10 produces 112..127. Depth 0 holds the captured center;
depth 127 can reach either limit. A 0–63 target scales depth 64 to a radius of 32.
Growing/falling saws use the corresponding upper/lower half of the same range.

K2 changes continuous depth, including during playback. Updating an active
gesture retains its captured center and musical phase. The existing shallow,
medium and deep key shortcuts now choose normalized depths 16, 32 and 48.
White key 11 starts/retriggers using the currently configured shape, depth and
length, allowing any custom depth to be used; white key 10 stops automation.
The other existing stop keys (12–16) retain their behavior.

## Momentary FX timing

Chopper now repeats one complete gate cycle per sixteenth note while held:
half the cycle open, half closed (16 chops per 4/4 bar). At 120 BPM this is a
125 ms cycle: 62.5 ms open, 62.5 ms closed. It starts open on keypress; tempo
changes update the rate while retaining its phase. Release still returns to the
live signal through the existing wet fade. Gate gain is smoothed rather than
low-pass filtering the audio stream itself.

Tape stop now slows linearly from normal speed to zero in 400 ms (17640 frames
at 44.1 kHz), rather than exponentially falling away near the beginning.
Amplitude is retained through most of the slowdown, then fades during the final
80 ms. The moving read head uses linear interpolation. Once stopped, output
stays silent while held; release returns to live audio. The existing capture
ring and its write guard are retained; no additional audio buffer is allocated.

## Scoped review and pending verification

Main: navigation, instrument labels, mixer cycling/sliders, beat highlight and
continuous-depth UI. Audio FX: sample-rate sixteenth gate, 400 ms tape-speed
envelope, bounded two-point tape interpolation and existing wet release. Audio:
the existing
control-queue consumer decodes the wider depth and updates eight fixed lanes.
The native radius is precomputed on start/depth commands, not per sample or tick.
The modulation tick retains its previous wave calculation and output clamps.
Baseline hardware: FM-1 / X0X 1.2.1, existing SDK/toolchain and execution model.
No synthesis, storage schema, loader, clock or driver changes. The internal
queue payload changes; the SDK ABI and persistent formats do not.

| Category | Status / evidence |
| --- | --- |
| Target / build / ABI | PASS for compilation/package scope: live7 built with the matched toolchain/SDK and retained manifest, size, register and loader checks. Tests are intentionally excluded. |
| Audio hot path / numerics | PASS for source bounds/structure: normalized radius precomputed on commands, fixed eight-slot scans, existing tick clamps, bounded stereo gate/envelope/interpolation. Chopper reciprocal is once per block, not per sample. No allocation/I/O/logging/waits/double/math-library additions. Wave*radius fits int32. Generated code/cycle cost remains UNVERIFIED. |
| Execution ownership / synchronization | PASS for source scope: UI remains on main; complete start/depth updates use the existing SPSC queue. Renderer owns lane writes; depth-only updates retain base/phase. |
| Memory / storage / I/O | UNVERIFIED for target totals/stack. Source budget: bounded 28-byte title buffer on UI stack, one extra radius byte per lane (8 pool bytes), one mixer-selection byte, bounded beat-display state, and 12 bytes of audio FX counter/phase/gain fields. No extra canvas/audio buffers, persistent state or storage operations. |
| Dependencies / licenses | PASS: no new dependencies/assets; existing engine voice names/fonts/notices reused. |
| Compatibility / package / recovery | Persistent formats unchanged; new package and device/recovery behavior UNVERIFIED. No package replaced or device flashed. |
| Host / browser validation | UNVERIFIED: intentionally not run. Navigation regressions and classic fixture gestures must be updated/checked before the next release. |
| Hardware real-time validation | UNVERIFIED: check physical chord consumption, cancellation, FX/note release and return to LIVE on the FM-1. |

Before building, verify standalone SEQ/GLO/HOME stay in performance; deliberate
entry chords consume SEL; HOME returns from editor, settings and confirmations;
mute/mix/tap-tempo combinations still work; selected drum labels change with keys
and PRESETS. Re-run relevant native/browser scenarios after approval.

Also exercise depths 0/10/64/127, low/high centers, target max 63/127, rejection
of -1/128, live depth edits without center/phase drift, custom-depth start with
white key 11, and LED bounds with depths 0/127. None of these checks has been run.
Verify LFO/SELECT cycle wrapping and per-track knob mapping for every mixer
quantity; centered pan, mute markers, HOME+SELECT tempo, and stopping/external
clock loss during a BPM pulse. Visually inspect the four columns at 240x240.
For FX, measure 16 gate cycles/bar at several tempos, including held tempo
changes; check full-band audio during open gates and release at every phase.
Verify tape speed reaches zero at 400 ms, stays silent while held, remains
audible during the early slowdown, and safely wraps the capture read head.
Run browser waveform/duration assertions and listen after build approval.

## Live7 build results — no tests run

Firmware source checkpoint: `c757989` (documentation-only build records may follow).
Built using `tools/build-fm1-wsl.sh --release 1.2.1-live7` with the existing
matched JieLi toolchain and AC79 SDK. No native, browser, functional, regression,
listening or hardware tests were run for this build, as requested.

- Emulator: 526356-byte WASM, compiled with X0X_VERSION=1.2.1-live7 via
  tools/build-emu-wsl.sh. It was not launched or exercised.
- Target app: 440540 bytes.
- Firmware package: 609682 bytes, identity FM-1_9010201.
- Package: build/x0x-1.2.1-live7.fwsc.
- SHA-256: 751774c28759275f0334de1742b4d8adf7e0caa72ef206487bdc048bb8979fa1.
- Ordinary RAM: 96428 / 98304 bytes (1876 free).
- Pool: 331752 / 344064 bytes (12312 free), retaining the 8192-byte reserve
  and leaving 4120 allocatable bytes above it.
- Build checks: 909 call-free RAM-code instructions, no software-double
  routines, register access confined to HAL, loader/package checks retained.

These are build/link observations, not runtime evidence. Memory totals PASS for
static linker scope; peak stack remains UNVERIFIED. Source-level bounds and
ownership review are recorded above; new DSP instruction cost, behavior, audible
quality, navigation and hardware deadlines remain UNVERIFIED until tests are
separately authorized. Storage formats, dependencies and licenses are unchanged;
actual installation/recovery remains UNVERIFIED. Nothing was flashed.

## Unbuilt follow-up: visible momentary filter position

This source follow-up is not included in the compiled live7 artifacts above.
No rebuild or tests were requested/run for it.

Momentary FX now retains four left-to-right sliders (K1 filter, K2 pump,
K3 delay feedback, K4 reverb level), even while a gesture is held. The filter
is bipolar with a center mark at 64 and a 0–127 position readout. Filter and
pump positions read the engine's actual pots, so temporary LP/HP/pump overrides
are visible rather than displaying the unchanged saved project values.
The header names a held gesture; the footer names the actual filter mode and
whether a gesture is held. Releasing restores the underlying live readout.

Source review: UI readback/drawing and latched-layer priority only; existing
getter functions, fonts and canvas reused. No audio processing, queue protocol,
persistent storage, dependency or loader changes. The loops stay bounded to
four displayed controls. Target build/ABI, exact memory/stack, functional/browser
behavior, package/recovery and hardware timing are UNVERIFIED for this follow-up.

## Unbuilt follow-up: instrument sliders and numeric alignment

Instrument controls now occupy four columns in physical K1–K4 order. Continuous
303 and drum parameters use sliders scaled to their parameter descriptor's
native range; named choices such as waveform retain text. Active automation
values are shown when available. Mixer pan and individual drum pan use horizontal
bipolar sliders with a center mark. Numeric readouts are right-aligned, including
the BPM, bar counter and existing automation/director controls. The selected drum
instrument remains named in the header. Fonts remain 20/24/36 px.

This source-only change is not in the live7 firmware or emulator artifacts.
No build or tests were run, following the user's instruction to ask first.

| Compliance category | Status and evidence |
| --- | --- |
| Target / build / ABI | UNVERIFIED — no rebuild for this follow-up; live7 remains the compiled baseline. |
| Audio hot path / numerics | PASS, source scope — changes are UI drawing only; no renderer or DSP changes. |
| Execution ownership / synchronization | PASS, source scope — existing main-loop parameter getters and automation readback reused; bounded four-control loops. |
| Memory / storage / I/O | UNVERIFIED — fixed local text buffers and existing canvas reused; target size and peak stack require a build/measurement. No storage or peripheral changes. |
| Dependencies / includes / licenses | PASS, source scope — no dependencies, assets or includes added; existing notice retained. |
| Compatibility / package / recovery | PASS for source scope — no persistent format, loader or package contract changed. Actual target package/recovery remains UNVERIFIED. |
| Host functional validation | UNVERIFIED — no tests or browser exercise authorized for this follow-up. |
| Hardware real-time validation | UNVERIFIED — no device validation; inspect display legibility, slider endpoints/center and UI responsiveness during playback after approval. |

## Live8 build results — no tests run

Firmware/emulator source checkpoint: `1fa8670`. This build includes both formerly
unbuilt display follow-ups and the SEQ=LIKE / GLO=RETURN remap. The existing
ENV+OCT+ and LFO+OCT- gestures still work. SEL+SEQ/GLO retain classic access.
The LIKE helper reuses the existing main-loop capture mask, selected-part mask
and bounded four-part pattern snapshot; RETURN uses the existing perform queue.
No new audio/DSP logic, persistent fields, dependencies or buffer allocations.

Commands: `tools/build-fm1-wsl.sh --release 1.2.1-live8`, followed by
`X0X_VERSION=1.2.1-live8 tools/build-emu-wsl.sh`, using the existing matched
JieLi toolchain/AC79 SDK and Emscripten 3.1.69.

- Target application: 441720 bytes.
- Ordinary RAM: 96428 / 98304 bytes (1876 free).
- Pool: 331752 / 344064 bytes (12312 free, 8192-byte reserve retained).
- Firmware: `build/x0x-1.2.1-live8.fwsc`, 609682 bytes, FM-1_9010201.
- SHA-256: `00a080d01d89e30df8dd80921b689522505b91815e531c2ead0fd4fc1631834f`.
- Matching emulator: 531667-byte WASM. Optional page voice absent; existing
  screen-reader support remains available.
- Build integrity checks: 909 call-free RAM-text instructions, no software-double
  routines, HAL-only register access; retained SDK/loader/package checks passed.

Compliance supersedes the unbuilt statuses above: target/build/ABI PASS for
compilation/link/package scope only; audio hot path/numerics and execution
ownership PASS for the unchanged DSP and bounded UI/capture source scope;
memory PASS for static linker budgets, peak stack UNVERIFIED; dependencies and
licenses PASS (none added); compatibility PASS for unchanged formats/contracts,
physical installation/recovery UNVERIFIED. Host functional/browser/listening
validation and hardware real-time validation remain UNVERIFIED. No tests were
run, no emulator runtime behavior was exercised, and no hardware was flashed.
After test approval, validate four-column controls, numeric alignment, pan
center/endpoints, held filter readback, direct LIKE/RETURN and retained modifier
gestures, plus the earlier live7 navigation/automation/FX scenarios above.
