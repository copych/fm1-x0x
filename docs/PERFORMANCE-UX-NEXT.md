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

## Unbuilt follow-up: consistent Auto / Director columns and cleaner pan

Baseline: live8 source `1fa8670`, build record `3655aa0`. Changes are confined to
main-loop LIVE drawing in `live_surface.h`, with this documentation update.
Auto now follows physical left-to-right K1 destination, K2 depth, K3 bars,
K4 shape. Depth uses the same vertical fader as instrument/mixer controls;
shape adds a bounded 24-point contour above its name. Director also uses four
columns: K1 Weird, K2 Full, K3 Bars, K4 Next. Its two continuous controls have
vertical sliders, and section choices use short names that fit at 20 px.
The current/pending section context elsewhere retains full names.
Pan uses a thin rail, a separate center tick and a compact inset handle;
value 64 maps exactly to center, with both extreme handles inside the rail.
20 px minimum fonts and right-aligned numeric readouts are retained.

No build/tests requested or run for this patch; the served emulator remains
live8. No parameter mapping, automation phase, section scheduling or DSP changes.

| Compliance category | Status and evidence |
| --- | --- |
| Target / build / ABI | UNVERIFIED — source-only changes after live8, no compilation. |
| Audio hot path / numerics | PASS for source scope — UI-only integer drawing, no audio-path edits. |
| Execution ownership / synchronization | PASS for source scope — existing main-loop getters/canvas reused, four-control and 24-point bounded loops. |
| Memory / storage / I/O | UNVERIFIED — fixed local buffers and existing canvas; new linker/stack totals unmeasured. No storage/peripheral changes. |
| Dependencies / includes / licenses | PASS — no dependencies/assets/includes added, notice retained. |
| Compatibility / package / recovery | PASS for unchanged persistent and loader contracts; installation/recovery UNVERIFIED. |
| Host functional validation | UNVERIFIED — builds and tests await separate authorization. Check all columns, shape names/contours, pan center/extremes and long section choices. |
| Hardware real-time validation | UNVERIFIED — check legibility and redraw responsiveness during loaded playback on FM-1. |

## Live9 scope and pre-build review

Includes the Auto/Director/pan follow-up plus latched Momentary/Delay/Reverb FX
views. FX taps and SELECT cycle views, HOME+SELECT retains tempo, and white-key
gestures retain their existing master targets. Four knobs reuse existing FX
parameter references: delay time/feedback/tone/return and reverb decay/tone/HPF/
return. Delay division uses the existing named choices. New finite view state
uses `enum eLiveFxView` and a count sentinel, resets at initialization, remains
session-only and is not serialized. Gesture release retains existing behavior.

Source review: changes execute in the UI main loop, retain the existing command
publication and getter mechanisms, add no DSP/math in the render path, no waits,
allocation or I/O in audio. Loops are bounded; drawing clamps descriptor ranges.
Dependencies/includes/licenses and storage/package contracts remain unchanged.
Build/ABI, new linker/stack totals, functional validation and hardware timing
are pending. Tests are explicitly excluded; documentation screenshots will be
captured from WASM without regression assertions or audio qualification.

## Live9 build and publication record

Compiled application source: `2b570c9`. Subsequent capture/documentation changes
do not change the compiled application. Built with the existing matched JieLi
toolchain and AC79 SDK using `tools/build-fm1-wsl.sh --release 1.2.1-live9`;
emulator via `X0X_VERSION=1.2.1-live9 tools/build-emu-wsl.sh` (Emscripten 3.1.69).

- App 442704 bytes; ordinary RAM 96428 / 98304 bytes; pool 331752 / 344064,
  preserving the 8192-byte reserve.
- Firmware `build/x0x-1.2.1-live9.fwsc`, 609682 bytes, FM-1_9010201.
- SHA-256 `8d2763381dc9b0cc7f73df35aac6fc458c2c220d0db5257cd32769655e8f35e7`.
- Matching emulator 534342-byte WASM; optional page voice absent.
- Retained build checks: 909 call-free RAM-text instructions, no soft-double
  routines, HAL-only register access, SDK/loader/package identity/size checks.
