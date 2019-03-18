#include "scene.h"

#include "pugixml/pugixml.hpp"
#include "../generator/generators.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <string.h>
#include <assert.h>


#define match(tag, func) \
	if (strcmp(tag, trans.name()) == 0) func(trans, scene, group)

static void sc_draw_group (struct scene * scene, struct group * group)
{
	for (struct gt gt : group->gt) {
		switch (gt.type) {
			case GT_TRANSLATE:
				glTranslatef(gt.p.x, gt.p.y, gt.p.z);
				break;
			case GT_ROTATE:
				glRotatef(gt.angle, gt.p.x, gt.p.y, gt.p.z);
				break;
			case GT_SCALE:
				glScalef(gt.p.x, gt.p.y, gt.p.z);
				break;
			default: assert(!"unreachable");
		}
	}

	glBegin(GL_TRIANGLES);
	for (auto mname : group->models) {
		std::vector<struct Point> model = scene->models[mname];
		for (auto p : model)
			glVertex3f(p.x, p.y, p.z);
	}
	glEnd();

	for (auto subgroup : group->subgroups) {
		glPushMatrix();
		sc_draw_group(scene, subgroup);
		glPopMatrix();
	}
}

void sc_draw (struct scene * scene)
{
	for (struct group * group : scene->groups) {
		glPushMatrix();
		sc_draw_group(scene, group);
		glPopMatrix();
	}
}

static void sc_load_model (const char * fname, std::vector<struct Point> * vec)
{
    FILE * inf = fopen(fname, "r");
    gen_model_read(inf, vec);
    fclose(inf);
}

static void sc_load_models (pugi::xml_node node, struct scene * scene, struct group * group)
{
    for (pugi::xml_node trans = node.first_child(); trans; trans = trans.next_sibling()) {
	if (strcmp("model", trans.name()) == 0) {
            const char * fname = trans.attribute("file").value();

            if (!scene->models.count(fname)) {
                std::vector<struct Point> model;
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
    struct gt rotate;
    rotate.type = GT_ROTATE;
    rotate.angle = maybe(node.attribute("ANGLE"), 0);
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
    struct gt translate;
    translate.type = GT_TRANSLATE;
    translate.p.x = maybe(node.attribute("X"), 0);
    translate.p.y = maybe(node.attribute("Y"), 0);
    translate.p.z = maybe(node.attribute("Z"), 0);
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
