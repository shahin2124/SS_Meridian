#pragma once
#include <string>
namespace Meridian {
// One calibration per animation family; never recenter or resize individual frames.
struct SpriteCalibration { float anchorX,anchorY,standingHeight; SpriteCalibration(float x=0,float y=0,float h=1):anchorX(x),anchorY(y),standingHeight(h){} };
SpriteCalibration calibration(const std::string& key);
float arenaActorHeight(float feetY);
}
