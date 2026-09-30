#pragma once
#include "Platform.h"
#include <string>
#include <vector>
namespace m3 {
enum Sequence {S_PlayerIdle,S_PlayerForward,S_PlayerBackward,S_PlayerJump,S_PlayerGuard,S_PlayerKick,S_PlayerHit,S_PlayerFire,S_PlayerWeapon,S_Monster1Advance,S_Monster1AttackA,S_Monster1AttackB,S_Monster1Hit,S_Monster1Dead,S_Monster1Cast,S_Monster2Advance,S_Monster2AttackA,S_Monster2AttackB,S_Monster2Hit,S_Monster2Dead,S_Monster2Cast,S_Monster3Advance,S_Monster3AttackA,S_Monster3AttackB,S_Monster3Hit,S_Monster3Dead,S_Monster3Cast,S_Monster3Idle,S_Cage,S_Projectile,S_Impact,S_Spark,S_Dust,S_Aura,S_SpaceInsert,S_MindInsert,S_PowerInsert,S_HandGlow,S_ShipWalk,S_Door,S_BeachWalk,S_Reflection,S_Waves,SequenceCount};
enum Art {A_IntroBG,A_FightBG,A_Cages,A_Trigger,A_Guide1,A_Guide2,A_Guide3,A_HUD,A_CompleteBG,A_RewardBG,A_ClaimNormal,A_ClaimHover,A_ClaimPressed,A_PowerStone,A_FinalArena,A_FinalArenaFG,A_FinalIssac,A_FinalGuide,A_FinalEye,A_FinalPanel,A_HandEmpty,A_HandSpace,A_HandMind,A_HandComplete,A_Ship,A_ShipFG,A_Beach,A_Sun,A_Watching,A_Journey,ArtCount};
struct Texture { unsigned int id; int width,height; float u,v; int left,top,right,bottom; Texture():id(0),width(0),height(0),u(1),v(1),left(0),top(0),right(1),bottom(1){} };
class Assets {
 Texture animations[SequenceCount][6], art[ArtCount];
 bool loadTexture(Texture& target,const std::string& path);
 std::string loadRoot;
 int loadCursor;
public:
 Assets():loadCursor(1+ArtCount+SequenceCount*6){}
 std::vector<std::string> errors;
 void beginLoad(const std::string& root);
 void loadNext();
 bool loading() const {return loadCursor<1+ArtCount+SequenceCount*6;}
 int loadedPercent() const {return loadCursor*100/(1+ArtCount+SequenceCount*6);}
 bool load(const std::string& root);
 void release();
 const Texture& get(Art id) const {return art[id];}
 const Texture& get(Sequence id,int frame) const {return animations[id][frame<0?0:(frame>5?5:frame)];}
};
}
