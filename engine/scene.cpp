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

#include <IL/il.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "scene.h"

#define maybe(atr, def) ((atr) ? atr.as_float() : (def))
#define match(tag, func) \
    if (strcmp(tag, trans.name()) == 0) func(trans, scene, group)

#define UNIMPLEMENTED() assert(!"unimplemented")
#define UNREACHABLE()   assert(!"unreachable")

static void sc_draw_rotate (const struct gt * gt)
{
    assert(gt->type == GT_ROTATE);
    glRotatef(gt->angle, gt->p.x, gt->p.y, gt->p.z);
}

static void sc_draw_rotate_anim (const struct gt * gt, unsigned elapsed)
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

static void get_global_catmull_rom_point (float gt, struct Point * pos, struct Point * deriv, const std::vector<struct Point> cp);
static void sc_draw_translate_anim (struct gt * gt, unsigned elapsed)
{
    assert(gt->type == GT_TRANSLATE_ANIM);
    float t = (float) elapsed / (float) gt->time;
    struct Point pos;
    struct Point deriv;
    get_global_catmull_rom_point(t, &pos, &deriv, gt->control_points);
    glTranslatef(pos.x, pos.y, pos.z);
}

static void mult_matrix_vector (const float m[16], const float v[4], float res[4])
{
    for (unsigned j = 0; j < 4; j++) {
        res[j] = 0;
        for (unsigned k = 0; k < 4; k++)
            res[j] += v[k] * m[j * 4 + k];
    }
}

static void get_catmull_rom_point (float t, struct Point p0, struct Point p1, struct Point p2, struct Point p3, struct Point * pos, struct Point * deriv)
{
    // catmull-rom matrix
    const float m[16] = {
        -0.5f, 1.5f,  -1.5f, 0.5f,
        1.0f,  -2.5f, 2.0f,  -0.5f,
        -0.5f, 0.0f,  0.5f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,
    };

#define cenas(f) \
    do { \
        float A[4];                                     \
        const float P[4] = { p0.f, p1.f, p2.f, p3.f };  \
        mult_matrix_vector(m, P, A);                    \
        pos->f = t*t*t*A[0] + t*t*A[1] + t*A[2] + A[3]; \
        deriv->f = 3*t*t*A[0] + 2*t*A[1] + A[2];        \
    } while(0)

    cenas(x);
    cenas(y);
    cenas(z);
}

static void get_global_catmull_rom_point (float gt, struct Point * pos, struct Point * deriv, const std::vector<struct Point> cp)
{
    unsigned POINT_COUNT = cp.size();
    float t = gt * POINT_COUNT; // this is the real global t
    int index = floor(t); // which segment
    t -= index; // where within the segment

    // indices store the points
    int i[4];
    i[0] = (index + POINT_COUNT - 1) % POINT_COUNT;
    i[1] = (i[0] + 1) % POINT_COUNT;
    i[2] = (i[1] + 1) % POINT_COUNT;
    i[3] = (i[2] + 1) % POINT_COUNT;

    get_catmull_rom_point(t, cp[i[0]], cp[i[1]], cp[i[2]], cp[i[3]], pos, deriv);
}

