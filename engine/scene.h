#ifndef _SCENE_H
#define _SCENE_H

#include "../generator/generators.h"

#include "pugixml/pugixml.hpp"

#include <map>
#include <string>
#include <vector>

/**
 * Geometric Transformation type
 */
enum gt_type {
    GT_ROTATE,
    GT_ROTATE_ANIM,
    GT_SCALE,
    GT_TRANSLATE,
    GT_TRANSLATE_ANIM,
};

/**
 * Geometric Transformation
 */
struct gt {
    /**
     * GT_ROTATE: axis to rotate around
     * GT_TRANSLATE: translation offset
     * GT_SCALE: scaling scalars
     */
    struct Point p;

    /** GT_ROTATE: angle to rotate */
    float angle;

    /**
     * GT_ROTATE_ANIM: time in msecs for a full 360 rotation
     * GT_TRANSLATE_ANIM: time in msecs to execute the full animation
     */
    unsigned time;

    /** GT_TRANSLATE_ANIM: Catmull-Rom Control Points */
    std::vector<struct Point> control_points;

    /** what kind of Geometric Transformation? */
    enum gt_type type;
};

/**
 * An instance of a model
 */
struct model {
    /* TODO: remove all the strings! */
    std::string fname; /*< Key to this model's IDs */
    unsigned id;       /*< Index to this model's attributes */
    float mm[4][4];
};

/**
 * A group of objects
 */
struct group {
    std::vector<struct gt> gt;            /*< Geometric Transformations */
    std::vector<struct model> models;     /*< Model instances */
    std::vector<struct group*> subgroups; /*< Subgroups */
};

/**
 * Attributes of an instance of a model
 */
struct attribs {
    unsigned char has_amb  : 1; /*< Has ambient light? */
    unsigned char has_diff : 1; /*< Has diffuse light? */
    unsigned char has_emi  : 1; /*< Has emissive light? */
    unsigned char has_spec : 1; /*< Has specular light? */
    unsigned char has_text : 1; /*< Has a texture? */

    unsigned text;     /*< Texture ID */
    struct Point amb;  /*< Ambient Light */
    struct Point diff; /*< Diffuse Light */
    struct Point emi;  /*< Emissive Light */
    struct Point spec; /*< Specular Light */
};

/**
 * A model
 */
struct model_vbo {
    unsigned v_id; /*< Verteces VBO ID */
    unsigned n_id; /*< Normals VBO ID */
    unsigned t_id; /*< Texture coordinates buffer ID */
    size_t length; /*< Vertex count */

    /** Vector with the attributes of every instance of this model */
    std::vector<struct attribs> attribs;
};

/**
 * Static Light type
 */
enum lt_type {
    LT_POINT, /*< Positional Light */
    LT_SPOT,  /*< Spotlight */
    LT_DIR,   /*< Directional Light */
};

/**
 * Static light
 */
struct light {
    enum lt_type type;  /*< The type of light */
    struct Point color; /*< Its color */
    struct Point pos;   /*< Its position */
};

/**
 * The scene type. It contains all the info necessary to draw a scene
 */
struct scene {
    /** Static lights */
    std::vector<struct light*> lights;

    /** Groups of objects */
    std::vector<struct group*> groups;

    /** Models data */
    std::map<std::string, struct model_vbo> models;
};

struct Plane {
	struct Point p;
	struct Point n;
};

struct frustum {
    struct Plane top;
    struct Plane bot;
    struct Plane far;
    struct Plane near;
    struct Plane left;
    struct Plane right;
};

/**
 * @brief Load a scene file
 * @param path The path to the file
 * @param[out] scene Where to save loaded data
 * @returns `true` if successfully loaded the scene file
 */
bool sc_load_file (const char * path, struct scene * scene);

/**
 * @brief Draw a scene
 * @param scene The scene
 * @param elapsed Number of ms since program start
 * @param draw_curves Draw Catmull-Rom curves?
 * @param draw_ligts Draw static lights?
 */
void sc_draw (struct scene * scene, struct frustum * frst, unsigned elapsed, bool draw_curves, bool draw_ligts);

/**
 * @brief Draw a scene's static lights
 * @param scene The scene
 */
void sc_draw_lights (const struct scene * scene);

struct Plane Plane (struct Point p, struct Point n);

#endif /* _SCENE_H */
