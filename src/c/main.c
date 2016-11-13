// author: korovkin@gmail.com
// all rights reserved

#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer = NULL;
static TextLayer *s_week_day_layer = NULL;
static TextLayer *s_date_layer = NULL;
static TextLayer *s_steps_layer = NULL;
static TextLayer *s_battery_layer = NULL;
static TextLayer *s_walk_meters_layer = NULL;
static TextLayer *s_sleep_hours_layer = NULL;

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static struct tm last_tick_time = {0};
  static char s_time_text[64] = {0};
  if (last_tick_time.tm_hour != tick_time->tm_hour || last_tick_time.tm_min != tick_time->tm_min) {
    strftime(s_time_text, sizeof(s_time_text), "%H:%M", tick_time);
    text_layer_set_text(s_time_layer, s_time_text);
  }

  static char s_date_text[64] = {0};
  static char s_full_date[64] = {0};
  if (last_tick_time.tm_mon != tick_time->tm_mon || last_tick_time.tm_mday != tick_time->tm_mday) {
    strftime(s_date_text, sizeof(s_date_text), "%d/%m", tick_time);
    char* day = "";
    switch(tick_time->tm_wday) {
      case 0:  day = "SUNDAY"; break;
      case 1:  day = "MONDAY"; break;
      case 2:  day = "TUESDAY"; break;
      case 3:  day = "WEDNESDAY"; break;
      case 4:  day = "THURSDAY"; break;
      case 5:  day = "FRIDAY"; break;
      case 6:  day = "SATURDAY"; break;
    }
    text_layer_set_text(s_week_day_layer, day);

    snprintf(s_full_date, sizeof(s_full_date), "%s", s_date_text);
    text_layer_set_text(s_date_layer, s_full_date);
  }

  const int32_t stepsToday = health_service_sum_today(HealthMetricStepCount);
  static char s_steps_text[64] = {0};
  snprintf(s_steps_text, sizeof(s_steps_text), "%d",
                  (int)stepsToday);
  text_layer_set_text(s_steps_layer, s_steps_text);

  static char s_battery_text[64] = {0};
  static BatteryChargeState lastBattery = {0};
  const BatteryChargeState battery = battery_state_service_peek();
  if (lastBattery.charge_percent != battery.charge_percent) {
    snprintf(s_battery_text,
             sizeof(s_battery_text), "%d",
             battery.charge_percent);
    text_layer_set_text(s_battery_layer, s_battery_text);
    lastBattery = battery;
  }

  const int32_t walkedMeters = health_service_sum_today(HealthMetricWalkedDistanceMeters);
  static int32_t lastWalkedMeters = 0;
  static char s_upper_right_text[64] = {0};
  if (lastWalkedMeters != walkedMeters) {
    snprintf(s_upper_right_text,
             sizeof(s_upper_right_text), "%d.%d",
             (int)walkedMeters/1000,
             (int)(10*(walkedMeters%1000)/1000));
    text_layer_set_text(s_walk_meters_layer, s_upper_right_text);
  }
  lastWalkedMeters = walkedMeters;

  const int32_t sleepSeconds = health_service_sum_today(HealthMetricSleepSeconds);
  static char s_upper_left_text[64] = {0};
  static int32_t lastSleepSeconds = 0;
  if (lastSleepSeconds != sleepSeconds) {
    snprintf(s_upper_left_text,
             sizeof(s_upper_left_text), "%d",
             (int)sleepSeconds/3600);
    text_layer_set_text(s_sleep_hours_layer, s_upper_left_text);
    lastSleepSeconds = sleepSeconds;
  }

  last_tick_time = *tick_time;
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  GRect weekdayFrame = bounds;
  weekdayFrame.origin.y += 24;
  s_week_day_layer = text_layer_create(weekdayFrame);
  text_layer_set_text_color(s_week_day_layer, GColorGreen);
  text_layer_set_background_color(s_week_day_layer, GColorClear);
  text_layer_set_text_alignment(s_week_day_layer, GTextAlignmentCenter);
  text_layer_set_font(s_week_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  GRect timeFrame = bounds;
  timeFrame.origin.y += 40;
  s_time_layer = text_layer_create(timeFrame);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));

  GRect dateFrame = timeFrame;
  dateFrame.origin.y += 40;
  dateFrame.origin.x += 60;

  GRect dayFrame = dateFrame;
  dayFrame.origin.x = 0;
  s_date_layer = text_layer_create(dayFrame);
  text_layer_set_text_color(s_date_layer, GColorGreen);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));

  GRect stepsFrame = dayFrame;
  stepsFrame.origin.x = -2;
  stepsFrame.origin.y = bounds.size.h-26;
  s_steps_layer = text_layer_create(stepsFrame);
  text_layer_set_text_color(s_steps_layer, GColorRed);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentRight);
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

  GRect batteryFrame = stepsFrame;
  batteryFrame.origin.x = 2;
  s_battery_layer = text_layer_create(batteryFrame);
  text_layer_set_text_color(s_battery_layer, GColorRed);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

  GRect upperRight = stepsFrame;
  upperRight.origin.x = -2;
  upperRight.origin.y -= 20;
  s_walk_meters_layer = text_layer_create(upperRight);
  text_layer_set_text_color(s_walk_meters_layer, GColorRed);
  text_layer_set_background_color(s_walk_meters_layer, GColorClear);
  text_layer_set_text_alignment(s_walk_meters_layer, GTextAlignmentRight);
  text_layer_set_font(s_walk_meters_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

  GRect upperLeft = upperRight;
  upperLeft.origin.x = -2;
  upperLeft.origin.y -= 20;
  s_sleep_hours_layer = text_layer_create(upperLeft);
  text_layer_set_text_color(s_sleep_hours_layer, GColorRed);
  text_layer_set_background_color(s_sleep_hours_layer, GColorClear);
  text_layer_set_text_alignment(s_sleep_hours_layer, GTextAlignmentRight);
  text_layer_set_font(s_sleep_hours_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  layer_add_child(window_layer, text_layer_get_layer(s_week_day_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_walk_meters_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_hours_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_week_day_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_steps_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_walk_meters_layer);
  text_layer_destroy(s_sleep_hours_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "hello hApp");
  init();
  app_event_loop();
  deinit();
}