static float norm (struct Point v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static void buildRotMatrix (struct Point x, struct Point y, struct Point z, float m[16])
{
    m[0] = x.x; m[1] = x.y; m[2] = x.z; m[3] = 0;
    m[4] = y.x; m[5] = y.y; m[6] = y.z; m[7] = 0;
    m[8] = z.x; m[9] = z.y; m[10] = z.z; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

static void sc_draw_cm_curve (const struct gt * gt)
{
    glBegin(GL_LINE_LOOP);
    for (unsigned i = 0; i < 100; i++) {
        struct Point pos;
        struct Point deriv;

        get_global_catmull_rom_point(((float) i) / 100, &pos, &deriv, gt->control_points);
        glVertex3f(pos.x, pos.y, pos.z);
    }
    glEnd();
}

static inline float distpp(struct Point n, struct Point p, struct Point c)
{
    float d = -(n.x*p.x + n.y*p.y + n.z*p.z);
    return n.x*c.x + n.y*c.y + n.z*c.z + d;
}

static inline bool is_out (struct Point c, float r, struct Plane plane)
{
    return distpp(plane.n, plane.p, c) + r < 0;
}

static void sc_draw_model (struct scene * scene, const struct frustum * frst, struct model * model)
{
    struct Point P = Point(0, 0, 0);
    bool shouldnt_draw = false
        || is_out(P, 40, frst->far)
        || is_out(P, 40, frst->near)
        || is_out(P, 40, frst->top)
        || is_out(P, 40, frst->bot)
        || is_out(P, 40, frst->left)
        || is_out(P, 40, frst->right);

    if (shouldnt_draw)
        return;

    glPushAttrib(GL_LIGHTING_BIT);
    struct model_vbo mvbo = scene->models[model->fname];
    struct attribs atr = mvbo.attribs[model->id];

#define draw_(T, GL) \
    if (atr.has_ ## T) do { \
        GLfloat color[4] = {atr.T.x, atr.T.y, atr.T.z, 1}; \
        glMaterialfv(GL_FRONT, GL, color);                 \
    } while (0)
    draw_(amb,  GL_AMBIENT);
    draw_(diff, GL_DIFFUSE);
    draw_(emi,  GL_EMISSION);
    draw_(spec, GL_SPECULAR);

    //glMaterialf(GL_FRONT, GL_SHININESS, 128);
#undef draw_


    /* bind and draw the triangles */
    glBindBuffer(GL_ARRAY_BUFFER, mvbo.v_id);
    glVertexPointer(3, GL_FLOAT, 0, NULL);

    /* bind and draw normals */
    glBindBuffer(GL_ARRAY_BUFFER, mvbo.n_id);
    glNormalPointer(GL_FLOAT, 0, 0);

    if (atr.has_text) {
        /* TODO: emissiva a 0 */
        glBindTexture(GL_TEXTURE_2D, atr.text);
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.t_id);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, mvbo.length);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
}

static void sc_draw_groups (struct scene * scene, const struct frustum * frst, std::vector<struct group*> groups, unsigned elapsed, bool draw_curves);
static void sc_draw_group (struct scene * scene, const struct frustum * frst, struct group * group, unsigned elapsed, bool draw_curves)
{
    for (struct gt gt : group->gt) {
        switch (gt.type) {
            case GT_ROTATE:         sc_draw_rotate(&gt); break;
            case GT_ROTATE_ANIM:    sc_draw_rotate_anim(&gt, elapsed); break;
            case GT_SCALE:          sc_draw_scale(&gt); break;
            case GT_TRANSLATE:      sc_draw_translate(&gt); break;
            case GT_TRANSLATE_ANIM: if (draw_curves)
                                        sc_draw_cm_curve(&gt);
                                    sc_draw_translate_anim(&gt, elapsed);
                                    break;
            default: UNREACHABLE();
        }
    }

    for (struct model model : group->models)
        sc_draw_model(scene, frst, &model);

    sc_draw_groups(scene, frst, group->subgroups, elapsed, draw_curves);
}

static void sc_draw_groups (struct scene * scene, const struct frustum * frst, std::vector<struct group*> groups, unsigned elapsed, bool draw_curves)
{
    for (struct group * group : groups) {
        glPushMatrix();
        sc_draw_group(scene, frst, group, elapsed, draw_curves);
        glPopMatrix();
    }
}

static void sc_draw_light (const struct scene * scene, struct light * light, unsigned i)
{
    float w = (light->type == LT_POINT) ?
        1:
        (light->type == LT_DIR) ?
        0:
        (light->type == LT_SPOT) ?
        2:
        0;
    GLfloat cenas[4] = { light->pos.x, light->pos.y, light->pos.z, w, };
    GLfloat colour[4] = { light->color.x, light->color.y, light->color.z, 1, };
    /* Assume `GL_LIGHT[0-7]` were defined sequentially */
    glEnable(GL_LIGHT0 + i);
    glLightfv(GL_LIGHT0 + i, GL_POSITION, cenas);
    glLightfv(GL_LIGHT0 + i, GL_AMBIENT, colour);
    glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, colour);
}

