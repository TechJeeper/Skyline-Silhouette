#!/usr/bin/env python3
"""Render Rebble App Store screenshots at exact platform resolutions.

No watch bezel — store requirement. Showcases sky periods + procedural cities.
"""
from __future__ import annotations

import os
import random
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "store" / "screenshots"
BLDG = ROOT / "resources" / "images" / "buildings"
SKY = ROOT / "resources" / "images" / "sky"

PLATFORMS = {
    "basalt": (144, 168, True),
    "diorite": (144, 168, False),
    "flint": (144, 168, False),
    "emery": (200, 228, True),
}

# Approximate Pebble 64-color values from skyline.c
PALETTES = {
    "dawn": {
        "top": (42, 24, 72),
        "bot": (255, 170, 85),
        "text": (255, 255, 255),
        "night": False,
    },
    "day": {
        "top": (85, 170, 255),
        "bot": (170, 255, 255),
        "text": (0, 0, 0),
        "night": False,
    },
    "dusk": {
        "top": (0, 0, 85),
        "bot": (255, 85, 0),
        "text": (255, 255, 255),
        "night": False,
    },
    "night": {
        "top": (0, 0, 0),
        "bot": (0, 0, 85),
        "text": (255, 255, 255),
        "night": True,
    },
}

SCENES = [
    {
        "file": "01-dawn.png",
        "period": "dawn",
        "time": "06:18",
        "date": None,
        "seed": 11,
        "density": "normal",
        "celestial": "sun",
    },
    {
        "file": "02-day.png",
        "period": "day",
        "time": "14:42",
        "date": "Mon 2 Nov",
        "seed": 27,
        "density": "dense",
        "celestial": "sun",
    },
    {
        "file": "03-dusk.png",
        "period": "dusk",
        "time": "18:05",
        "date": None,
        "seed": 44,
        "density": "normal",
        "celestial": "sun",
    },
    {
        "file": "04-night.png",
        "period": "night",
        "time": "23:17",
        "date": "Sat 31 Oct",
        "seed": 63,
        "density": "dense",
        "celestial": "moon",
        "stars": True,
    },
    {
        "file": "05-sparse-night.png",
        "period": "night",
        "time": "02:08",
        "date": None,
        "seed": 88,
        "density": "sparse",
        "celestial": "moon",
        "stars": True,
    },
]

OVERLAP = {
    "sparse": (2, 8),
    "normal": (-2, 4),
    "dense": (-6, 0),
}


def font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    if bold:
        candidates = [
            "C:/Windows/Fonts/consolab.ttf",
            "C:/Windows/Fonts/lucon.ttf",
            "C:/Windows/Fonts/segoeuib.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        ]
    else:
        candidates = [
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        ]
    for path in candidates:
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def leco_font(size: int) -> ImageFont.ImageFont:
    candidates = [
        "C:/Windows/Fonts/consolab.ttf",
        "C:/Windows/Fonts/lucon.ttf",
        "C:/Windows/Fonts/courbd.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
    ]
    for path in candidates:
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return font(size, bold=True)


def draw_text_center(draw: ImageDraw.ImageDraw, text: str, y: int, w: int, fnt, fill) -> None:
    bbox = draw.textbbox((0, 0), text, font=fnt)
    tw = bbox[2] - bbox[0]
    draw.text(((w - tw) // 2, y), text, font=fnt, fill=fill)


def load_buildings() -> list[Image.Image]:
    out = []
    for i in range(12):
        path = BLDG / f"bldg_{i:02d}.png"
        out.append(Image.open(path).convert("RGBA"))
    return out


def to_bw(src: Image.Image) -> Image.Image:
    rgba = src.convert("RGBA")
    out = Image.new("RGBA", rgba.size, (0, 0, 0, 0))
    px = rgba.load()
    opx = out.load()
    w, h = rgba.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                continue
            lum = 0.299 * r + 0.587 * g + 0.114 * b
            if lum >= 180:
                opx[x, y] = (255, 255, 255, 255)
            else:
                opx[x, y] = (0, 0, 0, 255)
    return out


def sky_gradient(w: int, h: int, top: tuple, bot: tuple, color: bool) -> Image.Image:
    if not color:
        return Image.new("RGB", (w, h), (0, 0, 0))
    img = Image.new("RGB", (w, h))
    draw = ImageDraw.Draw(img)
    bands = 8
    for i in range(bands):
        y0 = (h * i) // bands
        y1 = (h * (i + 1)) // bands
        c = top if i < bands // 2 else bot
        if i in (bands // 2 - 1, bands // 2):
            c = top if i % 2 == 0 else bot
        draw.rectangle((0, y0, w, y1), fill=c)
    return img


def layout_city(w: int, buildings: list[Image.Image], seed: int, density: str) -> list[tuple]:
    rng = random.Random(seed)
    lo, hi = OVERLAP[density]
    slots = []
    x = rng.randint(-8, 3)
    while x < w + 8 and len(slots) < 20:
        b = buildings[rng.randrange(len(buildings))]
        slots.append((x, b))
        x += b.size[0] + rng.randint(lo, hi)
    return slots


def render(w: int, h: int, color: bool, scene: dict, buildings: list[Image.Image]) -> Image.Image:
    pal = PALETTES[scene["period"]]
    if color:
        top, bot, text = pal["top"], pal["bot"], pal["text"]
    else:
        top = bot = (0, 0, 0)
        text = (255, 255, 255)

    img = sky_gradient(w, h, top, bot, color)
    draw = ImageDraw.Draw(img)

    if scene.get("stars") and (pal["night"] or not color):
        rng = random.Random(scene["seed"] ^ 0x51A5)
        for _ in range(18):
            sx = rng.randint(2, w - 3)
            sy = rng.randint(2, h // 2)
            draw.point((sx, sy), fill=text)
            if rng.randrange(4) == 0:
                draw.line((sx - 1, sy, sx + 1, sy), fill=text)
                draw.line((sx, sy - 1, sx, sy + 1), fill=text)

    if scene.get("celestial"):
        cel = Image.open(SKY / f"{scene['celestial']}.png").convert("RGBA")
        if not color:
            cel = to_bw(cel)
        cx = w - cel.size[0] - 10
        cy = 10 if scene["celestial"] == "sun" else 14
        img.paste(cel, (cx, cy), cel)

    bldgs = buildings if color else [to_bw(b) for b in buildings]
    for x, b in layout_city(w, bldgs, scene["seed"], scene["density"]):
        y = h - b.size[1]
        img.paste(b, (x, y), b)

    time_size = 36 if w >= 200 else 28
    draw_text_center(draw, scene["time"], 8 if h > 180 else 6, w, leco_font(time_size), text)
    if scene.get("date"):
        draw_text_center(draw, scene["date"], 50 if w >= 200 else 42, w, font(max(11, w // 13)), text)

    return img


def main() -> None:
    buildings = load_buildings()
    if OUT.exists():
        shutil.rmtree(OUT)
    for plat, (w, h, color) in PLATFORMS.items():
        dest = OUT / plat
        dest.mkdir(parents=True, exist_ok=True)
        for scene in SCENES:
            shot = render(w, h, color, scene, buildings)
            path = dest / scene["file"]
            shot.save(path, "PNG", optimize=True)
            print(f"wrote {path.relative_to(ROOT)} ({w}x{h})")
    print("Done:", OUT)


if __name__ == "__main__":
    main()
