#include <pebble.h>
#define KEY_CITY 0
#define KEY_TEMP 1
#define KEY_WEATHER 2

#define ANTIALIASING true

static Window *s_main_window;
static GFont s_digital_font, s_menu_font;
static Layer *s_battery_layer;
static TextLayer *s_clock_layer, *s_date_layer, *s_weather_layer, *s_temp_layer;
static uint8_t battery_level;
static int full_width = (int)144;
static int full_height = (int)168;
static int margin = (int)12;
static int bar_width = (int)7;
static int width;
static int start;
static int start_weather;

////////////////////
// handle battery //
////////////////////
void battery_state_handler(BatteryChargeState charge) {
	battery_level = charge.charge_percent;
  battery_level = battery_level/10;
  battery_level = 10-battery_level;
  layer_mark_dirty(s_battery_layer);
}

////////////////////
// battery update //
////////////////////
static void battery_update_proc(Layer *layer, GContext *ctx) {

  ////////////////////////////
  // draw gray battery bars //
  ////////////////////////////
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  
  ///////////////////////////////////////
  // loop until 10 gray bars are drawn //
  ///////////////////////////////////////
  for(int i=1; i<10; i++) {
    graphics_fill_rect(ctx, GRect(margin/2, start + (i*(bar_width+1)), width, bar_width), 0, 0);
  }

  ////////////////////////////////////////////////
  // draw white bars to represent battery level //
  ////////////////////////////////////////////////
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for(int i=battery_level; i<10; i++) {
    graphics_fill_rect(ctx, GRect(margin/2, start + (i*(bar_width+1)), width, bar_width), 0, 0);
  }
  
  ///////////////////////
  // draw black circle //
  ///////////////////////
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(124, 92+start), 98);
  
  /////////////////////////////////////////
  // draw gray top line on top of circle //
  /////////////////////////////////////////
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);  
  graphics_fill_rect(ctx, GRect(margin/2, start + (0*(bar_width+1)), width, bar_width), 0, 0);
  
  //////////////////////////////////////////////////////
  // draw white line on top of circle if 100% charged //
  //////////////////////////////////////////////////////
  if(battery_level==0) {
    // draw top line
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);  
    graphics_fill_rect(ctx, GRect(margin/2, start + (0*(bar_width+1)), width, bar_width), 0, 0);    
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  ///////////////////////////////////
  // set background color to black //
  ///////////////////////////////////
  window_set_background_color(window, GColorBlack);
  
  //////////////////////////
  // set font for digital //
  //////////////////////////
  s_digital_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DS_DIGITAL_FONT_36));
  
  //////////////////
  // set MGS font //
  //////////////////
  s_menu_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MENU_FONT_12));
  
  ////////////////////////
  // draw battery layer //
  ////////////////////////
  s_battery_layer = layer_create(bounds);
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
  
  //////////////////////
  // draw clock layer //
  //////////////////////
  int clock_width = 88;
  int clock_height = 36;
  s_clock_layer = text_layer_create(GRect(full_width-(margin/2)-clock_width, 91, clock_width, clock_height));
  text_layer_set_background_color(s_clock_layer, GColorClear);
  text_layer_set_text_color(s_clock_layer, GColorWhite);
  text_layer_set_text_alignment(s_clock_layer, GTextAlignmentRight);
  text_layer_set_font(s_clock_layer, s_digital_font);
  layer_add_child(s_battery_layer, text_layer_get_layer(s_clock_layer));
  
  /////////////////////
  // draw date layer //
  /////////////////////
  int starting_point = (full_height/2)+(bar_width*6);
  int layer_height = (full_height-starting_point)/3;
  s_date_layer = text_layer_create(GRect(margin/2, starting_point, full_width-margin, layer_height));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_menu_font);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));  
  
  ////////////////////////////
  // draw temperature layer //
  ////////////////////////////
  s_temp_layer = text_layer_create(GRect(margin/2, starting_point+layer_height, full_width-margin, layer_height));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temp_layer, s_menu_font);
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));    
  
  ///////////////////////////////////
  // draw weather conditions layer //
  ///////////////////////////////////
  s_weather_layer = text_layer_create(GRect(margin/2, starting_point+(layer_height*2), full_width-margin, layer_height));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_font(s_weather_layer, s_menu_font);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));    
}

static void update_time() {
  // get a tm strucutre
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write the current hours into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H.%M" : "%I.%M", tick_time);
  
  static char date_buffer[8];
  strftime(date_buffer, sizeof(date_buffer), "%a %e", tick_time);
  
  // display this time on the text layer
  text_layer_set_text(s_clock_layer, s_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

///////////////////
// unload window //
///////////////////
static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_digital_font);
  fonts_unload_custom_font(s_menu_font);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_clock_layer);
}

///////////////////////
// for weather calls //
///////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temp_buf[8];
  static char wea_buf[32];
  
  static char temp_layer_buf[32];
  static char wea_layer_buf[32];
  
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMP);
  Tuple *wea_tuple = dict_find(iterator, KEY_WEATHER);
  
  if(temp_tuple && wea_tuple) {
    snprintf(temp_buf, sizeof(temp_buf), "%d", (int)temp_tuple->value->int32);
    snprintf(wea_buf, sizeof(wea_buf), "%s", wea_tuple->value->cstring);
    
    snprintf(temp_layer_buf, sizeof(temp_layer_buf), "%s", temp_buf);
    text_layer_set_text(s_temp_layer, temp_buf);
    
    snprintf(wea_layer_buf, sizeof(wea_layer_buf), "%s", wea_buf);
    text_layer_set_text(s_weather_layer, wea_buf);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // set values for dynamic changes
  width = 144-margin;
  start = (168-(bar_width*10))/2;
  start_weather = 84+((bar_width*10)/2);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  // register for clock update
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();  
  
  // get battery value
  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_level = battery_level/10;
  battery_level = 10-battery_level;
  
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  
  // Open AppMessage for weather callbacks
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);   
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  battery_state_service_subscribe(&battery_state_handler);
  app_event_loop();
  deinit();
}