#include "Mission3.h"
#include <algorithm>
#include <cmath>
namespace m3 {
static float clamp(float x,float lo,float hi){return std::max(lo,std::min(x,hi));}
static void tick(float& x,float dt){x=std::max(0.0f,x-dt);}
bool Rect::contains(float px,float py) const {return px>=x&&px<=x+w&&py>=y&&py<=y+h;}
bool Rect::overlaps(const Rect& b) const{return x<b.x+b.w&&x+w>b.x&&y<b.y+b.h&&y+h>b.y;}
Fighter::Fighter():x(0),health(0),time(0),cooldown(0),invulnerable(0),jumpTime(-1),action(Idle),registered(false),attackCount(0),variant(0){}
void Fighter::play(Action a){action=a;time=0;registered=false;}
Progress::Progress():mission1Complete(true),mission2Complete(true),mission3Complete(false),hasSpaceStone(true),hasMindStone(true),hasPowerStone(false){for(int i=0;i<3;++i) defeated[i]=false;}
Mission::Mission():state(Enter),current(0),finalDialogue(0),beachTime(0),shipProgress(0),beachProgress(0),shipWalkTime(0),beachWalkTime(0),reflectionLine(0),walkArmed(false),stateTime(0),remaining(60),walkTravel(0),fireCooldown(0),weaponCooldown(0),kickCooldown(0),guardCooldown(0),triggerUsed(false),buttonDown(false),buttonHover(false){
 player.x=Layout::IntroStart;player.health=config.playerMaxHealth;
 for(int i=0;i<3;++i){monsters[i].x=Layout::MonsterStart;monsters[i].health=config.monsterHealth[i];}
}
void Mission::go(State s){state=s;stateTime=0;cancelPointer();
 if(s==FinalShipWalk){shipProgress=shipWalkTime=0;walkArmed=false;}
 if(s==FinalBeachWalk){beachProgress=beachWalkTime=beachTime=0;reflectionLine=0;walkArmed=false;}
}
void Mission::clearTransient(){for(int i=0;i<4;++i)projectiles[i]=Projectile();for(int i=0;i<8;++i)effects[i]=Effect();fireCooldown=weaponCooldown=kickCooldown=guardCooldown=0;}
void Mission::prepareFight(){
 clearTransient();player=Fighter();player.x=Layout::PlayerStart;player.health=config.playerHealthForFight(current);
 monsters[current]=Fighter();monsters[current].x=Layout::MonsterStart;monsters[current].health=config.monsterHealth[current];
 remaining=config.fightDuration;
}
Rect Mission::buttonRect() const {Rect r={480,492,320,100};return r;}
void Mission::cancelPointer(){buttonDown=false;buttonHover=false;}
void Mission::pointer(float x,float y,bool down,bool released){
 bool eligible=state==Enter||state==Reward;
 buttonHover=eligible&&buttonRect().contains(x,y);
 if(down)buttonDown=buttonHover;
 if(released){bool activate=buttonDown&&buttonHover;buttonDown=false;
  if(activate&&state==Enter)go(Intro);
  else if(activate&&state==Reward&&!progress.hasPowerStone&&progress.defeated[0]&&progress.defeated[1]&&progress.defeated[2]){
   progress.hasPowerStone=true;progress.mission3Complete=true;go(Claimed);
  }
  // Claimed has no interactive button; its timed ending follows in update().
 }
}
float Mission::jumpOffset() const {
 if(player.jumpTime<0)return 0;
 return config.jumpHeight*std::sin(3.14159265f*clamp(player.jumpTime/config.jumpDuration,0,1));
}
Rect Mission::playerHurtbox() const {Rect r={player.x-42,Layout::Ground-310-jumpOffset(),84,300};return r;}
Rect Mission::monsterHurtbox() const {Rect r={monsters[current].x-60,Layout::Ground-320,120,315};return r;}
float Mission::actionDuration(const Fighter& f,bool enemy) const {
 if(f.action==Dead)return config.defeatDuration;
 if(f.action==Hit)return config.hitDuration;
 if(f.action==Telegraph)return config.telegraphDuration;
 if(f.action==AttackA||f.action==AttackB)return config.attackDuration;
 if(f.action==Recovery)return config.recoveryDuration;
 if(f.action==Kick)return config.kickDuration;
 if(f.action==Weapon)return config.weaponDuration;
 if(f.action==Fire)return config.fireDuration;
 if(f.action==Guard)return config.guardDuration;
 if(f.action==Jump)return config.jumpDuration;
 return enemy?.84f:.72f;
}
int Mission::frame(const Fighter& f,bool enemy) const {
 int k=static_cast<int>(f.time/actionDuration(f,enemy)*6);
 if(f.action==Idle||f.action==Forward||f.action==Backward||f.action==Advance)return k%6;
 return std::min(5,k);
}
void Mission::effect(int kind,float x,float y){for(int i=0;i<8;++i)if(!effects[i].active){effects[i].active=true;effects[i].kind=kind;effects[i].x=x;effects[i].y=y;effects[i].time=0;break;}}
void Mission::hitMonster(float damage,bool fire){
 Fighter& m=monsters[current];if(m.health<=0)return;
 m.health=std::max(0.0f,m.health-damage);effect(fire?1:0,m.x-45,Layout::Ground-225);
 if(m.health<=0){m.play(Dead);for(int i=0;i<4;++i)projectiles[i].active=false;go(Defeat);}
 else if(m.invulnerable<=0&&m.action!=Telegraph&&m.action!=AttackA&&m.action!=AttackB){m.play(Hit);m.invulnerable=config.monsterStaggerCooldown;}
 // Telegraph/attack super armor avoids permanent stunlock. Configurable prototype pattern.
}
void Mission::hitPlayer(float damage){
 if(player.invulnerable>0||player.health<=0)return;
 if(player.action==Guard){damage*=config.guardMultiplier;}
 else {player.play(Hit);}
 player.health=std::max(0.0f,player.health-damage);player.invulnerable=config.invulnerability;
 effect(0,player.x+20,Layout::Ground-235-jumpOffset());
 if(player.health<=0){clearTransient();go(Failed);}
}
void Mission::movePlayer(float dt,const Input& in,bool fighting){
 bool free=player.action==Idle||player.action==Forward||player.action==Backward||player.action==Jump;
 if(!free)return;
 int dir=(in.right?1:0)-(in.left?1:0);
 float lo=fighting?static_cast<float>(Layout::PlayerMin):static_cast<float>(Layout::IntroStart);
 float hi=fighting?std::min(static_cast<float>(Layout::PlayerMax),monsters[current].x-Layout::Separation):static_cast<float>(Layout::TriggerX);
 if(fighting||state==TriggerReady)player.x=clamp(player.x+dir*config.playerSpeed*dt,lo,hi);
 else walkTravel=clamp(walkTravel+dir*config.playerSpeed*dt,0,config.walkDistance);
 if(player.action!=Jump){Action a=dir>0?Forward:(dir<0?Backward:Idle);if(player.action!=a)player.play(a);}
}
void Mission::update(float dt,const Input& in){
 if(dt<=0)return;
 // Platform feeds fixed 1/60 s updates, capped catch-up; tests use the same clock.
 dt=std::min(dt,1.0f/30.0f);stateTime+=dt;
 if(state==Enter){if(in.confirm)go(Intro);return;}
 if(state==Intro){if(stateTime>=config.introDuration)go(Walk);return;}
 if(state==Walk||state==TriggerReady){player.time+=dt;movePlayer(dt,in,false);
  if(state==Walk&&walkTravel>=config.walkDistance-.01f){player.x=Layout::IntroStart;go(TriggerReady);}
  if(state==TriggerReady&&player.x>=Layout::TriggerX-8&&!triggerUsed){triggerUsed=true;current=0;prepareFight();go(Guide);}return;}
 if(state==Guide){if(in.confirm)go(CageOpen);return;}
 if(state==CageOpen){if(stateTime>=1.2f)go(Fight);return;}
 // Temporary/configurable retry presentation and rule, not permanent story canon.
 if(state==Failed){if(in.confirm){prepareFight();go(Guide);}return;}
 if(state==Defeat){monsters[current].time+=dt;
  for(int i=0;i<8;++i)if(effects[i].active){effects[i].time+=dt;if(effects[i].time>=.54f)effects[i].active=false;}
  if(monsters[current].time>=config.defeatDuration){progress.defeated[current]=true;
   if(current==2)go(Complete);else {++current;prepareFight();go(Guide);}}
  return;
 }
 if(state==Complete){if(stateTime>=config.completeDuration)go(Reward);return;}
 if(state==Claimed){if(progress.hasPowerStone&&stateTime>=2.5f)go(FinalArenaEnter);return;}
 if(state>=FinalArenaEnter){updateEnding(dt,in);return;}
 if(state==Fight)combat(dt,in);
}
void Mission::combat(float dt,const Input& in){
 remaining=std::max(0.0f,remaining-dt);
 if(remaining<=0){clearTransient();go(Failed);return;}
 Fighter& m=monsters[current];player.time+=dt;m.time+=dt;
 tick(m.invulnerable,dt);tick(player.invulnerable,dt);tick(fireCooldown,dt);tick(weaponCooldown,dt);tick(kickCooldown,dt);tick(guardCooldown,dt);
 for(int i=0;i<8;++i)if(effects[i].active){effects[i].time+=dt;if(effects[i].time>=.54f)effects[i].active=false;}
 if(player.jumpTime>=0){player.jumpTime+=dt;if(player.jumpTime>=config.jumpDuration){player.jumpTime=-1;effect(2,player.x,Layout::Ground);if(player.action==Jump)player.play(Idle);}}
 if((player.action==Kick||player.action==Weapon||player.action==Fire||player.action==Hit||player.action==Guard)&&player.time>=actionDuration(player,false))player.play(Idle);
 bool free=player.action==Idle||player.action==Forward||player.action==Backward;
 if(free){
  if(in.guard&&guardCooldown<=0){player.play(Guard);guardCooldown=config.guardCooldown;}
  else if(in.jump&&player.jumpTime<0){player.play(Jump);player.jumpTime=0;}
  else if(in.weapon&&weaponCooldown<=0){player.play(Weapon);weaponCooldown=config.weaponCooldown;}
  else if(in.fire&&fireCooldown<=0){player.play(Fire);fireCooldown=config.fireCooldown;}
  else if(in.kick&&kickCooldown<=0){player.play(Kick);kickCooldown=config.kickCooldown;}
 }
 movePlayer(dt,in,true);
 int pf=frame(player,false);
 if((player.action==Kick||player.action==Weapon)&&pf>=3&&pf<=4&&!player.registered){
  float range=player.action==Kick?config.kickRange:config.weaponRange;
  Rect strike={player.x+20,Layout::Ground-300,range,265};
  if(strike.overlaps(monsterHurtbox())){player.registered=true;hitMonster(player.action==Kick?config.kickDamage:config.weaponDamage,false);if(state!=Fight)return;}
 }
 if(player.action==Fire&&pf>=3&&!player.registered){player.registered=true;
  for(int i=0;i<4;++i)if(!projectiles[i].active){projectiles[i].active=true;projectiles[i].x=player.x+130;projectiles[i].y=Layout::Ground-290;projectiles[i].time=0;break;}
 }
 for(int i=0;i<4;++i)if(projectiles[i].active){Projectile& p=projectiles[i];float old=p.x;p.x+=config.projectileSpeed*dt;p.time+=dt;
  Rect sweep={old-20,p.y-22,p.x-old+40,44};
  if(sweep.overlaps(monsterHurtbox())){p.active=false;hitMonster(config.fireDamage,true);if(state!=Fight)return;}
  else if(p.x>Layout::Width+60)p.active=false;
 }
 if(m.action==Hit){if(m.time>=config.hitDuration)m.play(Recovery);return;}
 if(m.action==Telegraph){if(m.time>=config.telegraphDuration)m.play(m.variant==0?AttackA:AttackB);return;}
 if(m.action==AttackA||m.action==AttackB){
  if(frame(m,true)>=3&&frame(m,true)<=4&&!m.registered){
   float reach=config.monsterRange[current][m.variant];
   // Low/mid attack volume: jump physically clears it near the apex.
   Rect strike={m.x-reach,Layout::Ground-100,reach-15,100};
   if(strike.overlaps(playerHurtbox())){m.registered=true;hitPlayer(config.monsterDamage[current][m.variant]);if(state!=Fight)return;}
  }
  if(m.time>=config.attackDuration)m.play(Recovery);return;
 }
 if(m.action==Recovery){if(m.time>=config.recoveryDuration)m.play(Idle);return;}
 float gap=m.x-player.x;
 m.variant=m.attackCount%2;
 if(gap<=config.monsterRange[current][m.variant]+28){++m.attackCount;m.play(Telegraph);}
 else {if(m.action!=Advance)m.play(Advance);m.x=clamp(m.x-config.monsterSpeed[current]*dt,std::max(static_cast<float>(Layout::MonsterMin),player.x+Layout::Separation),Layout::MonsterMax);}
}
void Mission::updateEnding(float dt,const Input& in){
 // Movement is clamped even in terminal beach/door states. Only a released,
 // then held W can move a newly entered scene; combat's W jump is unchanged.
 shipProgress=clamp(shipProgress,0,1);beachProgress=clamp(beachProgress,0,1);
 if((state==FinalShipWalk||state==FinalBeachWalk)&&!in.walkHeld)walkArmed=true;
 // Enter is already a key-down edge, consumed once by the existing fixed-step loop.
 // Each case handles one state only, so no input can cross two transitions.
 switch(state){
 case FinalArenaEnter: if(stateTime>=1.0f)go(FinalDialogue);break;
 case FinalDialogue: if(in.confirm){if(finalDialogue<3)++finalDialogue;else go(FinalSpace);}break;
 case FinalSpace: if(stateTime>=1.8f)go(FinalMind);break;
 case FinalMind: if(stateTime>=1.8f)go(FinalPower);break;
 case FinalPower: if(stateTime>=1.8f)go(FinalGlow);break;
 case FinalGlow: if(stateTime>=2.4f)go(FinalBlackFade);break;
 case FinalBlackFade: if(stateTime>=1.5f)go(FinalShipCount);break;
 case FinalShipCount: if(stateTime>=3.0f)go(FinalShipWalk);break;
 case FinalShipWalk:
  if(walkArmed&&in.walkHeld){shipProgress=clamp(shipProgress+dt/5.0f,0,1);shipWalkTime+=dt;}
  if(shipProgress>=1){shipProgress=1;go(FinalDoorOpen);}break;
 case FinalDoorOpen: if(stateTime>=1.2f)go(FinalDoorWait);break;
 case FinalDoorWait: if(in.confirm){beachTime=0;go(FinalBeachWalk);}break;
 case FinalBeachWalk:
  beachTime+=dt;
  if(walkArmed&&in.walkHeld){beachProgress=clamp(beachProgress+dt/10.0f,0,1);beachWalkTime+=dt;}
  if(beachProgress>=1){beachProgress=1;go(FinalReflection);}break;
 case FinalReflection:
  beachTime+=dt;if(in.confirm){if(reflectionLine<6)++reflectionLine;else go(FinalBeachWatch);}break;
 case FinalBeachWatch: beachTime+=dt;if(stateTime>=3.0f)go(FinalBeachFade);break;
 case FinalBeachFade: beachTime+=dt;if(stateTime>=.8f)go(FinalJourney);break;
 default: break; // FinalJourney is terminal; no additional mission.
 }
}
const char* Mission::stateName() const {const char* names[]={"MISSION 3 ENTER","ARENA INTRO","WALK TO CAGES","RED CIRCLE","MONSTER GUIDE","CAGE OPEN","FIGHT","DEFEAT","MISSION 3 COMPLETE","POWER STONE REWARD","POWER STONE CLAIMED","FIGHT FAILED","FinalArenaEnter","FinalDialogue","FinalSpace","FinalMind","FinalPower","FinalGlow","FinalBlackFade","FinalShipCount","FinalShipWalk","FinalDoorOpen","FinalDoorWait","FinalBeachWalk","FinalReflection","FinalBeachWatch","FinalBeachFade","FinalJourney"};return names[state];}
}
