# Rebble Appstore listing — Skyline Silhouette

Copy these fields into [dev-portal.rebble.io](https://dev-portal.rebble.io/submit).

| Field | Value |
| --- | --- |
| App name | Skyline Silhouette |
| Type | Watchface |
| Category | Faces *(automatic)* |
| Website | *(optional)* |
| Source code | https://github.com/TechJeeper/Skyline-Silhouette |
| `.pbw` | `build/SkylineSilhouette.pbw` |

## Description

Paste into each platform asset collection (max 1600 characters):

```
A new city every morning.

Skyline Silhouette pieces together PixelLab building sprites into a unique skyline at midnight — then holds that city until tomorrow.

• Procedural skyline regenerates every day at midnight
• 12 building sprites shuffled into a fresh silhouette
• Dawn · day · dusk · night skies (auto by hour, or lock one)
• Optional sun, moon, and night stars
• Density and city-variation controls in phone settings
• Clay configuration for time, sky, and look

Color watches get gradient skies; black & white watches keep the silhouette and optional invert.
```

*(~520 characters)*

## Release notes (v1.0.0)

```
Initial release: midnight procedural skyline, PixelLab buildings, Clay settings for density, sky mode, and celestial accents.
```

## Screenshots

Exact platform sizes (no watch bezel). Regenerate with:

```powershell
python tools/generate_store_screenshots.py
```

| Platform | Size | Folder |
| --- | --- | --- |
| basalt (Pebble Time) | 144×168 | `store/screenshots/basalt/` |
| diorite (Pebble 2) | 144×168 | `store/screenshots/diorite/` |
| flint (Pebble 2 Duo) | 144×168 | `store/screenshots/flint/` |
| emery (Pebble Time 2) | 200×228 | `store/screenshots/emery/` |

Five shots per platform:

1. Dawn skyline  
2. Day (+date, dense)  
3. Dusk  
4. Night (+date, stars)  
5. Sparse night

## Listing checklist

- [ ] Screenshots uploaded per platform (up to 5)
- [ ] Description above on each asset collection
- [ ] Source URL set to the GitHub repo
- [ ] Publish `build/SkylineSilhouette.pbw` as release 1.0.0
