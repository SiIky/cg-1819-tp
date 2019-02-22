#ifndef _SCENE_H
#define _SCENE_H

#include "pugixml/pugixml.hpp"

#include <vector>

bool sc_load_file (const char * path, std::vector<struct Point> * vec);

#endif /* _SCENE_H */
