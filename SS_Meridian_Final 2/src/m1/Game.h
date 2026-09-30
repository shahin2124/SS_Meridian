#pragma once
#include <string>
#include <vector>
#include "Navigation.h"
namespace Meridian {
const float WIDTH=1672, HEIGHT=941;
const int GUIDE_DIALOGUE_COUNT=11;
enum Scene { MENU, ARRIVAL, SHIP_A, SHIP_B, PICKUP, COUNTDOWN, ARENA, DIALOGUE, CHOICE, COWARD, R1, R2, R3, CONNECTOR, COCKPIT, DRIVE, RAIL_EXIT, CONGRATS, COMPLETE, REWARD, CLAIM_FLASH, CLAIMED, RETRY };
struct Input { bool w,a,s,d,attack,enter,click,hide; float mx,my; Input():w(false),a(false),s(false),d(false),attack(false),enter(false),click(false),hide(false),mx(-1),my(-1){} };
struct Draw {
 enum Kind { IMAGE, REGION, TEXT, BOX, PATCH, SHADOW } kind;
 std::string value; float x,y,w,h,alpha; bool flip; float sx,sy,sw,sh; std::vector<Point> polygon;
 Draw(Kind k,const std::string& v,float X,float Y,float W,float H,float A=1,bool F=false):kind(k),value(v),x(X),y(Y),w(W),h(H),alpha(A),flip(F),sx(0),sy(0),sw(1),sh(1){}
};
struct Enemy { float x,y,time; int type,hp,pose; bool removed; Enemy(float X,float Y,int T):x(X),y(Y),time(0),type(T),hp(2),pose(0),removed(false){} };
struct Effect { float x,y,time; bool hit; Effect(float X,float Y,bool H):x(X),y(Y),time(0),hit(H){} };
struct Tuning { static float walk(){return 220;} static float street(){return 180;} static float room(){return 220;} static float cabin(){return 32;} static float train(){return 20;} static float run(){return 20;} };
enum MonsterState { PATROL, SEARCH, CHASE, CATCH };
enum HideState { EXPOSED, HIDE_ENTER, HIDDEN, HIDE_EXIT };
class Game {
public:
 Scene scene, retryScene; float time,clock,x,y,anim,action,hurt,progress,monsterX,monsterY,monsterTime; int section,line,direction,hp,shots,monsterPose; bool mission1Complete,hasSpaceStone,moving,guideActive; std::vector<Enemy> enemies; std::vector<Effect> effects;
 MonsterState monsterState; HideState hideState;
 float spawnTimer,fireCooldown,hideTime,aiTime,lostSight,repath,monsterFacingX,monsterFacingY,turnTime;
 int totalSpawned,patrolIndex,hideIndex,retryPart; unsigned int randomState; bool hideWitnessed;
 Point lastSeen,monsterWaypoint; std::vector<Point> searchRoute;
 Game(); bool monsterCanSeePlayer() const; int nearbyHide() const; void update(float dt,const Input& in); std::vector<Draw> draw(const Input& in) const; void start(Scene s,int part=0); const char* sceneName() const;
private:
 void move(float dt,const Input& in,float speed,float minX,float maxX,float minY,float maxY); void fight(float dt,const Input& in,bool sword); void fail(Scene s); bool clear() const; void updateSpawner(float dt); void spawnZombie(); unsigned int random();
 void updateReality2(float dt,const Input& in); void moveMonster(float dt,Point target,float speed);
 void navigatePlayer(float dt,const Input& in,float speed,const Navigation& nav);
};
}