void sc_draw_lights (const struct scene * scene)
{
    unsigned i = 0;
    for (struct light * light : scene->lights)
        sc_draw_light(scene, light, i++);
}

void sc_draw (struct scene * scene, const struct frustum * frst, unsigned elapsed, bool draw_curves, bool draw_lights)
{
    if (draw_lights)
        sc_draw_lights(scene);
    sc_draw_groups(scene, frst, scene->groups, elapsed, draw_curves);
}

static bool sc_load_texture (struct scene * scene, std::string fname, std::map<std::string, unsigned> * texts, unsigned * ret)
{
    if (texts->count(fname))
        return (*ret = (*texts)[fname]), true;

    *ret = 0;

    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    unsigned t = 0;
    ilGenImages(1, &t);
    ilBindImage(t);
    if (!ilLoadImage(fname.c_str()))
        return fprintf(stderr, "Error loading texture `%s` (maybe it's missing?)\n", fname.c_str()), false;

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    glGenTextures(1, ret);
    glBindTexture(GL_TEXTURE_2D, *ret);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    unsigned w = ilGetInteger(IL_IMAGE_WIDTH);
    unsigned h = ilGetInteger(IL_IMAGE_HEIGHT);
    unsigned char * data = ilGetData();
    assert(data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

static struct model sc_load_3d_model (struct scene * scene, struct attribs atr, const char * fname)
{
    struct model_vbo mvbo = {0};

    if (scene->models.count(fname)) {
        mvbo = scene->models[fname];
    } else {
        FILE * inf = fopen(fname, "r");
        assert(inf);
        std::vector<struct Point> vec;
        std::vector<struct Point> norm;
        std::vector<struct Point> tcoords;
        gen_model_read(inf, &vec, &norm, &tcoords);
        fclose(inf);

        mvbo.length = vec.size();
        float * rafar = (float *) calloc(vec.size() * 3, sizeof(float));

        unsigned i = 0;
        for (struct Point p : vec) {
            rafar[i++] = p.x;
            rafar[i++] = p.y;
            rafar[i++] = p.z;
        }

        glGenBuffers(1, &mvbo.v_id);
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.v_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mvbo.length * 3, rafar, GL_STATIC_DRAW);

        i = 0;
        for (struct Point p : norm) {
            rafar[i++] = p.x;
            rafar[i++] = p.y;
            rafar[i++] = p.z;
        }

        glGenBuffers(1, &mvbo.n_id);
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.n_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mvbo.length * 3, rafar, GL_STATIC_DRAW);

        i = 0;
        for (struct Point p : tcoords) {
            rafar[i++] = p.x;
            rafar[i++] = p.y;
        }
        glGenBuffers(1, &mvbo.t_id);
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.t_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mvbo.length * 2, rafar, GL_STATIC_DRAW);

        free(rafar);
    }

    mvbo.attribs.push_back(atr);
    scene->models[fname] = mvbo;

    struct model model;
    model.fname = fname;
    model.id = mvbo.attribs.size() - 1;
    return model;
}

static void sc_load_model (pugi::xml_node node, struct scene * scene, struct group * group, std::map<std::string, unsigned> * texts)
{
    struct attribs atr;

#define has_(T, I) \
    has_ ## T = node.attribute(I "R") || node.attribute(I "G") || node.attribute(I "B")
    atr.has_(amb,  "amb");
    atr.has_(diff, "diff");
    atr.has_(spec, "spec");
    atr.has_(emi,  "emi");
#undef has_

#define read_(T, I) \
    if (atr.has_ ## T) do { \
        atr.T.x = maybe(node.attribute(I "R"), 0); \
        atr.T.y = maybe(node.attribute(I "G"), 0); \
        atr.T.z = maybe(node.attribute(I "B"), 0); \
    } while (0)
    read_(amb,  "amb");
    read_(diff, "diff");
    read_(spec, "spec");
    read_(emi,  "emi");
#undef read_

    atr.has_text = node.attribute("texture")
        && sc_load_texture(scene, node.attribute("texture").value(), texts, &atr.text);

    const char * fname = node.attribute("FILE").value();
    struct model model = sc_load_3d_model(scene, atr, fname);
    group->models.push_back(model);
}