- Documentation captures: `node tools/capture_live_screens.mjs`; nine actual
  240×240 WASM framebuffer PNGs. Framebuffer byte order matches the browser.
  Captures are documentation evidence only; no functional/audio assertions run.

| Compliance category | Status and evidence |
| --- | --- |
| Target / build / ABI | PASS for compile/link/package scope — matched toolchain/SDK and retained integrity checks above. |
| Audio hot path / numerics | PASS for source scope — UI-only additions; audio processing unchanged, build rejects soft-double routines. |
| Execution ownership / synchronization | PASS for source scope — bounded main-loop view/drawing logic uses existing getters and parameter publication; no new queue or ownership. |
| Memory / storage / I/O | PASS for static linker budgets above; peak stack UNVERIFIED. Existing canvas/storage/peripheral contracts unchanged. |
| Dependencies / includes / licenses | PASS — no firmware dependencies or assets introduced, existing notices retained; capture utility uses Node built-ins. |
| Compatibility / package / recovery | PASS for unchanged persistent formats and retained package checks; physical installation/recovery UNVERIFIED. |
| Host functional validation | UNVERIFIED — no test suites or audio/listening tests run; nine documentation screenshots captured and visually reviewed for publication. |
| Hardware real-time validation | UNVERIFIED — no hardware flashed or measured; loaded playback, timing/stack/soak and recovery remain pending. |

After approval, exercise FX view cycling in both directions, time division
bounds, each return control, held gestures/release across views, manual sound
and mute preservation, plus the earlier Auto/Director/navigation/FX scenarios.

## Unbuilt follow-up: compressor, combined 303 DRIVE and repeated selection

Baseline: published live9, application `2b570c9`, source/docs `81506e1`.
FM-1/AC79, existing JieLi/SDK and 44.1 kHz audio execution model retained.
Changed functions are main-loop UI mapping, drawing and gesture handlers in
`ui.c`, `live_surface.h`, `performer_ui.h`. Audio render/DSP unchanged; previous
live9 image/RAM/pool numbers above are baseline only, not new measurements.

- FX/SELECT cycles Momentary, Delay, Reverb, Compressor, Comp Detail.
  Compressor K1–K4: amount macro, attack, release, dry/wet mix.
  Comp Detail K1–K4: threshold, ratio, makeup, kick pump. The amount macro
  intentionally updates threshold/ratio/makeup through the existing setter;
  subsequent individual edits remain possible. Existing saved parameters reused.
- LIVE 303 extra controls: Accent, Wave, combined Drive, Slide. No separate
  type chooser on LIVE. Combined position 0 bypasses; 1–63 uses SOFT amounts
  `1 + floor((position-1)*126/62)`; 64–127 uses RAT, rank `position-63`.
  RAT ranks 1–5 map 1:1; ranks 6–64 map `6+floor((rank-6)*121/58)`.
  Examples: position 1→SOFT1, 63→SOFT127, 64→RAT1, 68→RAT5,
  69→RAT6, 127→RAT127. Only RAT has the soft toe, per user correction.
  Existing DSP type/amount fields remain stored separately; loading old settings
  does not rewrite them. The combined display uses the nearest representable
  position. Turning Drive writes both existing parameters; linked 303s receive
  relative changes independently. No added encoder acceleration skips the toe.
  Switching at 63/64 intentionally changes SOFT127 to RAT1; audible behavior
  and the existing two-command publication at that boundary require validation.
- Unmodified repeated selection of the focused black key calls the same EDIT
  helper: enter native sound from another mode, then toggle native/extra.
  Selecting another part still enters its native controls; SEL linking, HOME
  mutes, REC solos and Mixer black-key mutes retain priority.

