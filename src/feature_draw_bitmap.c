/*

   The original source image is from:

      <http://openclipart.org/detail/26728/aiga-litter-disposal-by-anonymous>

   The source image was converted from an SVG into a RGB bitmap using
   Inkscape. It has no transparency and uses only black and white as
   colors.

*/

#include "pebble.h"

#define MATH_PI 3.141592653589793238462
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50
#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define SHIP_WIDTH 22
#define SHIP_HEIGHT 22
#define METSPEEDMIN 4
#define METSPEEDMAX 15
  
static double next_radius = 3;
//static Window *window;
static GRect window_frame;
static Layer *disc_layer;
static AppTimer *timer;
static Window *window;
static Layer *layer;
static GBitmap *image;
static GBitmap *meteor;

static int px = 0; //144
static int py = 0; //168
static int mx = 0;
static int my = 0;
static int ms = 0;
int metVisible = 0;
static int deactAcc = 0;

/*static void disc_apply_accel(Disc *disc, AccelData accel) {
  Vec2d force;
  force.x = accel.x * ACCEL_RATIO;
  force.y = -accel.y * ACCEL_RATIO;
  disc_apply_force(disc, force);
}*/


static void timer_callback(void *data) {
  int createMet = rand() % 20;
  if (metVisible != 0) createMet = 0;
  //if (deactAcc>0) createMet = 0;
    
  if (createMet == 10) {
    // vytvori  meteorit
    mx = rand() % (SCREEN_WIDTH-22);
    my = -22;
    ms = METSPEEDMIN + (rand() % METSPEEDMAX);
    metVisible = 1;
  }
  
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };

  accel_service_peek(&accel);
  //if (deactAcc-- <= 0) {
    px = (SCREEN_WIDTH/2-SHIP_WIDTH/2)+(accel.x * 0.2);
    py = (SCREEN_HEIGHT/2-SHIP_HEIGHT/2)+(-accel.y * 0.2);
    //deactAcc = 0;
  //}
  
  if (px < 0) px = 0;
  if (px > SCREEN_WIDTH-SHIP_WIDTH) px = SCREEN_WIDTH-SHIP_WIDTH;
  if (py < 0) py = 0;
  if (py > SCREEN_HEIGHT-SHIP_HEIGHT*2) py = SCREEN_HEIGHT-(SHIP_HEIGHT*2);
  
  if (metVisible == 1) {
    my+=ms;
    if (my>SCREEN_HEIGHT) metVisible = 0;
  }
  if (px < (mx+22) && (px+22) > mx &&
        py < (my+22) && (py+22) > my) {
          metVisible = 0;
      /*    vibes_short_pulse();
          deactAcc = 5; */
  }
  
  layer_mark_dirty(layer);

  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);

}

 
  
static void layer_update_callback(Layer *me, GContext* ctx) {
  // We make sure the dimensions of the GRect to draw into
  // are equal to the size of the bitmap--otherwise the image
  // will automatically tile. Which might be what *you* want.

  GRect bounds = image->bounds;

  /*if ((deactAcc % 2) == 0)*/ {
    graphics_draw_bitmap_in_rect(ctx, image, (GRect) { .origin = { px, py }, .size = bounds.size });
  }
  
  if (metVisible>0) {
    graphics_draw_bitmap_in_rect(ctx, meteor, (GRect) { .origin = { mx, my }, .size = bounds.size });
  }
  
  /*graphics_draw_bitmap_in_rect(ctx, image, (GRect) { .origin = { 80, 60 }, .size = bounds.size });*/
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  /*text_layer_set_text(text_layer, "Select");*/
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  /*text_layer_set_text(text_layer, "Up");*/
  px-=10;
  layer_mark_dirty(layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  /*text_layer_set_text(text_layer, "Down");*/
  px+=10;
  layer_mark_dirty(layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
/*
static void window_load(Window *window) {
}

static void window_unload(Window *window) {
}*/

int main(void) {
  srand(time(NULL));
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  /*window_set_window_handlers(window, (WindowHandlers) {
	  .load = window_load,
    .unload = window_unload,
  });*/
  
  window_stack_push(window, true /* Animated */);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  image = gbitmap_create_with_resource(RESOURCE_ID_SHIP);
  meteor = gbitmap_create_with_resource(RESOURCE_ID_METEOR);
  
  /* acc a timer */
  accel_data_service_subscribe(0, NULL);
  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
  
  app_event_loop();
  accel_data_service_unsubscribe();
  gbitmap_destroy(image);
  gbitmap_destroy(meteor);

  window_destroy(window);
  layer_destroy(layer);
}
