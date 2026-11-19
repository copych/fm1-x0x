# X0X Live for the M-VAVE FM-1

Turn your FM-1 into an instrument for live generative acid performance. Discover a bassline, develop it while it plays, bring in drums, and shape the sound with your hands. Two independent 303s, 909 and 808 drum machines, and a single LIVE screen keep musical changes, sound controls and mixing close together.

**Current version: 1.2.1-live6 — experimental.** This fork builds on X0X 1.2.1. The new performance workflow has not yet been validated on an actual FM-1.

- **[Try the browser emulator](https://copych.github.io/fm1-x0x/emu/)** — no hardware needed.
- **[Install on your FM-1](https://copych.github.io/fm1-x0x/install/)** — Chrome or Edge, over USB.
- **[Performer controls](https://copych.github.io/fm1-x0x/performance/)** — complete gestures and screenshots.
- **[Download live6 firmware](https://copych.github.io/fm1-x0x/firmware/x0x-1.2.1-live6.fwsc)**.

![LIVE performer screen](docs/img/performer/live.png)

## What you can play

- **Two acid voices:** independent 303 synthesis, cutoff, resonance, envelope, decay, accent, waveform, drive, pan and effect sends.
- **Generate and develop:** find a new phrase, explore related variations, LIKE an idea and RETURN to it. Musical undo restores notes and hits while keeping your current sound and mute choices.
- **A Musical Director prototype:** keep 303 A as the lead and ask for a related response from B and a coordinated drum groove at the next segment boundary (four bars by default).
- **909 and 808 drums:** generate all eleven instrument parts. The starting mix brings in kicks and hats; unmute other instruments when you want them.
- **Predictable drum controls:** move from Straight to Weird and Hollow to Full. Straight grooves range from a single kick with a generated crash on the first beat to four-on-the-floor kicks and sixteenth-note hats.
- **Hands-on performance:** select several parts, turn linked controls, mute or solo tracks and individual drums, and restore a previous mix.
- **Effects:** independent 303 drive and sends, shared delay and reverb, master filtering and kick pump. Hold keys for filtering, pump, drive, tape stop, chopper or captured stereo repeats.

The display shows your selected parts, current controls, four-bar position and pending Director action. Detailed step editing remains available when needed. Breakbeat playback is disabled in this build to make room for the live features.

## Install

1. **Back up your current firmware's data first.** If you use Sloop, use the [Sloop editor](https://isod89.github.io/sloop-fm1/webapp/editor/). The [X0X backup page](https://copych.github.io/fm1-x0x/backup/) only handles X0X projects. Keep the firmware and restore instructions for your current setup.
2. Connect the FM-1 with a USB data cable. Close other MIDI applications and editor tabs.
3. Open the **[web installer](https://copych.github.io/fm1-x0x/install/)** in desktop Chrome or Edge. Allow MIDI / SysEx access when asked.
4. Follow the device connection and installation steps. Keep USB connected until the update finishes and the instrument restarts.
5. The firmware opens on the **LIVE** performer screen.

The installer checks device and package identity. If it cannot find your device, check its connection rather than bypassing those checks. Switching directly from every alternative firmware, including Sloop, is not yet verified. For recovery, see the [base manual](https://copych.github.io/fm1-x0x/manual/#recovering-an-fm-1-that-won-t-start).

## Your first groove

1. On the initial **303 A** selection, press **OCT+** to make a phrase, then **PLAY**.
2. Tap **ARP** to select Director, then tap **ARP again** for a complementary B phrase and drums. During playback it applies at the next segment boundary; tap ARP again to cancel.
3. Turn the four knobs for **cutoff, resonance, envelope modulation and decay**.
4. Use the first four **black keys** to select **909 / 808 / 303 A / 303 B**. Hold **SEL** while selecting to link several parts.
5. Select a drum kit, then tap **ARP**: knobs 1 and 2 control **Weird** and **Full**. In the instrument section, white keys select drum instruments.

| Gesture in LIVE | Action |
| --- | --- |
| OCT+ / OCT- | New material / develop or vary selected material |
| ENV + OCT+ / LFO + OCT- | LIKE / RETURN |
| REC tap / REC + OCT+ | Musical undo / redo |
| HOME + black key | Mute or unmute a part |
| HOME + white key, drum kit selected | Mute or unmute a drum instrument |
| REC + black or white key | Solo a part or drum instrument |
| HOME + SEL | Restore the captured mix |
| EDIT, then knobs | Instrument controls; tap EDIT again for additional sound controls |
| LFO, then knobs / keys | Mixer: four part levels / direct part and drum mutes |
| FX, then knobs | Master filter, pump, delay feedback, reverb level |
| FX, then hold white 1–9 | Filters, pump, drive, tape stop, chopper, repeats |
| ENV | Latched automation: 3 depths of modulation, growing and falling saws |
| ARP | Director: section queues, +/-1 bar and coordinated phrases |
| SELECT | Tempo |
| SAVE | Save the project |
| HOME + EDIT | Return to LIVE from the detailed editor or settings |

Key numbers count white and black keys separately from the left. The [complete controls guide](https://copych.github.io/fm1-x0x/performance/) explains linked selection, solo, effect release and Director requirements.

## What to expect from this version

Live6 adds bar-aligned automation and queued INTRO/GROOVE/BUILD/BREAK/BRIDGE/DROP/OUTRO sections, with manual segment extension/shrinking. The LIVE display uses a 20 px minimum, 24 px headings and 36 px values. Automation joins current musical phase; a four-bar preset engaged in bar 3 resets after bar 4. Turning its sound knob takes over immediately.

Audio FX currently target the master. Repeat history is about 371 ms, so very slow divisions are capped. To fit the capture ring, the maximum delay buffer is reduced from 2 seconds to about 1.26 seconds. Per-target repeats and drum-only DJ filtering remain planned. See the performer guide for the exact controls and limits.

LIKE captures musical patterns and generator settings, not synth patches or the mix. Captures, musical undo, manual mute states, section position and preset automation are session-only. SAVE keeps the project; it does not make those temporary performance memories persistent.

## About this fork

Based on [X0X by Charles Vestal](https://github.com/charlesvestal/fm1-x0x), built on [Felucca by Leo Kuroshita / Hügelton Instruments](https://github.com/hugelton/Felucca). The 303s use Open303; the drums derive from [9W9](https://github.com/athousanddetails/schwung-9W9) and [8W8](https://github.com/athousanddetails/schwung-8W8). The 909 hats and cymbals use ER-99 samples by Matthew Cieplak (GPL-3.0). Generators derive from [schwung-tb3po](https://github.com/charlesvestal/schwung-tb3po). Existing DSP, sample and font credits and licenses are retained in the source and base manual.

X0X is GPL-3.0-only; Open303 is MIT and the fonts are SIL OFL 1.1. See [LICENSE](LICENSE) and the source notices. FM-1 and Roland instrument names are trademarks of their owners; this project is independent of those companies.

For development and build instructions, see [BUILDING.md](BUILDING.md).
