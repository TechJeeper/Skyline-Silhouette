#pragma once

#include <pebble.h>
#include "settings.h"

#define BUILDING_SPRITE_COUNT 12
#define SKYLINE_MAX_SLOTS 20
#define STAR_COUNT 18

typedef enum {
  PERIOD_DAWN = 0,
  PERIOD_DAY,
  PERIOD_DUSK,
  PERIOD_NIGHT
} SkyPeriod;

typedef struct {
  GColor sky_top;
  GColor sky_bot;
  GColor ground;
  GColor text;
  bool night;
} SkyPalette;

void skyline_init(void);
void skyline_deinit(void);
void skyline_ensure_today(int year, int yday, int width);
void skyline_rebuild(int year, int yday, int width);
SkyPeriod skyline_period_for_hour(int hour, SkyMode mode);
SkyPalette skyline_palette(SkyPeriod period, bool invert_bw);
void skyline_draw(GContext *ctx, GRect bounds, int hour, int minute);
