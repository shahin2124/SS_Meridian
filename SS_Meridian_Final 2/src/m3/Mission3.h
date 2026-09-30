#pragma once
#include "Mission3Config.h"
namespace m3 {
enum State { Enter, Intro, Walk, TriggerReady, Guide, CageOpen, Fight, Defeat,
             Complete, Reward, Claimed, Failed, FinalArenaEnter, FinalDialogue, FinalSpace, FinalMind, FinalPower, FinalGlow, FinalBlackFade, FinalShipCount, FinalShipWalk, FinalDoorOpen, FinalDoorWait, FinalBeachWalk, FinalReflection, FinalBeachWatch, FinalBeachFade, FinalJourney };
enum Action { Idle, Forward, Backward, Jump, Guard, Kick, Weapon, Fire, Hit,
              Advance, Telegraph, AttackA, AttackB, Recovery, Dead };
struct Input {
    bool left,right,jump,guard,kick,weapon,fire,confirm,walkHeld;
    Input():left(false),right(false),jump(false),guard(false),kick(false),weapon(false),fire(false),confirm(false),walkHeld(false){}
};
struct Rect { float x,y,w,h; bool contains(float px,float py) const; bool overlaps(const Rect& b) const; };
struct Fighter {
    float x,health,time,cooldown,invulnerable,jumpTime;
    Action action;
    bool registered;
    int attackCount,variant;
    Fighter();
    void play(Action a);
};
struct Projectile { bool active; float x,y,time; Projectile():active(false),x(0),y(0),time(0){} };
struct Effect { bool active; int kind; float x,y,time; Effect():active(false),kind(0),x(0),y(0),time(0){} };
struct Progress {
    bool mission1Complete,mission2Complete,mission3Complete,hasSpaceStone,hasMindStone,hasPowerStone;
    bool defeated[3];
    Progress();
};
class Mission {
public:
    Config config;
    State state;
    Progress progress;
    Fighter player,monsters[3];
    Projectile projectiles[4];
    Effect effects[8];
    int current;
    int finalDialogue;
    float beachTime;
    float shipProgress,beachProgress,shipWalkTime,beachWalkTime;
    int reflectionLine;
    bool walkArmed;
    float stateTime,remaining,walkTravel,fireCooldown,weaponCooldown,kickCooldown,guardCooldown;
    bool triggerUsed,buttonDown,buttonHover;
    Mission();
    void update(float dt,const Input& input);
    void pointer(float x,float y,bool down,bool released);
    void cancelPointer();
    Rect buttonRect() const;
    Rect playerHurtbox() const;
    Rect monsterHurtbox() const;
    float jumpOffset() const;
    float actionDuration(const Fighter& f,bool enemy) const;
    int frame(const Fighter& f,bool enemy) const;
    const char* stateName() const;
private:
    void updateEnding(float dt,const Input& input);
    void go(State s);
    void prepareFight();
    void combat(float dt,const Input& input);
    void movePlayer(float dt,const Input& input,bool fighting);
    void hitMonster(float damage,bool fire);
    void hitPlayer(float damage);
    void effect(int kind,float x,float y);
    void clearTransient();
};
}
