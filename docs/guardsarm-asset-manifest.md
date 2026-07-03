# GuardSarm — Brand Asset Manifest

`branding.json` covers only **text**. Graphical brand assets (icons, logos,
installer bitmaps) are binary and cannot be substituted by the branding layer;
they must be produced as real files and dropped into the paths below.

**Status:** Real GuardSarm artwork supplied in `brand-assets/` (source SVGs + PNG).
Applied so far (filenames preserved → no code change):
- **SVG** logos/marks — dashboard render assets + README logos (all repos).
- **Windows `.ico`** — `src/win32/{favicon,install,uninstall}.ico`,
  `src/win32/ui/favicon.ico` regenerated from `guardsarm-icon.png` via
  `brand-assets/generate-icons-pillow.py` (Pillow), plus all dashboard favicons.

**Still pending:**
- **macOS `.icns`** — a reference `brand-assets/guardsarm.icns` was generated;
  place it at the macOS build's icon path (no canonical path in this repo yet).
- **MSI dialog `.bmp`** (WiX banner 493×58 / dialog 493×312) — needs the
  horizontal logo rasterized; run `brand-assets/generate-icons.sh` on a machine
  with ImageMagick (the SVG logo can't be rasterized by Pillow).

For sharper, SVG-sourced icons, regenerate with ImageMagick 7 (`magick`) via
`generate-icons.sh`. Note: the Windows `convert` command is the FAT→NTFS disk
tool, NOT ImageMagick.

> Note: The most user-visible artwork (dashboard favicon, login logo, app icons)
> lives in the **wazuh-dashboard** and **wazuh-indexer** repositories, which are
> out of scope for this core repo. See `docs/guardsarm-remaining-work.md`.

## Required assets (this repository)

| Asset | Path (source tree) | Format | Dimensions | Notes |
|-------|--------------------|--------|------------|-------|
| Windows agent EXE icon | `src/win32/*.ico` (referenced by the win32 build/WXS) | ICO (multi-res) | 16, 32, 48, 256 px | Shown in Explorer, Task Manager, Add/Remove Programs. |
| MSI dialog banner | WiX `WixUIBannerBmp` (installer UI) | BMP | 493 × 58 px | Top banner on installer wizard pages. |
| MSI dialog background | WiX `WixUIDialogBmp` | BMP | 493 × 312 px | Welcome/exit dialog background. |
| macOS installer background | `packages/macos/` installer resources | PNG | 620 × 418 px (@1x/@2x) | macOS pkg installer window. |
| macOS app icon (if any UI) | `packages/macos/` | ICNS | up to 1024 px | Only if a macOS UI bundle exists. |

## Recommended source assets to keep on hand

| Asset | Format | Dimensions | Used for |
|-------|--------|------------|----------|
| Primary logo (horizontal) | SVG + PNG | vector; 512 px PNG | Docs, README, reports. |
| Logomark / app icon master | SVG | vector | Source for ICO/ICNS export. |
| Monochrome logo | SVG | vector | Dark/light UI, watermarks. |

## Explicitly NOT changed by asset swaps

Certificate subjects, WPK/package **signing identities**, and code-signing /
notarization certificates are security material, not artwork. Re-signing under a
GuardSarm identity is an infrastructure/security decision tracked in
`docs/guardsarm-remaining-work.md`, not part of this manifest.
