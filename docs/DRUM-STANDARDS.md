# Drum generation with recognizable standards

Live4 replaces the initial three-voice priority experiment. Both kits now
compose all eleven instrument rows. At sparse settings some rows deliberately
contain rests; at Full 100 every instrument has material ready to unmute.
The 808's tom/conga and rim/claves switches still determine which sound its row
plays. The 909's ride role maps to the 808's cowbell.

With Weird at zero, these landmarks are exact for every seed:

| Full | Kick | Hats | Cymbal |
| ---: | --- | --- | --- |
| 0 | First beat only | None | Crash/cymbal on first beat |
| 25 | Four-on-the-floor | Offbeat eighths | First beat |
| 50 | Four-on-the-floor | All eighths | First beat |
| 75 | Four-on-the-floor | All sixteenths | First beat |

On the 808, its synthesized cymbal supplies the crash role. Full 0 now means
sparse anchored music, replacing the earlier generator's silence behavior.
Use mute for silence. The standard landmarks describe pattern contents,
independently of which instruments the performer chooses to hear.

Full progressively admits backbeats, rim/clap, tom fills, open hats and metal.
Above 75, seed-dependent thresholds provide denser variations. Weird transforms
placement through deterministic swaps and larger displacements, while keeping
the first kick and crash anchored. Returning to the same seed/coordinates returns
the same groove. Straight standards deliberately do not change when reseeded.

Open and closed hats share the same placement transform. An open hit replaces
the closed hit at that step, so they do not choke one another at the same instant.
Total hat occupancy grows with Full; changing Weird preserves each instrument's
hit count. Full can replace closed hats with open hats, rather than merely adding
an overlapping voice.

## Initial mix and manual priority

The first accepted generation request for a kit in the current session/project
initializes participation: kick, closed hat and open hat are available; the other
voices are muted. Their complete patterns still exist. On the drum screen, hold
HOME and tap the instrument's black key to unmute it. In particular, unmute the
909 crash or 808 cymbal to hear the sparse first-beat cymbal.

If the performer has already toggled any instrument mute in that kit, the default
mix initialization is skipped. Later generation, Full/Weird changes and LIKE /
RETURN do not change manual mutes. Existing whole-part mutes remain untouched.
Director requests use the same one-time starting mix; this mix initialization is
immediate, while its musical plan still waits for a phrase boundary.

Default participation is session state, not a newly persisted scene or mixer.
Reboot/project restoration resets this initialization marker, as with the current
generator coordinates and LIKE memory. Stored hit patterns and project layout
remain compatible. No sample storage or sound patch is changed.

## Director integration

The Director stages all eleven generated instrument rows rather than only kick,
snare and closed hat. It still keeps A as lead, asks the 303 engine for B's response,
and removes competing closed hats around acid attacks. Other percussion no longer
retains unrelated old rows during an explicit ensemble-generation action. Hidden
steps above step 16 and manually edited drum accents remain untouched by this
Director publication, while ordinary kit generation continues to replace its full
pattern. Manual mix/sound choices and LIKE captures remain independent.

## Validation

Base `a5a289e`; unchanged FM-1 / JieLi clang 4.0.1 pi32v2 r3 / AC79 SDK and linker
layout. Build: `build-fm1-wsl.sh --release 1.2.1-live4`. Test:
`test-fm1-wsl.sh`. 44.1 kHz stereo, 512-frame I2S half-buffer, 256-frame render
calls. The generators run on main before mailbox publication; render only copies
the prepared result. Audio deadlines and peak stack use remain unmeasured.

| Compliance category | Status / evidence |
| --- | --- |
| Target / build / ABI | PASS: release, loader/SDK manifest, register-access and image checks retained. Image 375556 B; data+bss 96288/98304; pool 329600/344064. |
| Audio hot path / numerics | PASS for source/build scope: generation remains on main; bounded Director publication grows from 3 to 11 row writes. No allocation, floating-point math, blocking, logging or storage added to audio. No soft-double routines; RAM flash code remains call-free. |
| Execution ownership / synchronization | PASS for source scope: existing main-to-render one-slot mailbox and barriers retained. Main applies initial mix and records manual mute intent; reset occurs in the existing reset service. No new worker or ISR. |
| Memory / storage / I/O | PASS for static budget: 32 additional pool bytes; 14464 pool bytes remain, including the 8192-byte reserve. No format, sample, partition or peripheral changes. Hardware peak stack UNVERIFIED. |
| Dependencies / licenses | PASS: existing modules and integer operations only; GPL-3.0-only notices retained. |
| Compatibility / package / recovery | PASS for source/package scope; saved pattern format unchanged, mute defaults scoped to first generation and never override an explicit instrument-mute choice. Device install/recovery UNVERIFIED. |
| Host functional validation | PASS: all 40 existing supported scenarios plus the new drum-landmarks scenario (41 total, suite plus focused new-scenario run). Unit tests exercise both kits, 64 seeds, every Full value and Weird sweeps, exact standards, monotonic density, exclusive hats, deterministic replay, all rows, bounds and Director integration. Storage, migration, backup, update and native/WASM comparison pass. |
| Hardware real-time validation | UNVERIFIED: no flash, load/timing measurement or physical listening session. Test dense all-voice playback/FX, rapid Full/Weird movement, manual mutes, Director transitions and a sustained soak; measure underruns, render maxima and stack usage. |

External 9W9/8W8/schwung-303 comparison checkouts and the optional TB3PO reference
remain unavailable. Eight obsolete breakbeat performance scenarios remain retired.
The single performance mode is specified in PERFORMANCE-MODE.md and is not yet
implemented by this build.

The full test runner exits 1 for the three unavailable external engine checkouts;
all available tests pass. The new panel scenario also verifies manual participation
before and after first generation, all-voice generation in both kits, exclusive
hats, and five seconds of dense two-kit playback (host peak -7.9 dBFS). Browser
comparison: 168021 samples, peak 0.261, maximum host difference 3.05e-5;
screen/lights match. These are host observations, not hardware timing evidence.

Package SHA-256:
`2bd725aef86e08fe044631c00244708f37ed5a355da2c1acf355ba6964df6751`.