| Compliance category | Status and evidence |
| --- | --- |
| Target / build / ABI | UNVERIFIED — no build authorized for this follow-up. |
| Audio hot path / numerics | PASS for source scope — integer mapping outside audio, existing DSP setters/processing reused, no renderer edits. |
| Execution ownership / synchronization | PASS for existing structural ownership — main-loop handlers use existing pref setters/command queue and bounded part loops; paired parameter audible behavior UNVERIFIED. |
| Memory / storage / I/O | UNVERIFIED for new target image/stack; no dynamic allocation, new audio buffers, storage fields or I/O added. |
| Dependencies / includes / licenses | PASS — no new dependencies/includes/assets, existing notices retained. |
| Compatibility / package / recovery | PASS for unchanged storage/package contracts; native legacy pots retained. Device installation/recovery UNVERIFIED. |
| Host functional validation | UNVERIFIED — no tests run. After approval, sweep all 128 Drive positions/inverse readback, boundaries and linked settings, compressor macro/detail/save restoration, repeated-key modes and modifier priorities. |
| Hardware real-time validation | UNVERIFIED — no hardware flashed/measured; listen for boundary clicks and verify encoder response, worst load and headroom after authorization. |

Not rebuilt, captured, published or tested. Public/local served artifacts remain
live9; this section describes source changes for a future build.

### 303 extra K4: transpose instead of slide

The freed fourth control is now TRANS, −24..+24 semitones, beside Accent,
Wave and combined Drive. It reuses the existing per-303 current-pattern
`R_BTRANS` field (stored 0..48, center 24). Notes are not rewritten. Display
uses a centered bipolar vertical slider and signed right-aligned values;
linked selection adjusts each selected 303 relative to its existing transpose.
The existing sequencer applies transpose at note events and clamps the result
to MIDI 0..127, including outgoing sequencer MIDI. It does not instantly retune
an already held note, and does not transpose incoming MIDI performance notes.
Pattern save/load and musical capture retain their existing transpose behavior.

Source-only mapping/drawing change in `ui.c` and `live_surface.h`; main-loop
getter/setter path and existing shared pattern ownership retained. No sequencer,
DSP, queue, format, allocation, dependencies, includes or license changes.
Compliance: audio-path/numerics, existing ownership, dependencies/licenses and
unchanged format/package contracts PASS for inspected source scope. Build/ABI,
new linker/stack totals, host behavior, physical recovery and hardware real-time
validation remain UNVERIFIED. No builds/tests/captures/publication run.
After approval, check −24/0/+24, signed display/center, independent and linked
303 edits, pattern switching/save/load/LIKE/RETURN, and note-range clipping.

## Live10 build record — no tests run

Application source checkpoint: `43e24e2`. Includes compressor tuning, combined
303 Drive with RAT-only toe, repeated focused black-key EDIT toggling and
current-pattern ±24-semitone transpose. Later README/build-record updates do not
change the compiled application. Built for FM-1 using the existing matched
JieLi toolchain and AC79 SDK:
`tools/build-fm1-wsl.sh --release 1.2.1-live10`, then
`X0X_VERSION=1.2.1-live10 tools/build-emu-wsl.sh` (Emscripten 3.1.69).

- Application: 443704 bytes.
- Ordinary RAM: 96428 / 98304 (1876 free).
- Pool: 331752 / 344064 (12312 free), retaining 8192-byte reserve.
- Package: `build/x0x-1.2.1-live10.fwsc`, 609682 bytes, FM-1_9010201.
- SHA-256: `d57c4c7680a27c5c3b7ea6d566c4ae1bd6571f5c933c14481eca2339896b77aa`.
- Matching emulator: 537468-byte WASM; optional page voice absent.
- Retained build checks: 909 call-free RAM-text instructions, no soft-double
  routines, HAL-only register access, SDK/loader/package integrity checks.

