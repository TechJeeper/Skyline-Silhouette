#include "face.h"
#include "settings.h"
#include "skyline.h"

static Window *s_window;
static Layer *s_layer;

static int s_hour;
static int s_minute;
static int s_wday;
static int s_mday;
static int s_mon;
static int s_year = -1;
static int s_yday = -1;

static bool use_24h(void) {
  const FaceSettings *s = settings_get();
  if (s->time_format == TIME_FMT_24H) return true;
  if (s->time_format == TIME_FMT_12H) return false;
  return clock_is_24h_style();
}

static void format_time(char *buf, size_t n) {
  int hour = s_hour;
  const bool is24 = use_24h();
  if (!is24) {
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  if (settings_get()->leading_zero) {
    snprintf(buf, n, "%02d:%02d", hour, s_minute);
  } else {
    snprintf(buf, n, "%d:%02d", hour, s_minute);
  }
}

static void format_date(char *buf, size_t n) {
  static const char *const DOW[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *const MON[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  snprintf(buf, n, "%s %d %s", DOW[s_wday % 7], s_mday, MON[s_mon % 12]);
}

static void update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const FaceSettings *cfg = settings_get();

  skyline_ensure_today(s_year, s_yday, bounds.size.w);
  skyline_draw(ctx, bounds, s_hour, s_minute);

  SkyPeriod period = skyline_period_for_hour(s_hour, cfg->sky_mode);
  SkyPalette pal = skyline_palette(period, cfg->invert_bw);

  char time_buf[16];
  format_time(time_buf, sizeof(time_buf));
  graphics_context_set_text_color(ctx, pal.text);
  graphics_draw_text(ctx, time_buf,
                     fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
                     GRect(0, 8, bounds.size.w, 44),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  if (cfg->show_date) {
    char date_buf[24];
    format_date(date_buf, sizeof(date_buf));
    graphics_draw_text(ctx, date_buf,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0, 50, bounds.size.w, 18),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
  }
}

static void mark_dirty(void) {
  if (s_layer) {
    layer_mark_dirty(s_layer);
  }
  if (s_window) {
    SkyPeriod period = skyline_period_for_hour(s_hour, settings_get()->sky_mode);
    SkyPalette pal = skyline_palette(period, settings_get()->invert_bw);
    window_set_background_color(s_window, pal.sky_top);
  }
}

void face_init(Window *window) {
  s_window = window;
  skyline_init();

  Layer *root = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);

  face_tick();
  mark_dirty();
}

void face_deinit(void) {
  if (s_layer) {
    layer_destroy(s_layer);
    s_layer = NULL;
  }
  skyline_deinit();
  s_window = NULL;
}

void face_tick(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  const int prev_yday = s_yday;
  const int prev_year = s_year;

  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_wday = t->tm_wday;
  s_mday = t->tm_mday;
  s_mon = t->tm_mon;
  s_year = t->tm_year;
  s_yday = t->tm_yday;

  // Midnight: new calendar day → brand-new procedural skyline.
  if (s_layer && (s_yday != prev_yday || s_year != prev_year) && prev_year >= 0) {
    GRect bounds = layer_get_bounds(s_layer);
    skyline_rebuild(s_year, s_yday, bounds.size.w);
  }

  mark_dirty();
}

void face_apply_settings(void) {
  if (s_layer) {
    GRect bounds = layer_get_bounds(s_layer);
    // City seed / density changes force a rebuild via ensure_today checks.
    skyline_ensure_today(s_year, s_yday, bounds.size.w);
  }
  mark_dirty();
}
