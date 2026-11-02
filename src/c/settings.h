#pragma once

#include <pebble.h>
#include <stdbool.h>

typedef enum {
  TIME_FMT_SYSTEM = 0,
  TIME_FMT_12H,
  TIME_FMT_24H
} TimeFormat;

typedef enum {
  SKY_AUTO = 0,
  SKY_DAWN,
  SKY_DAY,
  SKY_DUSK,
  SKY_NIGHT
} SkyMode;

typedef enum {
  DENSITY_SPARSE = 0,
  DENSITY_NORMAL,
  DENSITY_DENSE
} DensityMode;

typedef struct {
  TimeFormat time_format;
  bool leading_zero;
  bool show_date;
  bool show_celestial;
  bool show_stars;
  SkyMode sky_mode;
  DensityMode density;
  int city_seed;  // 0..99 mixed into daily seed
  bool invert_bw;
} FaceSettings;

typedef void (*SettingsChangedFn)(void);

void settings_init(void);
void settings_deinit(void);
void settings_set_changed_callback(SettingsChangedFn cb);
const FaceSettings *settings_get(void);