Compliance: target/build/ABI PASS for compile/link/package scope; audio hot
path/numerics and execution ownership PASS for reviewed UI-only changes using
existing setters/pattern fields, with paired Drive-command audible behavior
UNVERIFIED. Memory PASS for static linker budgets, stack UNVERIFIED.
Dependencies/includes/licenses PASS (unchanged). Compatibility PASS for unchanged
storage/loader contracts, physical installation/recovery UNVERIFIED. Host
functional and hardware real-time validation UNVERIFIED: no tests, listening,
browser runtime exercise or fresh screen captures run for this build. No device
flashed. The concrete pending checks are listed in the follow-up sections above.

Local install/emulator pages are prepared for live10. Source and build record
are committed locally; no GitHub push or public Pages deployment requested.

## Live11 implementation and validation scope

Baseline: live10 application `43e24e2`, source/build record `6122972`. Board and
bootloader/package identity unchanged: FM-1 / AC79, FM-1_9010201, existing matched
JieLi/AC79 SDK. 44.1 kHz stereo, engine chunks ≤256 frames within the existing
512-frame I2S half (~11.61 ms). Hardware timing remains unmeasured.

Changes: new `live_drums.h` and `live_quantized_fx.h`, engine/render scheduling,
main-loop performer controls/drawing, and a shared implementation wrapper in
`master.c` retaining the previous stereo path as well as an unfiltered variant.
Drum-bus filter reuses the existing master's prepared cutoff/resonance
coefficients, with four independent state pairs for drum L/R and their reverb/
delay sends. It runs after the 808 worker has retired and before mixing 303s.
No audio buffers added. Mode weights crossfade over 96 samples; cutoff glide,
SVF arithmetic and denormal flushing follow existing master filtering. Bypass
is exact after the weights return to zero. Master compression/limiting stay
global; existing send-effect tails bypass the drum filter.

UI sends complete commands on the existing SPSC queue (named subcommands 253
momentary, 254 roll, 255 reset), with bounded audio-owned state and no new shared
staging. Momentary starts wait for the 909's next step callback before drum hits;
release cancels a pending start or restores an active override. The chopper's
open edge resets at each 16th; its gain is open at the attack and closes during
the tail. Master processing and audio gestures now run per sequencer subchunk,
so a front cannot retroactively affect earlier audio in the same render block.
Saved pot snapshots restore only the regular held gesture; rolls are separate.

Rolls: white 10/11/12 in every FX view, 1/8, 1/16, 1/32, current snare of the
last focused kit. A deliberate roll auditions a muted pattern voice without
changing its state; whole-kit mute wins. It replaces sequenced snare triggers
while active, preserves pattern data, and leaves the final snare tail on release.
Midpoint hits split the existing subchunk loop and follow half the actual
upcoming interval, including swing. Rolls require internal clock; transport stop,
unsupported clock/length, mode change or key release cancels them. Effects and
rolls are discarded on project reset. All audio loops have fixed small bounds;
no allocation, I/O, logging, locks, waits, new math-library calls or doubles.
Coefficient divide/branch costs and peak stack still need target measurement.

New WASM tests cover all 128 combined Drive positions and inverse readback,
transpose bounds, repeated selection, compressor controls, all nine quantized
effect onsets, early cancellation, chopper attack/tail phase, roll rates and
simultaneous LP/knob/release/stop, finite/bounded audio, isolated 303 bit-identity
under drum filtering, and audible change on isolated drums. Browser-only
readback/counters are behind X0X_WEB and absent from target code/state.
Legacy scenario fixtures now explicitly use SEL+HOME to reach classic views;
their pattern/storage/audio assertions remain. The probability-reset fixture
needs one such gesture because it closes settings and enters patterns together.
Effect fixtures allow the new next-16th onset rather than assuming immediate FX.

