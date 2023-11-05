#include "sdl_wrapper.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "Duck Dive";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

struct text {
  SDL_Surface *surface;
  SDL_Texture *texture;
  vector_t position;
};

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return y_scale < x_scale ? y_scale : x_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  case SDL_BUTTON_LEFT:
    return BUTTON_LEFT;
  case SDLK_ESCAPE:
    return ESCAPE_KEY;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max)),
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
}

bool sdl_is_done(void *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      if (key_handler == NULL) {
        break;
      }
      char key = get_keycode(event->key.keysym.sym);
      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type;
      if (event->type == SDL_MOUSEBUTTONDOWN) {
        type = MOUSE_CLICK;
      } else if (event->type == SDL_MOUSEBUTTONUP) {
        type = MOUSE_RELEASE;
      } else if (event->type == SDL_KEYDOWN) {
        type = KEY_PRESSED;
      } else {
        type = KEY_RELEASED;
      }
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(state, key, type, held_time,
                  (vector_t){event->button.x, event->button.y});
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

text_t *sdl_create_text(char *text, char *font, size_t size, rgb_color_t color,
                        vector_t loc) {
  TTF_Font *text_font = TTF_OpenFont(font, size);
  SDL_Color text_color = {255 * color.r, 255 * color.g, 255 * color.b};
  SDL_Surface *surface = TTF_RenderText_Solid(text_font, text, text_color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  vector_t screen_loc = get_window_position(loc, get_window_center());
  vector_t centered = {screen_loc.x - surface->w / 2,
                       screen_loc.y - surface->h / 2};

  text_t *returned = malloc(sizeof(text_t));
  returned->surface = surface;
  returned->texture = texture;
  returned->position = centered;

  TTF_CloseFont(text_font);
  return returned;
}

void sdl_draw_text(text_t *text) {
  SDL_Rect text_box = {text->position.x, text->position.y, text->surface->w,
                       text->surface->h};
  SDL_RenderCopy(renderer, text->texture, NULL, &text_box);
}

void sdl_swap_text(text_t *text, char *string, char *font, size_t size,
                   rgb_color_t color) {
  SDL_FreeSurface(text->surface);
  SDL_DestroyTexture(text->texture);
  TTF_Font *text_font = TTF_OpenFont(font, size);
  SDL_Color text_color = {255 * color.r, 255 * color.g, 255 * color.b};
  SDL_Surface *surface = TTF_RenderText_Solid(text_font, string, text_color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  text->surface = surface;
  text->texture = texture;
}

void sdl_free_text(text_t *text) {
  SDL_FreeSurface(text->surface);
  SDL_DestroyTexture(text->texture);
  free(text);
}

bool button_is_clicked(list_t *shape, vector_t click) {
  // Get bounds
  vector_t min = {INFINITY, INFINITY};
  vector_t max = {-INFINITY, -INFINITY};
  vector_t *v;
  for (size_t i = 0; i < list_size(shape); i++) {
    v = list_get(shape, i);
    if (v->x < min.x) {
      min.x = v->x;
    }
    if (v->x > max.x) {
      max.x = v->x;
    }
    if (v->y < min.y) {
      min.y = v->y;
    }
    if (v->y > max.y) {
      max.y = v->y;
    }
  }
  vector_t window_center = get_window_center();
  vector_t max_pixel = get_window_position(max, window_center);
  vector_t min_pixel = get_window_position(min, window_center);
  if (click.x > min_pixel.x && click.x < max_pixel.x && click.y < min_pixel.y &&
      click.y > max_pixel.y) {
    return true;
  }
  return false;
}

void sdl_draw_sprite(body_t *sprite) {

  SDL_Texture *texture = body_get_texture(sprite);
  list_t *boundary = body_get_shape(sprite);
  vector_t *origin = list_get(boundary, 1);
  vector_t *bounds = list_get(boundary, 3);
  vector_t window_center = get_window_center();
  vector_t origin_pixel = get_window_position(*origin, window_center);
  vector_t bounds_pixel = get_window_position(*bounds, window_center);
  SDL_Rect *rect = malloc(sizeof(SDL_Rect));
  *rect = (SDL_Rect){.x = origin_pixel.x,
                     .y = origin_pixel.y,
                     .w = bounds_pixel.x - origin_pixel.x,
                     .h = bounds_pixel.y - origin_pixel.y};
  SDL_RenderCopyEx(
      renderer, texture, NULL, rect, body_get_angle(sprite) * 180 / M_PI, NULL,
      body_get_flipped(sprite) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  list_free(boundary);
  free(rect);
}

void sdl_show() {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    if (body_get_texture(body)) {
      sdl_draw_sprite(body);
    } else {
      list_t *shape = body_get_shape(body);
      sdl_draw_polygon(shape, body_get_color(body));
      list_free(shape);
    }
  }
  size_t text_count = scene_texts(scene);
  for (size_t i = 0; i < text_count; i++) {
    text_t *text = scene_get_text(scene, i);
    sdl_draw_text(text);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

SDL_Texture *sdl_create_texture(char *texture_path) {
  SDL_Texture *texture = NULL;
  SDL_Surface *surface = NULL;
  IMG_Init(IMG_INIT_PNG);
  surface = IMG_Load(texture_path);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  return texture;
}
