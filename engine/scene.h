#ifndef _SCENE_H
#define _SCENE_H

#include "../generator/generators.h"

#include "pugixml/pugixml.hpp"

#include <map>
#include <string>
#include <vector>

enum gt_type {
    GT_ROTATE,
    GT_ROTATE_ANIM,
    GT_SCALE,
    GT_TRANSLATE,
    GT_TRANSLATE_ANIM,
};

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

struct model {
    std::string fname;
    unsigned id;
};

struct group {
    std::vector<struct gt>     gt;
    std::vector<struct model>  models;
    std::vector<struct group*> subgroups;
};

struct textura_ou_merdas {
    bool has_amb;
    bool has_diff;
    bool has_emi;
    bool has_spec;
    bool has_text;
    std::string text;
    struct Point amb;
    struct Point diff;
    struct Point emi;
    struct Point spec;
};

struct model_vbo {
    /** Verteces VBO ID */
    GLuint v_id;

    /** Normals VBO ID */
    GLuint n_id;

    /** Vertex count */
    size_t length;

    std::vector<struct textura_ou_merdas> vector_de_textura_ou_merdas;
};

enum lt_type {
    LT_POINT,
    LT_SPOT,
    LT_DIR,
};

struct light {
    enum lt_type type;
    struct Point pos;
};

struct scene {
    std::vector<struct light*>          lights;
    std::vector<struct group*>          groups;
    std::map<std::string, struct model_vbo> models;
};

bool sc_load_file (const char * path, struct scene * scene);
void sc_draw      (struct scene * scene, unsigned int elapsed, bool draw_curves);

#endif /* _SCENE_H */
