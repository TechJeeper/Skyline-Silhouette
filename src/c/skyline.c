#include "skyline.h"

enum {
  PERSIST_SKY_DAY = 40,
  PERSIST_SKY_YEAR = 41,
  PERSIST_SKY_COUNT = 42,
  PERSIST_SKY_IDS = 43,
  PERSIST_SKY_SEED = 44,
};

typedef struct {
  uint8_t id;
  int16_t x;
} Slot;

static GBitmap *s_bldgs[BUILDING_SPRITE_COUNT];
static GBitmap *s_moon;
static GBitmap *s_sun;

static Slot s_slots[SKYLINE_MAX_SLOTS];
static int s_slot_count;
static int s_built_year = -1;
static int s_built_yday = -1;
static int s_built_width;
static int s_built_seed = -1;
static int s_built_density = -1;

static uint32_t s_rng;

static const uint32_t BLDG_RES[BUILDING_SPRITE_COUNT] = {
  RESOURCE_ID_IMAGE_BLDG_00,
  RESOURCE_ID_IMAGE_BLDG_01,
  RESOURCE_ID_IMAGE_BLDG_02,
  RESOURCE_ID_IMAGE_BLDG_03,
  RESOURCE_ID_IMAGE_BLDG_04,
  RESOURCE_ID_IMAGE_BLDG_05,
  RESOURCE_ID_IMAGE_BLDG_06,
  RESOURCE_ID_IMAGE_BLDG_07,
  RESOURCE_ID_IMAGE_BLDG_08,
  RESOURCE_ID_IMAGE_BLDG_09,
  RESOURCE_ID_IMAGE_BLDG_10,
  RESOURCE_ID_IMAGE_BLDG_11,
};

static void rng_seed(uint32_t seed) {
  s_rng = seed ? seed : 1u;
}

static uint32_t rng_next(void) {
  s_rng = s_rng * 1664525u + 1013904223u;
  return s_rng;
}

static int rng_range(int lo, int hi_exclusive) {
  if (hi_exclusive <= lo) return lo;
  return lo + (int)(rng_next() % (uint32_t)(hi_exclusive - lo));
}

static int building_width(int id) {
  if (id < 0 || id >= BUILDING_SPRITE_COUNT || !s_bldgs[id]) return 24;
  return gbitmap_get_bounds(s_bldgs[id]).size.w;
}

static int overlap_for_density(DensityMode d) {
  switch (d) {
    case DENSITY_SPARSE: return rng_range(2, 8);
    case DENSITY_DENSE:  return rng_range(-6, 0);
    case DENSITY_NORMAL:
    default:             return rng_range(-2, 4);
  }
}

static uint32_t day_seed(int year, int yday, int city_seed) {
  return (uint32_t)(year * 367 + yday) * 2654435761u ^
         (uint32_t)(city_seed * 97 + 13);
}

static void generate_layout(int width, DensityMode density) {
  s_slot_count = 0;
  int x = rng_range(-8, 4);
  while (x < width + 8 && s_slot_count < SKYLINE_MAX_SLOTS) {
    int id = rng_range(0, BUILDING_SPRITE_COUNT);
    s_slots[s_slot_count].id = (uint8_t)id;
    s_slots[s_slot_count].x = (int16_t)x;
    s_slot_count++;
    x += building_width(id) + overlap_for_density(density);
  }
}

static void persist_layout(int year, int yday, int city_seed) {
  persist_write_int(PERSIST_SKY_YEAR, year);
  persist_write_int(PERSIST_SKY_DAY, yday);
  persist_write_int(PERSIST_SKY_SEED, city_seed);
  persist_write_int(PERSIST_SKY_COUNT, s_slot_count);
  uint8_t ids[SKYLINE_MAX_SLOTS];
  for (int i = 0; i < s_slot_count; i++) {
    ids[i] = s_slots[i].id;
  }
  persist_write_data(PERSIST_SKY_IDS, ids, (size_t)s_slot_count);
}

static bool try_load_ids(int year, int yday, int city_seed, uint8_t *ids, int *count_out) {
  if (!persist_exists(PERSIST_SKY_YEAR) || !persist_exists(PERSIST_SKY_DAY) ||
      !persist_exists(PERSIST_SKY_SEED) || !persist_exists(PERSIST_SKY_COUNT) ||
      !persist_exists(PERSIST_SKY_IDS)) {
    return false;
  }
  if (persist_read_int(PERSIST_SKY_YEAR) != year ||
      persist_read_int(PERSIST_SKY_DAY) != yday ||
      persist_read_int(PERSIST_SKY_SEED) != city_seed) {
    return false;
  }
  int count = persist_read_int(PERSIST_SKY_COUNT);
  if (count <= 0 || count > SKYLINE_MAX_SLOTS) return false;
  int read = persist_read_data(PERSIST_SKY_IDS, ids, (size_t)count);
  if (read != count) return false;
  *count_out = count;
  return true;
}