The first run identified those stale navigation/timing fixtures and a test-export
linkage error (fixed through read-only engine getters). Initial isolated-drum
fixture had no generated groove; it now generates one and asserts an audible
reference before routing comparisons. Three external DSP comparisons could not
run without ../schwung-9W9, ../schwung-8W8, ../schwung-303. The runner now supports
explicit X0X_SKIP_EXTERNAL_REFS=1 for that scoped run; default requirements stay
unchanged. TB3PO reference is also absent; sequencer tests run with that optional
comparison omitted. Retired breakbeat scenarios remain explicitly skipped.

Pre-build source gate: audio hot path/numerics and ownership PASS for inspected
bounded structure, dependencies/includes/licenses PASS (no new firmware deps,
notices retained), compatibility PASS for unchanged persistent/loader contracts.
Target/build/ABI, updated memory/stack, complete host validation and hardware
real-time/recovery remain UNVERIFIED until the following results are recorded.

Restore ordering correction: `app_restored()` now queues gesture reset before
applying the loaded sound, in addition to its UI initialization reset. Otherwise
an active old filter snapshot could restore over newly loaded pots. A WASM-only
test callback simulates storage completion at a stopped boundary before another
render has retired the old gesture; it invokes the actual restore function.
The regression saves filter 64, moves to 54, activates temporary LP, restores,
and asserts both the project mirror and actual engine return to 64 with no
active/pending effect or roll. It passes. This test hook is absent on FM-1.

## Live11 final build and validation results

Final application source: `6e841fd` (main feature commit `a5b14ac`). Later changes
are this build record only. Firmware built with the existing matched JieLi
toolchain / AC79 SDK via `tools/build-fm1-wsl.sh --release 1.2.1-live11`.
The final scoped regression command rebuilt host and matching WASM with
`X0X_SKIP_EXTERNAL_REFS=1 X0X_VERSION=1.2.1-live11 tools/test-fm1-wsl.sh`.
Exit status 0: all available tests passed, including all 46 supported application
scenarios and all four emulator suites. The initial unrestricted run explicitly
failed the three unavailable reference comparisons; those were not passed.

- App: 446148 bytes.
- Ordinary RAM: 96512 / 98304 (1792 free), +84 bytes from live10.
- Pool: 331752 / 344064 (12312 free), 8192-byte reserve retained.
- Package: `build/x0x-1.2.1-live11.fwsc`, 609682 bytes, FM-1_9010201.
- SHA-256: `568972d708a42d8979b779a2b353a287a1492890f98efc2740eb28b6c98fb9ad`.
- Final emulator: 542524-byte WASM; Emscripten 3.1.69.
- Retained build checks: 909 call-free RAM-text instructions, no soft-double
  routines, HAL-only register access and SDK/loader/package integrity checks.
- Generated-code review: new unfiltered master entry calls the existing shared
  stereo implementation; drum SVF/roll control are inlined into existing render
  and event paths. Division and FPU instruction costs are not target-measured.
- Final WASM/host match: 168021 samples, peak 0.261, maximum difference 3.05e-5
  (16-bit WAV rounding), framebuffer and lights present.
- Captured repeats: 1/8 10584, 1/16 5292, 1/32 2646 frames; maximum cycle error
  1.19e-7, peaks 0.156 / 0.124 / 0.094. Tape reaches silence, chopper gates audio.
- New live suite passed all cases described above, plus 808 target selection,
  whole-kit mute/unmute, early roll cancellation and the restore-order case.
- Native master/stereo/reverb, drum/hash, sequencer, generation/motif/director,
  motion, MIDI, storage/backup, installer/OTA/update-loader checks passed.
- Explicit skips: 3 absent external DSP references, 8 retired breakbeat scenarios,
  optional TB3PO external comparison. Optional browser page voice remains absent.
- Browser smoke: fresh isolated origin `127.0.0.1:8002` booted, generated/played,
  entered FX, accepted roll/filter keyboard input, cycled to Compressor with the
  correct four controls, and stopped. This is basic UI/AudioWorklet evidence;
  precise timing/concurrency assertions come from the WASM suites. Original
  emulator origin's saved project was not edited/reset. No listening verdict.
