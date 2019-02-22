#include "scene.h"

#include "pugixml/pugixml.hpp"
#include "../generator/generators.h"

#include <string.h>

static void sc_load_models (pugi::xml_node node, std::vector<struct Point> * vec)
{
    FILE * inf = fopen(node.attribute("file").value(), "r");
    gen_model_read(inf, vec);
    fclose(inf);
}

bool sc_load_file (const char * path, std::vector<struct Point> * vec)
{
    pugi::xml_document doc;
    if (!doc.load_file(path))
        return false;

    pugi::xml_node models = doc.child("scene");

    for (pugi::xml_node trans = models.first_child(); trans; trans = trans.next_sibling()) {
        if (strcmp("model", trans.name()) == 0)
            sc_load_models(trans, vec);
    }

    return true;
}