static void layout_from_ids(const uint8_t *ids, int count, int width, DensityMode density) {
  s_slot_count = 0;
  int x = rng_range(-8, 4);
  for (int i = 0; i < count && s_slot_count < SKYLINE_MAX_SLOTS; i++) {
    int id = ids[i] % BUILDING_SPRITE_COUNT;
    s_slots[s_slot_count].id = (uint8_t)id;
    s_slots[s_slot_count].x = (int16_t)x;
    s_slot_count++;
    x += building_width(id) + overlap_for_density(density);
  }
  while (x < width + 8 && s_slot_count < SKYLINE_MAX_SLOTS) {
    int id = rng_range(0, BUILDING_SPRITE_COUNT);
    s_slots[s_slot_count].id = (uint8_t)id;
    s_slots[s_slot_count].x = (int16_t)x;
    s_slot_count++;
    x += building_width(id) + overlap_for_density(density);
  }
}

void skyline_rebuild(int year, int yday, int width) {
  const FaceSettings *cfg = settings_get();
  rng_seed(day_seed(year, yday, cfg->city_seed));
  generate_layout(width, cfg->density);
  persist_layout(year, yday, cfg->city_seed);
  s_built_year = year;
  s_built_yday = yday;
  s_built_width = width;
  s_built_seed = cfg->city_seed;
  s_built_density = (int)cfg->density;
}

void skyline_ensure_today(int year, int yday, int width) {
  const FaceSettings *cfg = settings_get();

  if (s_built_year == year && s_built_yday == yday &&
      s_built_width == width && s_built_seed == cfg->city_seed &&
      s_built_density == (int)cfg->density && s_slot_count > 0) {
    return;
  }

  uint8_t ids[SKYLINE_MAX_SLOTS];
  int count = 0;
  if (try_load_ids(year, yday, cfg->city_seed, ids, &count)) {
    rng_seed(day_seed(year, yday, cfg->city_seed) ^ 0xA5A5u);
    layout_from_ids(ids, count, width, cfg->density);
    persist_layout(year, yday, cfg->city_seed);
    s_built_year = year;
    s_built_yday = yday;
    s_built_width = width;
    s_built_seed = cfg->city_seed;
    s_built_density = (int)cfg->density;
    return;
  }

  skyline_rebuild(year, yday, width);
}

SkyPeriod skyline_period_for_hour(int hour, SkyMode mode) {
  switch (mode) {
    case SKY_DAWN:  return PERIOD_DAWN;
    case SKY_DAY:   return PERIOD_DAY;
    case SKY_DUSK:  return PERIOD_DUSK;
    case SKY_NIGHT: return PERIOD_NIGHT;
    case SKY_AUTO:
    default:
      if (hour >= 5 && hour < 8) return PERIOD_DAWN;
      if (hour >= 8 && hour < 17) return PERIOD_DAY;
      if (hour >= 17 && hour < 20) return PERIOD_DUSK;
      return PERIOD_NIGHT;
  }
}

SkyPalette skyline_palette(SkyPeriod period, bool invert_bw) {
  SkyPalette p;
#if defined(PBL_COLOR)
  (void)invert_bw;
  switch (period) {
    case PERIOD_DAWN:
      p.sky_top = GColorFromHEX(0x2A1848);
      p.sky_bot = GColorFromHEX(0xFFAA55);
      p.ground = GColorBlack;
      p.text = GColorWhite;
      p.night = false;
      break;
    case PERIOD_DAY:
      p.sky_top = GColorFromHEX(0x55AAFF);
      p.sky_bot = GColorFromHEX(0xAAFFFF);
      p.ground = GColorBlack;
      p.text = GColorBlack;
      p.night = false;
      break;
    case PERIOD_DUSK:
      p.sky_top = GColorFromHEX(0x000055);
      p.sky_bot = GColorFromHEX(0xFF5500);
      p.ground = GColorBlack;
      p.text = GColorWhite;
      p.night = false;
      break;
    case PERIOD_NIGHT:
    default:
      p.sky_top = GColorFromHEX(0x000000);
      p.sky_bot = GColorFromHEX(0x000055);
      p.ground = GColorBlack;
      p.text = GColorWhite;
      p.night = true;
      break;
  }
#else
  if (invert_bw) {
    p.sky_top = GColorWhite;
    p.sky_bot = GColorWhite;
    p.ground = GColorBlack;
    p.text = GColorBlack;
  } else {
    p.sky_top = GColorBlack;
    p.sky_bot = GColorBlack;
    p.ground = GColorBlack;
    p.text = GColorWhite;
  }
  p.night = (period == PERIOD_NIGHT || period == PERIOD_DUSK);
#endif
  return p;
}

