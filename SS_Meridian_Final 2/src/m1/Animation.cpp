#include "Animation.h"
#include <algorithm>
namespace Meridian {
static bool prefix(const std::string& k,const char* p){return k.find(p)==0;}
SpriteCalibration calibration(const std::string& k){
 if(prefix(k,"walk_left_"))return SpriteCalibration(211,303,285);
 if(prefix(k,"walk_right_"))return SpriteCalibration(218,303,285);
 if(prefix(k,"walk_"))return SpriteCalibration(213,303,285);
 if(prefix(k,"gun_run_"))return SpriteCalibration(246,443,425);
 if(prefix(k,"gun_"))return SpriteCalibration(196,657,615);
 if(prefix(k,"run_right_"))return SpriteCalibration(164,342,327);
 if(prefix(k,"run_back_"))return SpriteCalibration(160,341,328);
 if(prefix(k,"run_front_"))return SpriteCalibration(152,345,317);
 // Hiding art depicts a bent/crouched body. Preserve that reduction in height.
 if(prefix(k,"hide_"))return SpriteCalibration(290,757,925);
 if(prefix(k,"hand_pickup_"))return SpriteCalibration(280,681,675);
 if(prefix(k,"guide_idle_"))return SpriteCalibration(294,779,763);
 if(k=="guide_arena_idle")return SpriteCalibration(585,1494,1467);
 if(k=="issac_facing_guide")return SpriteCalibration(510,1497,1410);
 return SpriteCalibration();
}
float arenaActorHeight(float y){float t=std::max(0.0f,std::min(1.0f,(y-715)/200));return 140+42*t;}
}
