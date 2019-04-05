#include "pugixml/pugixml.hpp"
#include "../generator/generators.h"

#include <string.h>
#include <assert.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include "scene.h"

#define match(tag, func) \
    if (strcmp(tag, trans.name()) == 0) func(trans, scene, group)

#define UNIMPLEMENTED() assert(!"unimplemented")
#define UNREACHABLE()   assert(!"unreachable")

static void sc_draw_rotate (const struct gt * gt)
{
    assert(gt->type == GT_ROTATE);
    glRotatef(gt->angle, gt->p.x, gt->p.y, gt->p.z);
}

static void sc_draw_rotate_anim (const struct gt * gt, unsigned int elapsed)
{
    assert(gt->type == GT_ROTATE_ANIM);
    float angle = (360.0 * elapsed) / gt->time;
    glRotatef(angle, gt->p.x, gt->p.y, gt->p.z);
}

static void sc_draw_scale (const struct gt * gt)
{
    assert(gt->type == GT_SCALE);
    glScalef(gt->p.x, gt->p.y, gt->p.z);
}

static void sc_draw_translate (const struct gt * gt)
{
    assert(gt->type == GT_TRANSLATE);
    glTranslatef(gt->p.x, gt->p.y, gt->p.z);
}

static void sc_draw_translate_anim (const struct gt * gt, unsigned int elapsed)
{
    assert(gt->type == GT_TRANSLATE_ANIM);
    UNIMPLEMENTED();
}

static void sc_draw_group (struct scene * scene, struct group * group, unsigned int elapsed)
{
    for (struct gt gt : group->gt) {
        switch (gt.type) {
            case GT_ROTATE:         sc_draw_rotate(&gt);                  break;
            case GT_ROTATE_ANIM:    sc_draw_rotate_anim(&gt, elapsed);    break;
            case GT_SCALE:          sc_draw_scale(&gt);                   break;
            case GT_TRANSLATE:      sc_draw_translate(&gt);               break;
            case GT_TRANSLATE_ANIM: sc_draw_translate_anim(&gt, elapsed); break;
            default: UNREACHABLE();
        }
    }

    for (std::string mname : group->models) {
        struct model model = scene->models[mname];
        glBindBuffer(GL_ARRAY_BUFFER, model.id);
        glVertexPointer(3, GL_FLOAT, 0, NULL);
        glDrawArrays(GL_TRIANGLES, 0, model.length);
    }

    for (auto subgroup : group->subgroups) {
        glPushMatrix();
        sc_draw_group(scene, subgroup, elapsed);
        glPopMatrix();
    }
}

void sc_draw (struct scene * scene, unsigned int elapsed)
{
    for (struct group * group : scene->groups) {
        glPushMatrix();
        sc_draw_group(scene, group, elapsed);
        glPopMatrix();
    }
}

static void sc_load_model (const char * fname, struct model * model)
{
    FILE * inf = fopen(fname, "r");
    std::vector<struct Point> vec;
    gen_model_read(inf, &vec);
    fclose(inf);

    float * rafar = (float *) calloc(vec.size() * 3, sizeof(float));
    unsigned i = 0;
    for (struct Point p : vec) {
        rafar[i++] = p.x;
        rafar[i++] = p.y;
        rafar[i++] = p.z;
    }

    model->length = vec.size();

    glGenBuffers(1, &model->id);
    glBindBuffer(GL_ARRAY_BUFFER, model->id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->length * 3, rafar, GL_STATIC_DRAW);
    free(rafar);
}

static void sc_load_models (pugi::xml_node node, struct scene * scene, struct group * group)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("model", trans.name()) == 0) {
            const char * fname = trans.attribute("FILE").value();

            if (!scene->models.count(fname)) {
                struct model model;
                sc_load_model(fname, &model);
                scene->models[fname] = model;
            }

            group->models.push_back(fname);
        }
    }
}

#define maybe(atr, def) ((atr) ? atr.as_float() : (def))

static void sc_load_rotate (pugi::xml_node node, struct scene * scene, struct group * group)
{
    bool is_static = node.attribute("ANGLE");
    bool is_anim   = node.attribute("TIME");

    assert(is_static || is_anim);

    struct gt rotate;

    if (is_static) {
        rotate.angle = node.attribute("ANGLE").as_float();
        rotate.type = GT_ROTATE;
    } else {
        rotate.time = node.attribute("TIME").as_int() * 1000;
        rotate.type = GT_ROTATE_ANIM;
    }

    rotate.p.x = maybe(node.attribute("X"), 0);
    rotate.p.y = maybe(node.attribute("Y"), 0);
    rotate.p.z = maybe(node.attribute("Z"), 0);

    group->gt.push_back(rotate);
}

static void sc_load_scale (pugi::xml_node node, struct scene * scene, struct group * group)
{
    struct gt scale;
    scale.type = GT_SCALE;
    scale.p.x = maybe(node.attribute("X"), 1);
    scale.p.y = maybe(node.attribute("Y"), 1);
    scale.p.z = maybe(node.attribute("Z"), 1);
    group->gt.push_back(scale);
}

static void sc_load_translate (pugi::xml_node node, struct scene * scene, struct group * group)
{
    bool is_anim = node.attribute("TIME");

    struct gt translate;

    translate.type = (is_anim) ?
        GT_TRANSLATE_ANIM:
        GT_TRANSLATE;

    if (is_anim) {
        UNIMPLEMENTED();
        translate.time = node.attribute("TIME").as_int() * 1000; /* ms */
    } else {
        translate.p.x = maybe(node.attribute("X"), 0);
        translate.p.y = maybe(node.attribute("Y"), 0);
        translate.p.z = maybe(node.attribute("Z"), 0);
    }

    group->gt.push_back(translate);
}

static void sc_load_group (pugi::xml_node node, struct scene * scene, struct group * group)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling()) {
        match("translate", sc_load_translate);
        else match("rotate", sc_load_rotate);
        else match("scale", sc_load_scale);
        else match("models", sc_load_models);
        else if (strcmp("group", trans.name()) == 0) {
            struct group * subgroup = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, subgroup);
            group->subgroups.push_back(subgroup);
        }
    }
}

bool sc_load_file (const char * path, struct scene * scene)
{
    pugi::xml_document doc;
    if (!doc.load_file(path))
        return false;

    pugi::xml_node models = doc.child("scene");

    for (pugi::xml_node trans = models.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("group", trans.name()) == 0) {
            struct group * group = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, group);
            scene->groups.push_back(group);
        }
    }

    return true;
}
