#include "pebble.h"

static Window *window;
static TextLayer *time_layer; 
static TextLayer *date_layer; 
static TextLayer *battery_layer;
static GBitmap *City_Night;
static GBitmap *City_Day;
static BitmapLayer* City_Layer;
static GBitmap *bt_connected;
static GBitmap *bt_disconnected;
static BitmapLayer* bt_connected_layer;
static InverterLayer *inv_layer;
static int day = 0;

int WIDTH = 144;
int HEIGHT = 168;
int ICON_WIDTH = 14;
int ICON_HEIGHT = 14;

static long catoi(const char *s) {
  const char *p = s, *q;
  long n = 0;
  int sign = 1, k = 1;
  if (p != NULL) {
      if (*p != '\0') {
          if ((*p == '+') || (*p == '-')) {
              if (*p++ == '-') sign = -1;
          }
          for (q = p; (*p != '\0'); p++);
          for (--p; p >= q; --p, k *= 10) n += (*p - '0') * k;
      }
  }
  return n * sign;
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "~%d", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d ", charge_state.charge_percent);
  }
  inverter_layer_destroy(inv_layer);
  inv_layer = inverter_layer_create(GRect(3, 3, (charge_state.charge_percent/10)*2, 9));
  layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
  text_layer_set_text(battery_layer, battery_text);
}

static void handle_bluetooth(bool connected) {
  if (connected == 1){
    bitmap_layer_set_bitmap(bt_connected_layer, bt_connected);
  } else {
    bitmap_layer_set_bitmap(bt_connected_layer, bt_disconnected);
  }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";
  static char date_text[] = "September 01,  1995";
  static char dp[] = "18";
  static int old_day = 0;
  old_day = day;
  if (clock_is_24h_style() == true){
    strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
  } else {
    strftime(time_text, sizeof(time_text), "%l:%M", tick_time);
  }
  strftime(date_text, sizeof(date_text), "%b %d, %G", tick_time);
  strftime(dp, sizeof(dp), "%H", tick_time);
  if (catoi(dp)>=18 || catoi(dp)<6) {
	  day=0;
  }else{
	  day=1;
  }
  if(day != old_day){
	  if (day == 0) {
		text_layer_set_text_color(time_layer, GColorWhite);
		text_layer_set_text_color(date_layer, GColorBlack);
		text_layer_set_text_color(battery_layer, GColorWhite);
		bitmap_layer_set_background_color(City_Layer, GColorBlack);
		bitmap_layer_set_bitmap(City_Layer, City_Night);
		bt_disconnected = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
		bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
	  } else {
		text_layer_set_text_color(time_layer, GColorBlack);
		text_layer_set_text_color(date_layer, GColorWhite); 
		text_layer_set_text_color(battery_layer, GColorBlack);
		bitmap_layer_set_background_color(City_Layer, GColorWhite);
		bitmap_layer_set_bitmap(City_Layer, City_Day);
		bt_disconnected = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED_WHITE);
		bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED_WHITE);
	  }
	  handle_bluetooth(bluetooth_connection_service_peek());
  }
  
  text_layer_set_text(time_layer, time_text);
  text_layer_set_text(date_layer, date_text);
  handle_battery(battery_state_service_peek());
}

static void do_init(void) {

  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  /* Background */
  City_Night = gbitmap_create_with_resource(RESOURCE_ID_BG_NIGHT_CITY);
  City_Day = gbitmap_create_with_resource(RESOURCE_ID_BG_DAY_CITY);
  City_Layer = bitmap_layer_create(GRect(0, 0, WIDTH, HEIGHT));
  bitmap_layer_set_background_color(City_Layer, GColorBlack);
  bitmap_layer_set_bitmap(City_Layer, City_Night);
  layer_add_child(root_layer, bitmap_layer_get_layer(City_Layer));

  /* Time block */
  time_layer = text_layer_create(GRect(0, 52, frame.size.w, 44));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font( FONT_KEY_BITHAM_42_BOLD ));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
  /* Date block */
  date_layer = text_layer_create(GRect(0, 150, frame.size.w, 16));
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14_BOLD ));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);

  /* Bluetooth block */
  bt_disconnected = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
  bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
  bt_connected_layer = bitmap_layer_create(GRect(WIDTH - ICON_WIDTH, 1, ICON_WIDTH, ICON_HEIGHT));
  bitmap_layer_set_background_color(bt_connected_layer, GColorBlack);
  layer_add_child(root_layer, bitmap_layer_get_layer(bt_connected_layer));

  /* Battery block */
  battery_layer = text_layer_create(GRect(30, -2, 20, 16 ));
  text_layer_set_text_color(battery_layer, GColorBlack);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "100");

  /* Init blocks */
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  battery_state_service_subscribe(&handle_battery);

  bool connected = bluetooth_connection_service_peek();
  handle_bluetooth(connected);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  layer_add_child(root_layer, text_layer_get_layer(date_layer));
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));
}

static void do_deinit(void) {
  gbitmap_destroy(City_Night);
  gbitmap_destroy(City_Day);
  gbitmap_destroy(bt_disconnected);
  gbitmap_destroy(bt_connected);
  bitmap_layer_destroy(City_Layer);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  inverter_layer_destroy(inv_layer);
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(battery_layer);
  window_destroy(window);
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}