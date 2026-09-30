#include "../../src/m2/Presentation.h"
#include <iostream>
#include <cstdlib>
using namespace m2;
int main(int argc,char**argv){Game g;g.state=argc>1?State(std::atoi(argv[1])):M2_START;g.stateTime=.45f;if(g.state==M2_DRAGON_FIGHT){g.attacks.push_back(Attack(0,g.x+250));g.attacks.back().age=.7f;}Presentation p;p.draw(g);for(size_t i=0;i<p.list.size();++i){const Draw&d=p.list[i];std::cout<<d.asset<<"|"<<d.box.x<<"|"<<d.box.y<<"|"<<d.box.w<<"|"<<d.box.h<<"|"<<d.alpha<<"|"<<d.angle<<"|"<<d.crop<<"|"<<d.size<<"|"<<d.r<<"|"<<d.g<<"|"<<d.b<<"|"<<d.text<<"\n";}}
