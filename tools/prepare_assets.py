"""Crop/key PixelLab building sprites and pack the menu icon for Pebble."""
from __future__ import annotations

import zipfile
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
RAW = ROOT / "docs" / "raw"
BLDG_RAW = RAW / "buildings"
OUT_BLDG = ROOT / "resources" / "images" / "buildings"
OUT_SKY = ROOT / "resources" / "images" / "sky"
OUT_IMG = ROOT / "resources" / "images"

# Near-black becomes transparent so silhouettes layer over the sky gradient.
BLACK_KEY = 28


def ensure_png(path: Path) -> Path:
    """Object download endpoint returns a PNG even when named .zip."""
    data = path.read_bytes()
    if data[:4] == b"\x89PNG":
        png = path.with_suffix(".png")
        if png != path:
            png.write_bytes(data)
        return png
    if zipfile.is_zipfile(path):
        dest = path.with_suffix("")
        dest.mkdir(exist_ok=True)
        with zipfile.ZipFile(path) as z:
            z.extractall(dest)
        pngs = list(dest.rglob("*.png"))
        if not pngs:
            raise SystemExit(f"no png in {path}")
        return pngs[0]
    raise SystemExit(f"unknown asset format: {path}")


def key_and_crop(im: Image.Image, threshold: int = BLACK_KEY) -> Image.Image:
    im = im.convert("RGBA")
    px = im.load()
    w, h = im.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a == 0:
                continue
            if r <= threshold and g <= threshold and b <= threshold:
                px[x, y] = (0, 0, 0, 0)

    # Crop to opaque content, keep bottom-aligned feel by trimming sides/top only.
    bbox = im.getbbox()
    if not bbox:
        return im
    left, top, right, bottom = bbox
    # Keep a 1px pad; force bottom flush with original bottom if content touches it.
    left = max(0, left - 1)
    top = max(0, top - 1)
    right = min(w, right + 1)
    if bottom >= h - 1:
        bottom = h
    else:
        bottom = min(h, bottom + 1)
    return im.crop((left, top, right, bottom))


def force_silhouette(im: Image.Image) -> Image.Image:
    """Opaque non-black pixels → black body; bright windows stay pale."""
    im = im.convert("RGBA")
    px = im.load()
    w, h = im.size
    out = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    opx = out.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 16:
                continue
            lum = (r + g + b) // 3
            if lum >= 180:
                opx[x, y] = (255, 255, 220, 255)  # lit window
            else:
                opx[x, y] = (0, 0, 0, 255)
    return out


def pad_menu(src: Path, dest: Path, size: int = 25) -> None:
    im = Image.open(src).convert("RGBA")
    im = im.resize((size, size), Image.Resampling.NEAREST)
    dest.parent.mkdir(parents=True, exist_ok=True)
    im.save(dest)
    print(f"menu {dest.name} {size}x{size}")


def main() -> None:
    OUT_BLDG.mkdir(parents=True, exist_ok=True)
    OUT_SKY.mkdir(parents=True, exist_ok=True)

    for i in range(12):
        zpath = BLDG_RAW / f"bldg_{i:02d}.zip"
        png_direct = BLDG_RAW / f"bldg_{i:02d}.png"
        if zpath.exists():
            src = ensure_png(zpath)
        elif png_direct.exists():
            src = png_direct
        else:
            raise SystemExit(f"missing building {i}")
        im = Image.open(src)
        im = key_and_crop(im)
        im = force_silhouette(im)
        # Re-crop after silhouette (transparent bg).
        bbox = im.getbbox()
        if bbox:
            im = im.crop(bbox)
        dest = OUT_BLDG / f"bldg_{i:02d}.png"
        im.save(dest)
        print(f"building {dest.name} {im.size[0]}x{im.size[1]}")

    for name in ("moon.png", "sun.png"):
        src = RAW / name
        im = key_and_crop(Image.open(src), threshold=10)
        dest = OUT_SKY / name
        im.save(dest)
        print(f"sky {dest.name} {im.size[0]}x{im.size[1]}")

    menu_src = RAW / "menu_icon.png"
    pad_menu(menu_src, OUT_IMG / "menu_icon.png", 25)

    print("done")


if __name__ == "__main__":
    main()
