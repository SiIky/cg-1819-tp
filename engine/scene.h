#ifndef _SCENE_H
#define _SCENE_H

#include "../generator/generators.h"

#include "pugixml/pugixml.hpp"

#include <map>
#include <string>
#include <vector>

enum type {
	GT_TRANSLATE,
	GT_ROTATE,
	GT_SCALE,
};

struct gt {
	enum type type;
	float angle;
	struct Point p;
};

struct group {
	std::vector<struct gt>     gt;
	std::vector<std::string>   models;
	std::vector<struct group*> subgroups;
};

struct model {
	GLuint id;
	size_t length;
};

struct scene {
	std::vector<struct group*> groups;
	std::map<std::string, struct model> models;
};

bool sc_load_file (const char * path, struct scene * scene);
void sc_draw      (struct scene * scene);

#endif /* _SCENE_H */
