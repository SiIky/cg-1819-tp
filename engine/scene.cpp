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

#define _USE_MATH_DEFINES
#include <math.h>

#include "scene.h"

#define match(tag, func) \
    if (strcmp(tag, trans.name()) == 0) func(trans, scene, group)

#define UNIMPLEMENTED() assert(!"unimplemented")
#define UNREACHABLE()   assert(!"unreachable")

static void get_global_catmull_rom_point (float gt, struct Point * pos, struct Point * deriv, const std::vector<struct Point> cp);

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

static void sc_draw_translate_anim (struct gt * gt, unsigned int elapsed)
{
    assert(gt->type == GT_TRANSLATE_ANIM);
    float t = (float) elapsed / (float) gt->time;

    struct Point pos;
    struct Point deriv;

    get_global_catmull_rom_point(t, &pos, &deriv, gt->control_points);
    glTranslatef(pos.x,pos.y,pos.z);
}

static void mult_matrix_vector (float *m, float *v, float *res)
{
    for (int j = 0; j < 4; j++) {
        res[j] = 0;
        for (int k = 0; k < 4; k++) {
            res[j] += v[k] * m[j * 4 + k];
        }
    }
}

static void get_catmull_rom_point (float t, struct Point p0, struct Point p1, struct Point p2, struct Point p3, struct Point * pos, struct Point * deriv)
{
    // catmull-rom matrix
    float m[16] = {
        -0.5f, 1.5f,  -1.5f, 0.5f,
        1.0f,  -2.5f, 2.0f,  -0.5f,
        -0.5f, 0.0f,  0.5f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,
    };

#define cenas(f) \
    do { \
        float A[4];                                   \
        float P[4] = { p0.f, p1.f, p2.f, p3.f };      \
        mult_matrix_vector(m, P, A);                  \
        pos->f = t*t*t*A[0] + t*t*A[1]+t*A[2] + A[3]; \
        deriv->f = 3*t*t*A[0] + 2*t*A[1]+A[2];        \
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

static struct Point cross (struct Point a, struct Point b)
{
    return Point(
            a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x
            );
}

static float length (struct Point v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static struct Point normalize (struct Point p)
{
    float l = length(p);
    return Point(p.x / l, p.y / l, p.z / l);
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
    glBegin(GL_LINE_LOOP); {
        for (float i = 0; i < 1; i += 0.01) {
            struct Point pos;
            struct Point deriv;

            get_global_catmull_rom_point(i, &pos, &deriv, gt->control_points);
            glVertex3f(pos.x, pos.y, pos.z);
        }
    } glEnd();
}

static void sc_draw_group (struct scene * scene, struct group * group, unsigned int elapsed, bool draw_curves)
{
    for (struct gt gt : group->gt) {
        switch (gt.type) {
            case GT_ROTATE:         sc_draw_rotate(&gt);                  break;
            case GT_ROTATE_ANIM:    sc_draw_rotate_anim(&gt, elapsed);    break;
            case GT_SCALE:          sc_draw_scale(&gt);                   break;
            case GT_TRANSLATE:      sc_draw_translate(&gt);               break;
            case GT_TRANSLATE_ANIM:
                                    if (draw_curves) sc_draw_cm_curve(&gt);
                                    sc_draw_translate_anim(&gt, elapsed);
                                    break;
            default: UNREACHABLE();
        }
    }

    for (struct model model : group->models) {
        struct model_vbo mvbo = scene->models[model.fname];

        /* bind and draw the triangles */
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.v_id);
        glVertexPointer(3, GL_FLOAT, 0, NULL);

        /* bind and draw normals */
        glBindBuffer(GL_ARRAY_BUFFER, mvbo.n_id);
        glNormalPointer(GL_FLOAT, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, mvbo.length);
    }

    for (auto subgroup : group->subgroups) {
        glPushMatrix();
        sc_draw_group(scene, subgroup, elapsed, draw_curves);
        glPopMatrix();
    }
}

static void sc_draw_light (struct scene * scene, struct light * light, unsigned i)
{
    float w = (light->type == LT_POINT) ?
        1:
        (light->type == LT_DIR) ?
        0:
        (light->type == LT_SPOT) ?
        2:
        0;
    GLfloat cenas[4] = { light->pos.x, light->pos.y, light->pos.z, w, };
    GLfloat colour[4] = { 1, 1, 1, 0 };
    glLightfv(GL_LIGHT0 + i, GL_POSITION, cenas);
    glLightfv(GL_LIGHT0 + i, GL_AMBIENT, colour);
    glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, colour);
}

void sc_draw (struct scene * scene, unsigned int elapsed, bool draw_curves)
{
    unsigned i = 0;
    for (struct light * light : scene->lights) {
        sc_draw_light(scene, light, i++);
    }

    for (struct group * group : scene->groups) {
        glPushMatrix();
        sc_draw_group(scene, group, elapsed, draw_curves);
        glPopMatrix();
    }
}

#define maybe(atr, def) ((atr) ? atr.as_float() : (def))

static void sc_load_model (pugi::xml_node node, struct scene * scene, struct group * group)
{
	struct textura_ou_merdas tom;

#define has_(T, I) \
	has_ ## T = node.attribute(I "R") || node.attribute(I "G") || node.attribute(I "B")
	tom.has_(amb, "amb");
	tom.has_(diff, "diff");
	tom.has_(spec, "spec");
	tom.has_(emi, "emi");

#define read_(T, I) \
	if (tom.has_ ## T) do { \
		tom.T.x = maybe(node.attribute(I "R"), 0); \
		tom.T.y = maybe(node.attribute(I "G"), 0); \
		tom.T.z = maybe(node.attribute(I "B"), 0); \
	} while (0)
	read_(amb, "amb");
	read_(diff, "diff");
	read_(spec, "spec");
	read_(emi, "emi");

	if (node.attribute("texture")) {
		tom.has_text = true;
		tom.text = node.attribute("texture").value();
	}

	const char * fname = node.attribute("FILE").value();
	struct model_vbo mvbo;

	if (scene->models.count(fname)) {
		mvbo = scene->models[fname];
	} else {
		FILE * inf = fopen(fname, "r");
		assert(inf);
		std::vector<struct Point> vec;
		std::vector<struct Point> norm;
		gen_model_read(inf, &vec, &norm);
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

		free(rafar);
	}

	mvbo.vector_de_textura_ou_merdas.push_back(tom);
	scene->models[fname] = mvbo;

	struct model model;
	model.fname = fname;
	model.id = mvbo.vector_de_textura_ou_merdas.size() - 1;
	group->models.push_back(model);
}

static void sc_load_models (pugi::xml_node node, struct scene * scene, struct group * group)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("model", trans.name()) == 0) {
            sc_load_model(trans, scene, group);
        }
    }
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

static void sc_load_light (pugi::xml_node node, struct scene * scene, unsigned i)
{
    if (!node.attribute("TYPE"))
        return;

    struct light * light = (struct light*) calloc(1, sizeof(struct light));
    float x = maybe(node.attribute("X"), 0);
    float y = maybe(node.attribute("Y"), 0);
    float z = maybe(node.attribute("Z"), 0);

    light->pos = Point(x, y, z);
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

    pugi::xml_node models = doc.child("scene");

    for (pugi::xml_node trans = models.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("group", trans.name()) == 0) {
            struct group * group = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, group);
            scene->groups.push_back(group);
        } else if (strcmp("lights", trans.name()) == 0) {
            sc_load_lights(trans, scene);
        }
    }

    return true;
}
