#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
  BUTTON_LEFT = 6,
  ESCAPE_KEY = 7,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum {
  KEY_PRESSED,
  KEY_RELEASED,
  MOUSE_CLICK,
  MOUSE_RELEASE
} key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 * @param click location of mouse click
 */
typedef void (*key_handler_t)(void *state, char key, key_event_type_t type,
                              double held_time, vector_t click);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

/**
 * Inititalizes a text object with the given information.
 *
 * @param text text to add to scene
 * @param font path to desired font
 * @param size size of text
 * @param color color of text
 * @param position position of the top left corner of text
 */
text_t *sdl_create_text(char *text, char *font, size_t size, rgb_color_t color,
                        vector_t loc);

/**
 * Draws text on a scene.
 */
void sdl_draw_text(text_t *text);

/**
 * Swaps texture(text) being displayed on a scene at same location.
 * @param text text struct being rendered on a scene to swap texture of.
 * @param string string to create new texture from.
 * @param font path to desired font
 * @param size size of text
 * @param color color of text
 */
void sdl_swap_text(text_t *text, char *string, char *font, size_t size,
                   rgb_color_t color);

/**
 * Frees text struct.
 */
void sdl_free_text(text_t *text);

/**
 * Draws a sprite from the coordinates and texture created in the body.
 *
 * @param sprite the body to be drawn
 */
void sdl_draw_sprite(body_t *sprite);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies and text in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), sdl_draw_text(),
 * and sdl_show() so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Checks if click occured within the bounds of an on-screen button.
 * Bounds are taken to be a rectangle enclosing the button.
 *
 * @param button shape of body on screen acting as a button.
 * @param click location of click in window/screen coordinates.
 * @return true if click was in bounds of button, else false.
 */
bool button_is_clicked(list_t *shape, vector_t click);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Creates an SDL texture from a filepath
 *
 * @param texture_path the path to the PNG texture
 *
 * @return an SDL_Texture generated from this PNG file
 */
SDL_Texture *sdl_create_texture(char *texture_path);

#endif // #ifndef __SDL_WRAPPER_H__
