# Live6 validation and hardware limits

This is an experimental FM-1 build, not a hardware-qualified release.
Baseline: a4d982a (live5 plus public-site tooling), based on X0X v1.2.1
502b2c28c14ab2bff31eb4f0777fdfd0efcf8bea. Breakbeat remains disabled.
The commit containing this report identifies the delivered source.

## Build and execution record

Board: M-VAVE FM-1 / AC79. Existing pi32v2 r3 toolchain
jieli-linux-toolchains-20260730.1, matched AC79 SDK
`d179b4484759423312073f5fbb232501aa491047`. Existing build, manifest, loader,
register-access, package-size and software-double checks are retained.

Audio: 44.1 kHz stereo; engine_render accepts up to 256 frames per invocation.
The existing 512-frame I2S half-buffer schedule has an approximately 11.61 ms
service period. Host timings do not establish device headroom.

Main owns UI, project mirrors and symbolic generation. Existing SPSC control
queue transfers complete compact commands to the renderer; form, automation and
capture DSP runtime are audio-owned. The optional existing 808-worker model is
unchanged. No allocation, file/flash I/O, logs, waits or locks are introduced in
the audio path.

## Changed hot path

engine_render -> drain -> live_runtime_command / live_audio_command prepares
bounded state. live_runtime_tick scans eight fixed lanes at control rate;
s_step updates the shared musical position and applies boundary values before
hits. mot_apply defers a recorded lane while a preset gesture owns its parameter.
Manual queued parameter changes cancel the matching gesture.

master_process_st -> live_audio_process scans at most 256 stereo frames. A fixed
16384-frame stereo int16 ring captures real processed audio. Repeat reads are
bounded by the amount of recorded history; tape uses a wrapped, slowing read
head; chopper uses musical steps. Wet entry/release and repeat seams are smoothed.
All numeric DSP literals are float; no new libm calls or double helpers are used.
Counters saturate where clock loss or a long held gesture would otherwise grow.

The old queue's overload policy remains drop-on-full; this change introduces no
waiting. UI readback may observe adjacent state fields across an audio update,
which can affect one display frame but does not govern audio publication.

## Compliance gate

| Category | Status and evidence |
| --- | --- |
| Target / build / ABI | PASS for target build and retained SDK/manifest/loader/package checks. Source changes stay in the application and font generator. |
| Audio hot path / numerics | PASS for source and generated-code structural checks: fixed eight-lane scans, bounded stereo processing, float math, no allocations/I/O/locks/waits or soft-double linkage. Device CPU headroom remains UNVERIFIED. |
| Execution ownership / synchronization | PASS for source/host scope: complete commands through the existing SPSC queue, renderer-owned runtime, no new worker or interrupt arrangement. |
| Memory / storage / I/O | PASS for static link budget and storage/backup regressions. Capture replaces 32768 int16 delay samples with the same byte count; automation adds 80 pool bytes. No new canvas, storage schema or flash partition. Target peak stack remains UNVERIFIED. |
| Dependencies / includes / licenses | PASS: no new dependency. Existing Barlow fonts (SIL OFL), GPL sources and notices retained. Dedicated 20/24/36 px fonts avoid shrinking LIVE text; legacy editor fonts are unchanged. |
| Compatibility / package / recovery | PASS for host/package scope: project/backup/update/loader checks retained, stored track IDs unchanged. Delay buffer is intentionally shorter; long requested delays clamp. Real device installation/rollback remains UNVERIFIED. |
| Host functional validation | Native performer scenarios, full available suite and matching browser-WASM regressions; see results below. |
| Hardware real-time validation | UNVERIFIED: no hardware flashed or measured. Dense playback, latency, stack, underruns, analog/USB outputs, sustained use and recovery still require the intended FM-1. |

## Functional evidence

Native scenarios cover latched sections and linked relative controls, drum
instrument mutes, audio effect release on selection changes, musical undo/RETURN,
phase-aligned automation, manual takeover and section extension.

Browser-WASM tests exercise real UI button/key/encoder handlers. A four-bar grow
engaged in bar 3 joins the existing phase and resets at the square boundary.
A manual knob cancels it. Extending to five bars postpones BREAK, shrinking the
break advances DROP, and both new sections establish bar 1 origins.

Repeat tests at 125 BPM validate the captured audio's periodicity: 1/8 = 10584
frames, 1/16 = 5292, 1/32 = 2646. Observed cycle error is approximately 1.19e-7,
with nonzero captured audio and finite bounded samples. Tape stop fades into
silence; chopper creates rhythmic gaps; key release returns to the live path.
These are behavior tests, not device timing measurements.

The actual local browser emulator was opened and operated, including sound,
latched Mixer and Automation screens. Screens were visually inspected.

The full runner reports three unavailable external comparisons (9W9, 8W8 and
schwung-303); their sources are absent, so these are not passes. The optional
TB3PO reference and browser speech voice are also unavailable. Eight legacy
breakbeat scenarios remain retired. Existing unrelated signedness warnings in
project.c/ui.c remain. Host/browser comparisons and available engine, generator,
sequencer, motion, MIDI, storage, backup, loader and installer checks are run.

## Artifact record

Final target app: **438768 bytes**; package: **609682 bytes**, identity **FM-1_9010201**. Ordinary RAM occupancy **96416 / 98304 bytes** (1888 free). Pool **331744 / 344064 bytes** (12320 free), preserving the 8192-byte reserve and leaving 4128 allocatable bytes above it. The flash RAM-code closure is 909 instructions with no calls; software-double and register-access checks pass.

Package SHA-256: `2b8260287e57742d222a38243821d85513132127cf2662ac6117900abc33e6d5`.

Full run before the final session-reset fix: all **46 supported application scenarios** pass, plus available generator/DSP/MIDI/storage/update tests and all three browser suites. The full runner exits 1 solely because the three external engine comparison checkouts are missing. Final focused capture/solo and project-restore checks were rerun separately after that fix. The matching browser suites were also rerun after the final power-of-two capture scaling change. Final WASM: 521413 bytes. Native/browser comparison: 168021 samples, peak 0.261, maximum difference 3.05e-5; screen and lights present. Browser performer tests confirm phase/reset, takeover, revised segment origins, finite tape/chopper output and repeat cycle error 1.19e-7.

Commands: `tools/build-fm1-wsl.sh --release 1.2.1-live6`; `X0X_VERSION=1.2.1-live6 tools/test-fm1-wsl.sh`; native scripts `tests/scenarios/performer*.x0x`; browser script `node tests/host/emu_performer_test.mjs build/emu/x0x.wasm`. The Windows helpers run in the installed Ubuntu environment. Individual logs are in `build/host/`. The page voice is absent; detailed screen-reader descriptions for the new custom sections remain limited.

## Hardware follow-up

Install only after backing up the currently installed firmware's data with its
own editor (Sloop backups require Sloop's editor). Retain the appropriate rollback
image and recovery instructions; cross-firmware restore is not presumed.

On hardware, measure render/service maxima, underruns and stack high-water with
two dense 303s, both kits, delay/reverb, eight preset lanes, section boundaries,
repeated capture gestures and MIDI/USB activity. Check physical key/modifier
reach, takeover, output clicks, channel order and a sustained soak. Exercise
normal install/reboot/rollback on the intended device. Until measured, timing,
stack and recovery remain UNVERIFIED.