- Thirteen actual WASM framebuffer captures generated with
  `node tools/capture_live_screens.mjs`; new extra/compressor/roll views visually
  reviewed. They are documentation, not hardware screen/legibility measurements.

| Compliance category | Final status and evidence |
| --- | --- |
| Target / build / ABI | PASS for compile/link/package scope — matched compiler/SDK, retained build checks and identity above. |
| Audio hot path / numerics | PASS for inspected bounded source and generated-helper checks; single-precision existing SVF/filter coefficients, no new allocation/I/O/log/wait/math-library dependencies. Device instruction cost/headroom UNVERIFIED. |
| Execution ownership / synchronization | PASS for inspected queue/audio/worker structure and tested concurrency/release/restore scenarios. Existing bounded drop-on-full queue policy retained; stop/reset discard gestures. Extreme queue-pressure behavior remains UNVERIFIED. |
| Memory / storage / I/O | PASS for static linker and buffer-bound scope, no new audio buffers/storage fields; peak stack and flash/I/O contention on device UNVERIFIED. |
| Dependencies / includes / licenses | PASS — no new firmware dependencies; existing notices retained, screenshot utility uses Node built-ins. |
| Compatibility / package / recovery | PASS for unchanged persistent contracts, host restore and loader/package checks. Physical install/rollback/recovery UNVERIFIED. |
| Host functional validation | PASS for scoped suite above, with explicit unavailable/retired skips. Browser smoke is separate limited evidence. |
| Hardware real-time validation | UNVERIFIED — no hardware flashed, timed or soaked; no physical audio/USB output or listening qualification. |

Before hardware qualification, measure worst render/service cadence versus the
11.61 ms half-buffer deadline with both kits, two driven 303s, high resonance,
roll/filter plus repeat/FX tails, tempo extremes, busy UI/MIDI/USB; measure peak
stack, queue pressure, underruns and clipping, listen for mode/release clicks,
and verify stop/clock loss, save/restore and recovery on the actual FM-1.

## Unbuilt follow-up: revert separate drum-filter routing

User requested reverting the drum/mix split after live11. Baseline source
`7820ddc`; compiled application `6e841fd`. Source now restores the existing
stereo master filter on the complete mix, after shared delay/reverb returns.
The filter therefore affects drums, 303s and effect tails together. Removed the
four-channel drum-filter state/loop and the unfiltered master wrapper; master.c
and master.h match their pre-live11 implementation. Original drum/303/send
mixing order is retained. No extra mix buffers are introduced.

Per-event subchunk processing remains so momentary fronts stay beat-aligned.
Snare roll scheduling, independent roll/filter operation, chopper attack/tail
alignment and restore-order protection are retained. LIVE identifies FX as
GLOBAL; README/performer/emulator help reflect master filtering. Existing screen
captures still depict the published live11 build until refreshed after approval.
The WASM routing regression now requires an audible filter change for isolated
303s as well as isolated drums; it has been updated but not run.

Changed paths: engine render call, snare helper's removed filter/reset state,
master DSP wrapper/header, LIVE heading, routing assertions and documentation.
Audio execution remains in the same render owner after worker retirement;
filtering reuses the original master loop and prepared coefficients.

Compliance: audio hot-path/numerics, ownership, dependencies/includes/licenses
and unchanged storage/package contracts PASS for inspected source scope, with
master DSP source restored exactly to the prior implementation. Target/build/ABI,
new linker/stack totals, host functional validation, physical installation/recovery
and hardware real-time validation remain UNVERIFIED for this revert. No build,
tests, captures, flash or publication run. After approval, run master/stereo,
native scenarios and all four WASM suites, checking full-mix routing, synced FX,
roll combinations, stop/release and restoration. Served/public artifacts remain
live11 with the previous drum-only routing until a new build is requested.
