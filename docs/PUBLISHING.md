# Publishing the live site

The public site is https://copych.github.io/fm1-x0x/. GitHub Pages serves the
root of the `gh-pages` branch. The source and README are on `main` and
`codex/motif-development`.

After building a release package and matching emulator, generate the site:

```sh
./build.sh --release 1.2.1-live5
X0X_VERSION=1.2.1-live5 sh web/emu/build.sh
python3 tools/make_live_pages.py 1.2.1-live5
```

Publish the contents of `build/live-site` to `gh-pages` using a separate checkout
of that branch, preserving its history. Do not publish the entire build folder.
GitHub Pages must be configured to deploy from that branch's root directory.

The generator retains the base installer's model/package checks, adds the live
build identity and backup notice, supplies the current performer guide, disables
the break uploader, and copies the actual firmware and emulator assets. Its
landing page comes from README.md. Update the README's version/download link
when publishing a new version. `build-info.json` records the firmware source
revision, package size and SHA-256; publishing a site does not qualify the
firmware for hardware use or flash any device.
