# Musical Director embryo

The Director coordinates musical relationships. The 303 and drum generators still
produce the material; the Director does not own synthesis, effects, mixing or storage.

In the ARP / GROOVE view, hold **ENV + LFO** and tap **OCT+** to request an ensemble.
Hold **ENV + LFO** and tap **OCT-** to cancel a pending request.

The first relationship is deliberately small:

- 303 A remains the lead and is preserved exactly.
- 303 B answers the lead two sixteenth notes later, normally an octave higher,
  leaving space on overlapping lead attacks. It inherits A's tonal vocabulary,
  with a separate generator seed.
- The selected drum kit supplies kick, snare and closed hat using its current
  Straight–Weird / Hollow–Full settings. Hats leave space around the acid parts.
  Other percussion rows, hidden steps and drum accents remain as edited.

From a 303 view the supporting kit is 909; from a drum view it is that kit.
Both 303s and the supporting kit must use 16 steps at sixteenth-note rate,
the 303s must run forward with automatic mutation off, and the 909 bar clock
must also be 16 sixteenths. Song playback and pattern chains are unsupported.
An empty A is refused. These restrictions appear as DIRECTOR NOT READY.

While stopped the prepared plan applies on the next render block. During playback
it waits for the next four-bar boundary on X0X's existing 909 bar clock. The change
lands before the first drum hit and the other parts' coincident notes. Existing
polymetric, reverse and random patterns remain available outside this action.

DIRECTOR QUEUED, ENSEMBLE READY and DIRECTOR CANCELLED report the outcome. The
queue has one slot: additional composition/capture requests are refused while it
is occupied; direct synthesis and mixing remain available. Editing/regenerating
source material or switching active patterns cancels a stale plan at its boundary.
Stopping applies an outstanding valid plan; explicit cancellation discards it.
Project initialization clears the queue. LIKE captures remain independent and
RETURN can recover B's previous captured phrase after an ensemble change.

This is an explicit coordination action, not yet an autonomous conductor. Section
behaviors, continuous development, queued BUILD/BREAK/DROP and favorite selection
are future work. Repeating the action with the same A and groove settings gives
the same musical relationship.

## Implementation and ownership

`seq/director.h` holds the symbolic context, roles, compact plan and coordination
rules. `tb3po_response()` in the 303 generator voices B's response. The existing
drum generator supplies its seeded groove; the Director removes competing hats.

Main-loop UI handling prepares a complete plan before publishing an aligned
32-bit pending state with the project's compiler memory barriers. The single
core's render context only validates and applies it. It never generates phrases,
allocates, logs, accesses storage or waits. The plan shares a union with the
existing drum-generation staging; mailbox guards prevent concurrent use. Fixed
32-bit FNV fingerprints of A, B and the selected drum part reject stale plans
without retaining three full source copies. This is change detection, with the
usual possibility of a hash collision, not a cryptographic guarantee.

The audio boundary checks at most 388 source bytes, copies one 144-byte bass part
and publishes three drum words. Actual timing must still be measured on FM-1.
No project/storage format, synthesis parameters, mutes, solo, FX, clocks,
bootloader or flash layout changes are introduced.

## Validation record

Base: X0X v1.2.1 (`502b2c2`), custom features through `7879b1d`. M-VAVE FM-1,
existing single-core pi32v2 r3 build, JieLi clang 4.0.1, AC79 SDK checkout
`d179b448`. Release package: `1.2.1-live2` / identity `FM-1_9010201`.
The suffix is shown on the installer; numeric device identity does not encode it.

The target uses 44.1 kHz stereo, a 512-frame I2S half-buffer (about 11.61 ms),
rendered in 256-frame calls. Callback timing and hardware headroom are unmeasured.

| Compliance category | Status and evidence |
| --- | --- |
| Target / build / ABI | PASS: existing release build and SDK/loader/manifest checks retained; image 496536 bytes. No compiler/platform flags changed. |
| Audio hot path / numerics | PASS, structural scope: bounded integer fingerprints, copies and three row writes only. Target disassembly has fixed 144/144/100-byte hash loops and a 128-byte step-copy loop; no new float/double math, allocation, blocking or library calls in `director_apply`. Build's soft-double and RAM-text checks pass. Hardware cost remains unmeasured. |
| Execution ownership / synchronization | PASS, source and host scope: main prepares; render consumes complete plan; one pending slot; cancel/reset and source-change rejection. Existing single-core mailbox convention retained; no new thread or ISR. |
| Memory / storage / I/O | PASS for static target budget: data+bss 98068/98304, pool 335852/344064; 8212 bytes remain, preserving the 8192-byte reserve. Added persistent state is 80 bytes through shared staging. Formats, flash and peripherals unchanged. Peak stack remains UNVERIFIED on hardware. |
| Dependencies / includes / licenses | PASS: existing generator modules and integer types only; GPL-3.0-only notices retained. No library, task, timer or peripheral dependency added. |
| Compatibility / package / recovery | PASS for source/package tests: captures and project layout retained; normal updater and loader used. Device installation and rollback remain UNVERIFIED. |
| Host functional validation | PASS for all available suites and all 47 application scenarios. Director unit cases cover response/vocabulary, determinism, drum space, edited-source rejection, silence, bounds and unsupported input. The 20.483-second application scenario exercises stopped/queued commits, phrase timing, capture preservation, cancellation, manual drum mute and cutoff preservation. Browser/host comparison: 168021 samples, peak 0.261, maximum difference 3.05e-5; screen/lights match. |
| Hardware real-time validation | UNVERIFIED: no flash or device test. Measure render maxima/underruns during a four-bar commit under dense two-303/drum/FX and MIDI/USB load; test mute/sound priority, stop/restart, repeated requests and a sustained playback soak. |

Commands use the workspace helpers: `tools/build-fm1-wsl.sh --release 1.2.1-live2`
and `tools/test-fm1-wsl.sh`. They keep the existing integrity checks enabled.

The full runner exits 1 because three external engine comparisons cannot run:
9W9, 8W8 and schwung-303 source checkouts are absent. The optional schwung-tb3po
comparison is also skipped. These are pre-existing environment omissions, not
passing comparisons. Unrelated legacy signedness warnings remain in the host
build. No hardware install, real-time timing, stack high-water or recovery test
has been performed.

Package SHA-256:
`4871a51d8912191e7c570214b919afbb5f6482760c4108aa134985a712a7781b`.
