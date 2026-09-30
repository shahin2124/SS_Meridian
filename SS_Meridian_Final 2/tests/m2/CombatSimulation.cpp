#include "../../src/m2/Game.h"
#include <cassert>
#include <iostream>
using namespace m2;
int main(){Game g;g.activate();g.tick(1.3f,Input());g.activate();bool right=true;int peak=0;for(int n=0;n<21600&&g.state==M2_DRAGON_FIGHT;++n){Input in;if(g.x>=380)right=false;if(g.x<=20)right=true;in.right=right;in.left=!right;in.fire=true;in.reload=g.ammo==0;g.tick(1.f/120,in);peak=std::max(peak,int(g.attacks.size()));}std::cout<<"Continuous-movement combat simulation: state="<<g.state<<" HP="<<g.hp<<" hits="<<g.hits<<" elapsed="<<g.elapsed<<" peak attacks="<<peak<<"\n";assert(g.state==M2_DRAGON_DEFEATED&&g.hits==30&&peak>=2&&peak<=3);}
