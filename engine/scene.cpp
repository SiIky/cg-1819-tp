#include "scene.h"

#include "pugixml/pugixml.hpp"
#include "../generator/generators.h"

#include <string.h>

#define match(tag, func) \
	if (strcmp(tag, trans.name()) == 0) func(trans, scene, group)

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

            group->models.push_back(trans.name());
	}
    }
}

static void sc_load_rotate (pugi::xml_node node, struct scene * scene, struct group * group)
{
    struct gt rotate;
    rotate.type = GT_ROTATE;
    /*
     * TODO: LER O CARALHO DO VECTOR
     */
    group->gt.push_back(rotate);
}

static void sc_load_scale (pugi::xml_node node, struct scene * scene, struct group * group)
{
    struct gt scale;
    scale.type = GT_SCALE;
    /*
     * TODO: LER O CARALHO DO VECTOR
     */
    group->gt.push_back(scale);
}

static void sc_load_translate (pugi::xml_node node, struct scene * scene, struct group * group)
{
    struct gt translate;
    translate.type = GT_TRANSLATE;
    /*
     * TODO: LER O CARALHO DO VECTOR
     */
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
            struct group * group = (struct group*) calloc(1, sizeof(struct group));
            sc_load_group(trans, scene, group);
            group->subgroups.push_back(group);
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
