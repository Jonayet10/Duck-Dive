#ifndef __SCENE_H__
#define __SCENE_H__

#include "body.h"
#include "list.h"

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene scene_t;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*force_creator_t)(void *aux);

/**
 * A text object containing all info to be rendered on the scene.
 * Implemented in sdl_wrapper.
 */
typedef struct text text_t;

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @param width the total width of the scene in blocks
 * @param height the total height of the scene in blocks
 *
 * @return the new scene
 */
scene_t *scene_init(size_t width, size_t height);

/**
 * Releases memory allocated for a given scene
 * and all the bodies, force creators, and text it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(scene_t *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
body_t *scene_get_body(scene_t *scene, size_t index);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(scene_t *scene, body_t *body);

/**
 * @deprecated Use body_remove() instead
 *
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(scene_t *scene, size_t index);

/**
 * @deprecated Use scene_add_bodies_force_creator() instead
 * so the scene knows which bodies the force creator depends on
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 * The force creator is registered with a list of bodies it applies to,
 * so it can be removed when any one of the bodies is removed.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param bodies the list of bodies affected by the force creator.
 *   The force creator will be removed if any of these bodies are removed.
 *   This list does not own the bodies, so its freer should be NULL.
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer);

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 * If any bodies are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt);

/**
 * Gets the width of the scene in blocks
 *
 * @param scene the sceen to get the width of
 * @return size_t the width of the scene in blocks
 */
size_t scene_get_width(scene_t *scene);

/**
 * Gets the height of the scene in blocks
 *
 * @param scene the sceen to get the height of
 * @return size_t the height of the scene in blocks
 */
size_t scene_get_height(scene_t *scene);

// /**
//  * Inititalizes a text object with the given information.
//  *
//  * @param text text to add to scene
//  * @param path path to desired font
//  * @param size size of text
//  * @param color color of text
//  * @param position position of the top left corner of text
//  */
// text_t *text_init(char *text, char *path, size_t size, rgb_color_t color,
// vector_t position);

/**
 * Adds text to a scene.
 *
 * @param scene scene to add text to
 * @param text text to add to scene
 */
void scene_add_text(scene_t *scene, text_t *text);

/**
 * Returns number of text objects on a scene
 * @param scene scene to check texts of
 * @param text text object to add to scene
 */
size_t scene_texts(scene_t *scene);

/**
 * Removes text at a given index.
 *
 * @param scene scene to remove text from.
 * @param index the index of the text in the scene (0 indexed)
 */
text_t *scene_get_text(scene_t *scene, size_t index);

/**
 * Removes text at a given index and returns it.
 *
 * @param scene scene to remove text from.
 * @param index the index of the text in the scene (0 indexed)
 *
 * @return the removed text object
 */
text_t *scene_remove_text(scene_t *scene, size_t index);

/**
 * Returns number of text on scene.
 *
 * @param scene
 */
size_t scene_text(scene_t *scene);

#endif // #ifndef __SCENE_H__
