# Skyline Silhouette (Pebble)

A procedural city watchface. Every midnight it shuffles PixelLab building sprites into a brand-new skyline — unique for the day, then locked until tomorrow.

## How it works

- **12 building sprites** (PixelLab 1-direction objects) are packed as resources.
- At midnight (or on first launch), an LCG seeded by the calendar date picks and places buildings left-to-right until the screen is filled.
- The day's building IDs are persisted so the same city stays until the next morning.
- Dawn / day / dusk / night sky gradients (auto by hour, or locked in Settings).

## Settings (Clay)

Open **Skyline Silhouette → Settings** in the Pebble/Rebble app:

- Hour format / leading zero / date
- Building density (sparse / normal / dense)
- City variation seed (0–99) for a different skyline today
- Sky mode (auto / dawn / day / dusk / night)
- Sun & moon / night stars
- Invert for black & white displays

## Build

On Windows (Podman or Docker):

```powershell
./build.ps1
```

Output: `build/SkylineSilhouette.pbw`

```powershell
./build.ps1 -Phone 192.168.1.23
./build.ps1 -Zip
```

## Assets

Raw PixelLab downloads live under `docs/raw/`. Rebuild watch resources with:

```powershell
python tools/prepare_assets.py
```

## Layout

```
src/c/                 watchface + procedural skyline
src/pkjs/              Clay settings
resources/images/      buildings, sun/moon, menu icon
```
