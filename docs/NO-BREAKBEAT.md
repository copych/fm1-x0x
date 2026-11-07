# Four-part performance build

`1.2.1-live3` removes the breakbeat player from the default custom firmware.
The active ensemble is 909, 808, 303 A and 303 B. The Musical Director, drum
generator, LIKE/RETURN, both drum synthesis engines and shared effects remain.

BREAK no longer appears in part selection, HOME, the mixer, send display,
song columns or MIDI channel settings. Its live-key and MIDI playback, motion
lanes, audio decoding, render buffer and built-in sample bank are inactive.
The browser emulator hides the break-loop uploader as well.

Existing project track IDs, arrays, sound slots, pattern fields and storage
versions remain unchanged. Old break notes/slices/loop selections survive
save/load but remain silent. User sample storage is not erased or repartitioned.
The fixed-size install package remains 609682 bytes; a smaller app image does
not enlarge the user sample partition.

## Measured target memory

Source baseline: `9efa424` (Director build). Same FM-1 linker layout, JieLi
pi32v2 r3 clang 4.0.1, AC79 SDK and release build checks. At 44.1 kHz, the
existing 512-frame I2S half-buffer and 256-frame renderer are unchanged.

| Allocation | live2 | live3 | Reclaimed |
| --- | ---: | ---: | ---: |
| Ordinary RAM region occupancy | 98068 B | 96288 B | 1780 B |
| Buffer pool | 335852 B | 329568 B | 6284 B |
| App image | 496536 B | 375040 B | 121496 B |

Remaining ordinary RAM: 2016 bytes. Remaining pool: 14496 bytes, of which
8192 remain reserved by the existing integrity check. This leaves 6304 bytes
available for additional pool allocations. App-slot headroom: 206524 bytes.

The pool saving is the 6284-byte `breaks_t` state. Ordinary RAM savings include
the 1024-byte render buffer and decoder/slot bookkeeping. Flash savings include
110250 bytes of built-in loops plus removed playback code and associated data.
Allocation figures are from the target ELF/build, not host `sizeof` estimates.

## Implementation and checks

`X0X_BREAKBEAT` defaults to zero in `engine.h`. Dormant stored IDs still use
`NPARTS`; visible/control paths use `NPERF_PARTS`. Conditional render code removes
the player while retaining the existing 808 worker/fallback and mixer ownership.
No DSP algorithms, gain staging, delay capacity, display allocation or undo
capacity were changed. Existing upstream player source remains for reference
and its standalone tests still run.

The no-breakbeat application scenario injects a legacy break pattern, saves and
reboots it, verifies its data survives, and measures silence during playback.
It also checks ALGORITHM stops at 303 B and the fifth HOME/mixer key cannot
select or mute a removed track. The browser slot test requires uploaded loops
to have no effect when the player is absent.

| Compliance category | Status / scope |
| --- | --- |
| Target / build / ABI | PASS: `build-fm1-wsl.sh --release 1.2.1-live3`; SDK, loader, register-access and size checks retained. Target image/region sizes above. |
| Audio hot path / numerics | PASS, source/build scope: player calls, break mix and scratch buffer removed; inactive break motion cannot automate the mix. No new math, allocation, storage I/O, logging or blocking. No soft-double routines linked; RAM flash code remains call-free. |
| Execution ownership / synchronization | PASS, source scope: existing renderer, sequencer and optional 808 worker/fallback retained; no new task, ISR, queue or synchronization. Director publication unchanged. |
| Memory / storage / I/O | PASS for static target budget and host round-trip: region limits/reserve pass, legacy break pattern fields retained and silent. Flash layout and storage versions unchanged. Hardware peak stack remains UNVERIFIED. |
| Dependencies / includes / licenses | PASS: no dependencies added; removed runtime references and bank inclusion, existing GPL notices/source retained. |
| Compatibility / package / recovery | PASS for host/package scope: legacy data round-trip and updater checks; no erase or repartition. Device installation and recovery UNVERIFIED. |
| Host functional validation | PASS for all 40 supported application scenarios (full suite plus focused rerun of the corrected MIX MIDI-channel menu expectation), generator/Director/DSP golden tests, storage/migration/backup/update suites and native/WASM audio comparison. Eight legacy breakbeat performance scenarios are retired: brkfilter, slices, upload, getting_started, groove, jam, showcase, tour. |
| Hardware real-time validation | UNVERIFIED: no device flash, timing, stack high-water or analog-output test. Test four-part dense playback/FX with Director commits, rapid controls and MIDI/USB; measure render maxima, underruns and stack usage, then soak. |

Three external engine comparisons cannot run without 9W9, 8W8 and schwung-303
checkouts. The optional schwung-tb3po reference comparison and fm1-reader browser
voice remain unavailable. These omissions are separate from the retired BREAK
performance scenarios. Browser visual automation is unavailable; host/WASM tests
verify application behavior without claiming device qualification.

The full runner still reports missing external reference checkouts. The last
remaining application failure was a menu-navigation expectation in `midiccout`;
after correcting the offset it passes, including channel selection, snapshot,
persistence, echo suppression and lane CC output. The no-break compatibility
scenario measures -180 dBFS (the host test's silence floor) from a populated
legacy break pattern. Browser uploads produce identical audio with/without a
user loop. Native/WASM comparison: 168021 samples, peak 0.261, maximum difference
3.05e-5; screen/lights match. The host mixer screenshot was visually inspected:
only the four active tracks are shown. Target ELF inspection finds no break
player state, scratch buffer, runtime entry points or built-in loop symbols.

Package SHA-256:
`045c3a2772b5ec37ed344fac6b2b1182b9d557617f0921983e90fd3143e988d9`.
