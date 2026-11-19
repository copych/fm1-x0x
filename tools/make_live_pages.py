#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Generate the live fork's public site from a release package and build/emu.

Usage: python3 tools/make_live_pages.py 1.2.1-live5
Output: build/live-site (publish its contents to the gh-pages branch).
"""
import hashlib
import html
import json
from pathlib import Path
import shutil
import subprocess
import sys

from make_pages import main as make_base_pages

REPO = Path(__file__).resolve().parents[1]


def main(version):
    package = REPO / 'build' / f'x0x-{version}.fwsc'
    site = REPO / 'build' / 'live-site'
    if site.is_symlink():
        raise SystemExit('Refusing a symlink for site output')
    make_base_pages(package, version, site)
    revision = subprocess.check_output(['git', '-C', str(REPO), 'rev-parse', 'HEAD'], text=True).strip()
    manifest = dict(version=version, source=revision,
                    sha256=hashlib.sha256(package.read_bytes()).hexdigest(),
                    bytes=package.stat().st_size, package=f'firmware/{package.name}',
                    hardware_validation='UNVERIFIED')
    (site / 'build-info.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')

    def render(source, destination):
        subprocess.run([sys.executable, str(REPO / 'tools' / 'manual_html.py'),
                        str(source), str(destination)], check=True)

    render(REPO / 'README.md', site / 'index.html')
    shutil.copytree(REPO / 'docs' / 'img' / 'performer', site / 'docs' / 'img' / 'performer')
    landing = site / 'index.html'
    content = landing.read_text(encoding='utf-8').replace('class="home" href="../"', 'class="home" href="./"')
    for name in ('LICENSE', 'BUILDING.md'):
        content = content.replace(f'href="{name}"',
                                  f'href="https://github.com/copych/fm1-x0x/blob/codex/motif-development/{name}"')
    content=content.replace('https://copych.github.io/fm1-x0x/','./')
    landing.write_text(content, encoding='utf-8')
    guide = site / 'performance'
    guide.mkdir()
    # Keep engineering validation in the repository; the web guide is for playing.
    guide_source = REPO / 'build' / 'performer-user.md'
    guide_source.write_text((REPO / 'docs' / 'PERFORMER.md').read_text(encoding='utf-8')
                            .split('## Validation and limits')[0], encoding='utf-8')
    render(guide_source, guide / 'index.html')
    shutil.copytree(REPO / 'docs' / 'img' / 'performer', guide / 'img' / 'performer')
    notice = ('<aside style="border:1px solid currentColor;padding:16px;margin:20px 0">'
              f'<strong>X0X Live {html.escape(version)} — experimental</strong>'
              '<p>Two 303s, 909 and 808, with the LIVE performer interface. '
              'Hardware validation is pending. Back up before switching firmware; '
              'this site\'s backup tool handles X0X projects only.</p>'
              '<p>Using Sloop? Back up with the '
              '<a href="https://isod89.github.io/sloop-fm1/webapp/editor/">Sloop editor</a>, '
              'then close it. Direct switching from Sloop is not yet verified.</p>'
              '<p><a href="../performance/">Performer controls</a> · '
              '<a href="../build-info.json">Build identity and checksum</a></p></aside>')
    installer = site / 'install' / 'index.html'
    installer.write_text(installer.read_text(encoding='utf-8').replace('<main>', '<main>' + notice, 1), encoding='utf-8')
    for path in (installer, site / 'emu' / 'index.html'):
        path.write_text(path.read_text(encoding='utf-8')
                        .replace('https://github.com/charlesvestal/fm1-x0x', 'https://github.com/copych/fm1-x0x')
                        .replace('href="../manual/">Manual', 'href="../performance/">Controls'), encoding='utf-8')
    (site / 'breaks' / 'index.html').write_text(
        '<!doctype html><html lang="en"><meta charset="utf-8"><title>Breakbeat disabled</title>'
        '<h1>Breakbeat playback is disabled in X0X Live</h1>'
        '<p>Existing sample storage is retained. Loop uploads are unavailable here.</p>'
        '<p><a href="../">X0X Live</a></p></html>', encoding='utf-8')
    print('Live site ready:', site)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    main(sys.argv[1])