static void sc_load_models (pugi::xml_node node, struct scene * scene, struct group * group, std::map<std::string, unsigned> * texts)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling())
        if (strcmp("model", trans.name()) == 0)
            sc_load_model(trans, scene, group, texts);
}

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

static struct Point sc_load_translate_control_point (pugi::xml_node node)
{
    return Point(
            maybe(node.attribute("X"), 0),
            maybe(node.attribute("Y"), 0),
            maybe(node.attribute("Z"), 0)
            );
}

static void sc_load_translate (pugi::xml_node node, struct scene * scene, struct group * group)
{
    bool is_anim = node.attribute("TIME");

    struct gt translate;

    translate.type = (is_anim) ?
        GT_TRANSLATE_ANIM:
        GT_TRANSLATE;

    if (is_anim) {
        translate.time = node.attribute("TIME").as_int() * 1000; /* ms */
        for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling())
            if (strcmp("point", trans.name()) == 0)
                translate.control_points.push_back(sc_load_translate_control_point(trans));
        assert(translate.control_points.size() >= 4);
    } else {
        translate.p.x = maybe(node.attribute("X"), 0);
        translate.p.y = maybe(node.attribute("Y"), 0);
        translate.p.z = maybe(node.attribute("Z"), 0);
    }

    group->gt.push_back(translate);
}

static void sc_load_group (pugi::xml_node node, struct scene * scene, struct group * group, std::map<std::string, unsigned> * texts)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling()) {
        match("translate", sc_load_translate);
        else match("rotate", sc_load_rotate);
        else match("scale", sc_load_scale);
        else if (strcmp("models", trans.name()) == 0) {
            sc_load_models(trans, scene, group, texts);
        } else if (strcmp("group", trans.name()) == 0) {
            struct group * subgroup = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, subgroup, texts);
            group->subgroups.push_back(subgroup);
        }
    }
}

static void sc_load_light (pugi::xml_node node, struct scene * scene, unsigned i)
{
    if (!node.attribute("TYPE"))
        return;

    struct light * light = (struct light*) calloc(1, sizeof(struct light));

    light->color = Point(
            maybe(node.attribute("R"), 0),
            maybe(node.attribute("G"), 0),
            maybe(node.attribute("B"), 0)
            );

    light->pos = Point(
            maybe(node.attribute("X"), 0),
            maybe(node.attribute("Y"), 0),
            maybe(node.attribute("Z"), 0)
            );

    light->type = (strcmp("POINT", node.attribute("TYPE").value()) == 0) ?
        LT_POINT:
        (strcmp("DIR", node.attribute("TYPE").value()) == 0) ?
        LT_DIR:
        (strcmp("SPOT", node.attribute("TYPE").value()) == 0) ?
        LT_SPOT:
        LT_POINT;

    scene->lights.push_back(light);
}

static void sc_load_lights (pugi::xml_node node, struct scene * scene)
{
    unsigned i = 0;
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling())
        if (strcmp("light", trans.name()) == 0)
            sc_load_light(trans, scene, i++);
}

bool sc_load_file (const char * path, struct scene * scene)
{
    pugi::xml_document doc;
    if (!doc.load_file(path))
        return false;

    /*
     * There's no need to load the same texture more than once, so we
     * the textures loaded so far here
     */
    std::map<std::string, unsigned> texts;

    pugi::xml_node models = doc.child("scene");
    for (pugi::xml_node trans = models.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("group", trans.name()) == 0) {
            struct group * group = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, group, &texts);
            scene->groups.push_back(group);
        } else if (strcmp("lights", trans.name()) == 0) {
            sc_load_lights(trans, scene);
        }
    }

    return true;
}

struct Plane Plane (struct Point p, struct Point n)
{
	struct Plane ret;
	ret.p = p;
	ret.n = n;
	return ret;
}
