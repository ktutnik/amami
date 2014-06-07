// Standard includes
#include "pebble.h"
#include "time_as_words.h"


// App-specific data
Window *window; 
TextLayer *time_layer;
TextLayer *hour_layer;
TextLayer *minute_one_layer;
TextLayer *minute_two_layer;
TextLayer *info_layer;
TextLayer *battery_layer;
TextLayer *date_layer;
TextLayer *charging_layer;
bool is_in_detail_face = false;

static void battery_callback(BatteryChargeState state){
  if(state.is_charging){
    static char battery_text[] = "Mengisi 100%";
    layer_set_hidden(text_layer_get_layer(charging_layer), false);
    snprintf(battery_text, sizeof(battery_text), "Mengisi %d%%", state.charge_percent);
    text_layer_set_text(charging_layer, battery_text);
  }
  /*else if(state.is_plugged){
    layer_set_hidden(text_layer_get_layer(charging_layer), false);
    text_layer_set_text(charging_layer, "Penuh");
  }*/
  else {
    layer_set_hidden(text_layer_get_layer(charging_layer), true);
  }
}

static void show_normal_face(){
  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  layer_set_bounds(text_layer_get_layer(time_layer), GRect(0, 20, frame.size.w, frame.size.h));
  layer_set_hidden(text_layer_get_layer(info_layer), true);
  //tampilkan charging kalo lagi ngecas
  battery_callback(battery_state_service_peek());
}

static void show_detail_face(){
  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  layer_set_bounds(text_layer_get_layer(time_layer), GRect(0, 0, frame.size.w, frame.size.h));
  layer_set_hidden(text_layer_get_layer(info_layer), false);
  
  //print battery
  BatteryChargeState charge_state = battery_state_service_peek();
  static char battery_text[] = "100%";
  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  text_layer_set_text(battery_layer, battery_text);
  
  //print date
  static const char* const DAYS[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  static char date_text[] = " 31";
  static char day_text[] = " Minggu 09 ";
  strcpy(day_text, DAYS[tick_time->tm_wday]);
  strftime(date_text, sizeof(date_text), " %e", tick_time);
  strcat(day_text, date_text);
  text_layer_set_text(date_layer, day_text);
  
  //sembunyikan charging kalo lagi ngecas
  layer_set_hidden(text_layer_get_layer(charging_layer), true);
}


static void timer_callback(void *context) {
  is_in_detail_face = false;
  show_normal_face();
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if(is_in_detail_face == false){
    is_in_detail_face = true;
    show_detail_face();
    app_timer_register(5000, timer_callback, NULL);
  }
}


static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char current_hour[] = "duabelas";
  static char current_tens[] = "duabelas";
  static char current_ones[] = "duabelas";
    
  time_as_words(tick_time->tm_hour, tick_time->tm_min, current_hour, current_tens, current_ones);
    
  text_layer_set_text(hour_layer, current_hour);
  text_layer_set_text(minute_one_layer, current_tens);
  text_layer_set_text(minute_two_layer, current_ones);
}

static TextLayer * my_create_layer(GRect rect, char *font, GAlign align, GColor foreColor, GColor bgColor){
  TextLayer *layer = text_layer_create(rect);  
  text_layer_set_text_color(layer, foreColor);
  text_layer_set_background_color(layer, bgColor);
  text_layer_set_font(layer, fonts_get_system_font(font));
  text_layer_set_overflow_mode(layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text_alignment(layer, align);
  return layer;
}

// Handle the start-up of the app
static void do_init(void) {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  
  charging_layer = my_create_layer(GRect(0, 0, frame.size.w - 2, 26), FONT_KEY_ROBOTO_CONDENSED_21, 
                                   GTextAlignmentCenter, GColorWhite, GColorClear);
  hour_layer = my_create_layer(GRect(0, 0, frame.size.w, 50), FONT_KEY_BITHAM_42_BOLD, GAlignLeft, 
                               GColorWhite, GColorClear);
  minute_one_layer = my_create_layer(GRect(0, 38, frame.size.w, 50), FONT_KEY_BITHAM_42_LIGHT, 
                                     GAlignLeft, GColorWhite, GColorClear);
  minute_two_layer = my_create_layer(GRect(0, 75, frame.size.w, 50), FONT_KEY_BITHAM_42_LIGHT, 
                                     GAlignLeft, GColorWhite, GColorClear);
  battery_layer = my_create_layer(GRect(0, 0, frame.size.w - 2, 26), FONT_KEY_ROBOTO_CONDENSED_21, 
                                  GAlignRight, GColorWhite, GColorClear);
  date_layer = my_create_layer(GRect(0, 19, frame.size.w - 2, 26), FONT_KEY_ROBOTO_CONDENSED_21, 
                               GAlignRight, GColorWhite, GColorClear);
  
  text_layer_set_text(battery_layer, "100%");
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
  accel_tap_service_subscribe(&accel_tap_handler);
  battery_state_service_subscribe(&battery_callback);
  
  layer_add_child(root_layer, text_layer_get_layer(charging_layer));
  
  info_layer = text_layer_create(GRect(0, 120, frame.size.w, 50));
  text_layer_set_background_color(info_layer, GColorClear);
  layer_add_child(text_layer_get_layer(info_layer), text_layer_get_layer(battery_layer));
  layer_add_child(text_layer_get_layer(info_layer), text_layer_get_layer(date_layer));
  layer_add_child(root_layer, text_layer_get_layer(info_layer));

  time_layer = text_layer_create(GRect(0, 0, frame.size.w, frame.size.h));
  text_layer_set_background_color(time_layer, GColorClear);
  layer_add_child(text_layer_get_layer(time_layer), text_layer_get_layer(hour_layer));
  layer_add_child(text_layer_get_layer(time_layer), text_layer_get_layer(minute_one_layer));
  layer_add_child(text_layer_get_layer(time_layer), text_layer_get_layer(minute_two_layer));
  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  
  show_normal_face();
  
}

static void do_deinit(void) {
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  battery_state_service_unsubscribe();
  
  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_one_layer);
  text_layer_destroy(minute_two_layer);
  text_layer_destroy(battery_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(info_layer);
  text_layer_destroy(charging_layer);
  window_destroy(window);
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}