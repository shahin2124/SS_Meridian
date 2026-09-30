#pragma once
#include "Game.h"
#include <map>
#include <set>
class Renderer {
 struct Texture { unsigned int id; float u,v; unsigned int last; size_t bytes; Texture():id(0),u(1),v(1),last(0),bytes(0){} };
 std::map<std::string,Texture> textures; unsigned int tick; size_t bytes;
 Texture& load(const std::string& name);
public:
 std::set<std::string> rendered;
 Renderer():tick(0),bytes(0){} ~Renderer();
 void paint(const std::vector<Meridian::Draw>& commands);
 void report()const;
};
