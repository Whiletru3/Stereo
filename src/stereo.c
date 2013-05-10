#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

// Stereo (by Whiletrue), based on Illusion's code (https://github.com/dmnd/illusion) 


#define MY_UUID { \
  0x59, 0x0E, 0x42, 0x1D, 0x47, 0x9E, 0x4C, 0xE9, \
  0xA4, 0xBD, 0xD2, 0xCF, 0xFE, 0x34, 0x94, 0x01 \
}

PBL_APP_INFO(MY_UUID,
             "Stereo", "Whiletrue",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);             

Window window;
BmpContainer background_image;

const int RESOURCE_ID_NUMERAL[] = {
  RESOURCE_ID_NUMERAL_0,
  RESOURCE_ID_NUMERAL_1,
  RESOURCE_ID_NUMERAL_2,
  RESOURCE_ID_NUMERAL_3,
  RESOURCE_ID_NUMERAL_4,
  RESOURCE_ID_NUMERAL_5,
  RESOURCE_ID_NUMERAL_6,
  RESOURCE_ID_NUMERAL_7,
  RESOURCE_ID_NUMERAL_8,
  RESOURCE_ID_NUMERAL_9,
  RESOURCE_ID_NUMERAL_10,
  RESOURCE_ID_NUMERAL_11,
  RESOURCE_ID_NUMERAL_12,
  RESOURCE_ID_NUMERAL_13,
  RESOURCE_ID_NUMERAL_14,
  RESOURCE_ID_NUMERAL_15,
  RESOURCE_ID_NUMERAL_16,
  RESOURCE_ID_NUMERAL_17,
  RESOURCE_ID_NUMERAL_18,
  RESOURCE_ID_NUMERAL_19
};

#define TOTAL_TIME_DIGITS 4
BmpContainer time_digits_images[TOTAL_TIME_DIGITS];

void set_container_image(BmpContainer *bmp_container, const int resource_id,
                         GPoint origin) {
  layer_remove_from_parent(&bmp_container->layer.layer);
  bmp_deinit_container(bmp_container);

  bmp_init_container(resource_id, bmp_container);

  GRect frame = layer_get_frame(&bmp_container->layer.layer);
  frame.origin.x = origin.x;
  frame.origin.y = origin.y;
  layer_set_frame(&bmp_container->layer.layer, frame);

  layer_add_child(&window.layer, &bmp_container->layer.layer);
}

const int XS[] = {76, 112};
const int YS[] = {18, 100};

void set_time(unsigned short display_hour, unsigned short display_min) {
  // hours
  set_container_image(
    &time_digits_images[0],
    RESOURCE_ID_NUMERAL[display_hour/10],
    GPoint(XS[0], YS[0]));
  set_container_image(
    &time_digits_images[1],
    RESOURCE_ID_NUMERAL[(display_hour%10)+10], // + 10 to catch the right ressources
    GPoint(XS[1], YS[0]));

  // minutes
  set_container_image(
    &time_digits_images[2],
    RESOURCE_ID_NUMERAL[display_min/10],
    GPoint(XS[0], YS[1]));
  set_container_image(
    &time_digits_images[3],
    RESOURCE_ID_NUMERAL[(display_min%10)+10], // + 10 to catch the right ressources
    GPoint(XS[1], YS[1]));
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  return display_hour ? display_hour : 12;
}

void update_display(PblTm *current_time) {
  // TODO: Only update changed values?
  unsigned short display_hour = get_display_hour(current_time->tm_hour);
  set_time(display_hour, current_time->tm_min);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  update_display(t->tick_time);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Whiletrue");
  window_stack_push(&window, true /* Animated */);

  resource_init_current_app(&APP_RESOURCES);

  // draw background
  bmp_init_container(RESOURCE_ID_BACKGROUND, &background_image);
  layer_add_child(&window.layer, &background_image.layer.layer);

  // avoid a blank screen on watch start
  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;
  bmp_deinit_container(&background_image);

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    bmp_deinit_container(&time_digits_images[i]);
  }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
