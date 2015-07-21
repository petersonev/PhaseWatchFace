#include <pebble.h>
#include <math.h>

#include "my_math.h"
  
  //144x168

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_weather_date_layer;
static Layer *ellipse_layer;

static unsigned factorial(unsigned num) {
  if (num==1) {
    return 1;
  } else {
    return num*factorial(num-1);
  }
}

// static double pow1(double base, double exp) {
//   double val = 1;
//   for(int i=0; i<exp; i++) {
//     val*=val;
//   }
//   return val;
// }

// static double arccos(double num) {
//   if (num>1 || num<-1) {
//     return -99;
//   }
//   double val = M_PI/2;
//   for(double n = 0; n<5; n++) {
//     val -= 0.5*n/(2*n-1)/factorial(n)*pow(num, 2*5+1);
//   }
//   return val;
// }

static double get_declin() {
  int month = 7;
  int day = 17;
//   double latitude;
//   int hour;
//   int minute;
  
//   double a_declination;
  

//   latitude = 28.715545677;
//   hour = 1;
//   minute = 17;
  int day_num = 30*(month-1)+day;
  
//   double a_hour = (double)hour+(double)minute/60.0;
  
//   a_declination = a_hour + latitude;
  double a_declination = 23.45*sin(360.0/365.0*(284+day_num)*M_PI/180.0);
  
  
  return a_declination;
}

static double get_hour_angle() {
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int hour = tick_time->tm_hour;
  int min = tick_time->tm_min;
  
  double hours = hour + 1.0/60*min; // number of hours
  double month = 7;
  double day = 17;
  double timezone = -5;
  double longitude = -81.14248081;
  
  double LSTM = 15*timezone;
  double B = 360/365*((30*(month-1) + day)-81);
  double EoT = 9.87*sin(2*B*M_PI/180)-7.53*cos(B*M_PI/180)-1.5*sin(B*M_PI/180);
  double TC = 4*(longitude - LSTM) + EoT;
  double LST = hours + TC/60;
  double hour_angle = 15*(LST-12);
  double hour_angle_adj;
  if (hour_angle < -180) {
    hour_angle_adj = 360+hour_angle;
  } else {
    hour_angle_adj = hour_angle;
  }
  
  
//   double eq_time = 
//   double true_solar = mod(hours/24*1440 + eq_time + 4*longitude - 60*timezone, 1440);
//   double hour_angle = (abs(true_solar/4)-180)*abs(true_solar)/true_solar;
  
  return hour_angle_adj;
}

static double hour_angle_to_el(double hour_angle) {
  
  double latitude = 28.71554525;
  
  double declination = get_declin();
  double temp0 = sin(latitude*M_PI/180)*sin(declination*M_PI/180) + cos(latitude*M_PI/180)*cos(declination*M_PI/180)*cos(hour_angle*M_PI/180);
//   float temp1 = 0;
//   if (temp0<1 && temp0>-1) {
//     temp1 = temp0;
//   }
  float zenith = my_acos(temp0)*180/M_PI;
  return 90-zenith;
}

static void path_layer_update_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorRed);
  
  double a_x = 120;
  double a_y = 100;
  
  for (double t = 0; t < 2.0*M_PI; t += .01) {
    
    int x = a_x/2 * (sin(t) + 1) + (144-a_x)/2;
    int y = a_y/2 * (cos(t) + 1) + 60;

    graphics_draw_pixel(ctx, GPoint(x, y));
  }
  
//   graphics_context_set_stroke_color(ctx, GColorBlack);
//   double angle = hour_angle_to_el(get_hour_angle()) * PI/180;
//   int x1 = a_x/2 * (sin(angle) + 1) + (144-a_x)/2;
//   int y1 = a_y/2 * (cos(angle) + 1) + 60;
//   graphics_draw_pixel(ctx, GPoint(x1, y1));
  
}

// static double get_sun_elevation() {
//   double pi = 3.14159265;
  
// //   int n = 31+29+20;
// //   int n = 1;
  
// //   double declination = 23.45*(pi/180)*sin(2*pi*((284+n)/36.25));
  
//   int month;
//   int day;
//   double latitude;
//   int hour;
//   int minute;
  
//   int day_num;
  
//   double a_declination;
//   double a_hour;
  
  
//   month = 7;
//   day = 17;
//   latitude = 28.715545677;
//   hour = 1;
//   minute = 17;
//   day_num = 30*(month-1)+day;
  
//   a_hour = (double)hour+(double)minute/60.0;
  
//   a_declination = a_hour + latitude;
//   a_declination = 23.45*sin(360.0/365.0*(284+day_num)*pi/180.0);
  
  
//   return a_declination;
// }

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "00:00";
  
  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  s_time_layer = text_layer_create(GRect(0, 0, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  s_weather_date_layer = text_layer_create(GRect(0, 70, 144, 25));
  text_layer_set_background_color(s_weather_date_layer, GColorClear);
  text_layer_set_text_color(s_weather_date_layer, GColorBlack);
  text_layer_set_font(s_weather_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_weather_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_date_layer, "");
  
//   double elevation = hour_angle_to_el(0);
  
  double elevation = hour_angle_to_el(get_hour_angle());
//   double elevation = hour_an;
  static char buffer[60];
  snprintf(buffer, sizeof(buffer), "%d.%d", (int)(elevation),abs((int)(elevation*1000) % 1000));
  
  text_layer_set_text(s_weather_date_layer, buffer);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_date_layer));
  
  ellipse_layer = layer_create(layer_get_frame(window_get_root_layer(window)));
  layer_set_update_proc(ellipse_layer, path_layer_update_callback);
  layer_add_child(window_get_root_layer(window), ellipse_layer);
  
  
//   ellipse_path = gpath_create(ellipse_path_points);
  
  
}  

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  
  text_layer_destroy(s_weather_date_layer);
}

static void init() {
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}