static void draw_sky_gradient(GContext *ctx, GRect bounds, SkyPalette pal) {
#if defined(PBL_COLOR)
  const int bands = 8;
  for (int i = 0; i < bands; i++) {
    int y0 = bounds.origin.y + (bounds.size.h * i) / bands;
    int y1 = bounds.origin.y + (bounds.size.h * (i + 1)) / bands;
    GColor c = (i < bands / 2) ? pal.sky_top : pal.sky_bot;
    if (i == bands / 2 - 1 || i == bands / 2) {
      c = (i % 2 == 0) ? pal.sky_top : pal.sky_bot;
    }
    graphics_context_set_fill_color(ctx, c);
    graphics_fill_rect(ctx, GRect(bounds.origin.x, y0, bounds.size.w, y1 - y0),
                       0, GCornerNone);
  }
#else
  graphics_context_set_fill_color(ctx, pal.sky_top);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
#endif
}

void skyline_init(void) {
  for (int i = 0; i < BUILDING_SPRITE_COUNT; i++) {
    s_bldgs[i] = gbitmap_create_with_resource(BLDG_RES[i]);
  }
  s_moon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOON);
  s_sun = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUN);
}

void skyline_deinit(void) {
  for (int i = 0; i < BUILDING_SPRITE_COUNT; i++) {
    if (s_bldgs[i]) {
      gbitmap_destroy(s_bldgs[i]);
      s_bldgs[i] = NULL;
    }
  }
  if (s_moon) { gbitmap_destroy(s_moon); s_moon = NULL; }
  if (s_sun) { gbitmap_destroy(s_sun); s_sun = NULL; }
}

void skyline_draw(GContext *ctx, GRect bounds, int hour, int minute) {
  (void)minute;
  const FaceSettings *cfg = settings_get();
  SkyPeriod period = skyline_period_for_hour(hour, cfg->sky_mode);
  SkyPalette pal = skyline_palette(period, cfg->invert_bw);

  draw_sky_gradient(ctx, bounds, pal);

  if (cfg->show_stars && pal.night) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    rng_seed(day_seed(t->tm_year, t->tm_yday, 11) ^ 0x51A5u);
    graphics_context_set_fill_color(ctx, pal.text);
    for (int i = 0; i < STAR_COUNT; i++) {
      int sx = rng_range(2, bounds.size.w - 2);
      int sy = rng_range(2, bounds.size.h / 2);
      graphics_fill_rect(ctx, GRect(sx, sy, 1, 1), 0, GCornerNone);
      if ((rng_next() & 3u) == 0) {
        graphics_fill_rect(ctx, GRect(sx - 1, sy, 3, 1), 0, GCornerNone);
        graphics_fill_rect(ctx, GRect(sx, sy - 1, 1, 3), 0, GCornerNone);
      }
    }
  }

  if (cfg->show_celestial) {
    GBitmap *cel = pal.night ? s_moon : s_sun;
    if (cel) {
      GSize sz = gbitmap_get_bounds(cel).size;
      int cx = bounds.size.w - sz.w - 10;
      int cy = 10 + (pal.night ? 4 : 2);
#if defined(PBL_COLOR)
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
#else
      graphics_context_set_compositing_mode(ctx,
          cfg->invert_bw ? GCompOpAssignInverted : GCompOpAssign);
#endif
      graphics_draw_bitmap_in_rect(ctx, cel, GRect(cx, cy, sz.w, sz.h));
    }
  }

#if defined(PBL_COLOR)
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
#else
  graphics_context_set_compositing_mode(ctx,
      cfg->invert_bw ? GCompOpAssignInverted : GCompOpAssign);
#endif
  for (int i = 0; i < s_slot_count; i++) {
    int id = s_slots[i].id;
    GBitmap *bmp = s_bldgs[id];
    if (!bmp) continue;
    GSize sz = gbitmap_get_bounds(bmp).size;
    int y = bounds.size.h - sz.h;
    graphics_draw_bitmap_in_rect(ctx, bmp,
                                 GRect(s_slots[i].x, y, sz.w, sz.h));
  }

  graphics_context_set_stroke_color(ctx, pal.ground);
  graphics_draw_line(ctx,
                     GPoint(0, bounds.size.h - 1),
                     GPoint(bounds.size.w, bounds.size.h - 1));
}
