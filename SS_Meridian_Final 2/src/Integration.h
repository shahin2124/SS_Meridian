#pragma once
#include "m1/Game.h"
#include "m2/Game.h"
#include "m3/Mission3.h"
namespace app {
struct Session {
 bool mission1Complete,mission2Complete,mission3Complete,hasSpaceStone,hasMindStone,hasPowerStone;
 Session():mission1Complete(false),mission2Complete(false),mission3Complete(false),hasSpaceStone(false),hasMindStone(false),hasPowerStone(false){}
 void begin(int mission){*this=Session();mission1Complete=hasSpaceStone=mission>=2;mission2Complete=hasMindStone=mission>=3;}
 void observe(const Meridian::Game& g){mission1Complete=g.mission1Complete;hasSpaceStone=g.hasSpaceStone;}
 void observe(const m2::Game& g){mission2Complete=hasMindStone=g.claimed;}
 void observe(const m3::Mission& g){mission3Complete=g.progress.mission3Complete;hasPowerStone=g.progress.hasPowerStone;}
 void apply(m3::Mission& g)const{g.progress.mission1Complete=mission1Complete;g.progress.mission2Complete=mission2Complete;g.progress.hasSpaceStone=hasSpaceStone;g.progress.hasMindStone=hasMindStone;g.progress.mission3Complete=mission3Complete;g.progress.hasPowerStone=hasPowerStone;}
};
inline bool requestMission2(const Meridian::Game& g,const Meridian::Input& in){
 return g.scene==Meridian::CLAIMED&&g.mission1Complete&&g.hasSpaceStone&&in.click&&in.mx>=636&&in.mx<=1036&&in.my>=720&&in.my<=820;
}
inline bool requestMission3(const m2::Game& g){return g.state==m2::M2_HANDOFF_END&&g.claimed;}
}
