#include "Game.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
namespace Meridian {
static float clamp(float v,float lo,float hi){return std::max(lo,std::min(v,hi));}
static float dist(float x,float y,float a,float b){return std::sqrt((x-a)*(x-a)+(y-b)*(y-b));}
static bool button(const Input& i,float y){return i.mx>=636 && i.mx<=1036 && i.my>=y && i.my<=y+100;}

Game::Game():scene(MENU),retryScene(R1),time(0),clock(0),x(350),y(740),anim(0),action(-1),hurt(0),progress(0),monsterX(0),monsterY(0),monsterTime(0),section(0),line(0),direction(0),hp(3),shots(0),monsterPose(0),mission1Complete(false),hasSpaceStone(false),moving(false),guideActive(false),monsterState(PATROL),hideState(EXPOSED),spawnTimer(0),fireCooldown(0),hideTime(0),aiTime(0),lostSight(0),repath(0),monsterFacingX(-1),monsterFacingY(0),turnTime(0),totalSpawned(0),patrolIndex(0),hideIndex(-1),retryPart(0),randomState(0x93471u),hideWitnessed(false){}

const char* Game::sceneName()const {
 static const char* n[]={"Menu","Arrival","Ship A","Ship B","Hand pickup","Countdown","Main Arena","Guide dialogue","Ready choice","Elimination","Reality 1","Reality 2","Reality 3","Connector","Cockpit","Train drive","Railway exit","CONGRATS","Mission 1 Complete","Space Stone reward","Claim flash","Space Stone claimed","Temporary retry"};
 return n[scene];
}

void Game::start(Scene s,int part){
 scene=s;time=0;section=part;action=-1;hurt=0;anim=0;fireCooldown=0;moving=false;effects.clear();enemies.clear();
 if(s==MENU){mission1Complete=false;hasSpaceStone=false;guideActive=false;retryPart=0;}
 if(s==SHIP_A||s==SHIP_B){x=460;y=640;direction=3;}
 if(s==ARENA){x=760;y=890;direction=3;guideActive=false;}
 if(s==DIALOGUE)line=0;
 if(s==R1){
  x=230;y=710;direction=3;hp=3;spawnTimer=1.0f;
  if(part==0)totalSpawned=0;
  for(int i=0;i<6;++i)spawnZombie();
 }
 if(s==R2){
  x=part==0?540.0f:375.0f;y=810;direction=3;hp=1;
  const Navigation& nav=roomNavigation(part);
  monsterX=nav.patrol[0].x;monsterY=nav.patrol[0].y;monsterPose=0;monsterTime=0;
  monsterState=PATROL;hideState=EXPOSED;hideTime=0;hideIndex=-1;hideWitnessed=false;
  aiTime=0;lostSight=0;repath=0;patrolIndex=1;monsterFacingX=0;monsterFacingY=1;turnTime=0;
  searchRoute.clear();lastSeen=Point(monsterX,monsterY);monsterWaypoint=lastSeen;
 }
 if(s==R3){
  x=836;y=820;direction=1;hp=3;spawnTimer=.45f;
  // A few ghosts begin ahead; more keep entering from the far end while Issac advances.
  enemies.push_back(Enemy(795,470,(0+part)%2));
  enemies.push_back(Enemy(880,385,(1+part)%2));
 }
 if(s==CONNECTOR){x=836;y=820;direction=1;}
 if(s==COCKPIT){x=836;y=810;direction=1;progress=0;}
 if(s==DRIVE){progress=0;}
 if(s==RAIL_EXIT){progress=20.0f;}
}

void Game::move(float dt,const Input& in,float speed,float minX,float maxX,float minY,float maxY){
 float oldX=x,oldY=y;float dx=float(in.d)-float(in.a),dy=float(in.s)-float(in.w);
 if(dx&&dy){dx*=.70710678f;dy*=.70710678f;}
 if(in.a)direction=2;else if(in.d)direction=3;else if(in.w)direction=1;else if(in.s)direction=0;
 x=clamp(x+dx*speed*dt,minX,maxX);y=clamp(y+dy*speed*dt,minY,maxY);
 moving=dist(x,y,oldX,oldY)>.01f;if(moving)anim+=(scene==SHIP_A||scene==SHIP_B)?dist(x,y,oldX,oldY)/speed:dt;else anim=0;
}

bool Game::clear()const{for(size_t i=0;i<enemies.size();++i)if(!enemies[i].removed)return false;return true;}

void Game::fail(Scene s){
 // Scene-local checkpoint: retain the current reality sub-scene, not the beginning of the reality.
 retryScene=s;retryPart=section;scene=RETRY;time=0;action=-1;moving=false;
}

void Game::fight(float dt,const Input& in,bool sword){
 const float duration=sword?.64f:.42f;
 if(action>=0){
  float prev=action;action+=dt;
  if(prev<.16f&&action>=.16f){
   int target=-1;float nearest=1e9f;
   for(size_t j=0;j<enemies.size();++j){
    Enemy& e=enemies[j];float d=dist(x,y,e.x,e.y);
    if(!e.removed&&e.hp>0&&d<nearest&&(sword?d<220:(d<720&&e.x>30&&e.x<WIDTH-30))){target=int(j);nearest=d;}
   }
   float ex=x+(direction==2?-340:340),ey=y-100;
   if(target>=0){Enemy& e=enemies[target];--e.hp;e.pose=4;e.time=0;ex=e.x;ey=e.y-100;direction=e.x<x?2:3;}
   effects.push_back(Effect(ex,ey,target>=0));++shots;
  }
  if(action>=duration){action=-1;if(!sword)fireCooldown=.30f;}
 }
 if(in.attack&&action<0&&hurt<=0&&(sword||fireCooldown<=0))action=0;

 for(size_t j=0;j<enemies.size();++j){
  Enemy& e=enemies[j];if(e.removed)continue;e.time+=dt;
  if(e.pose==4){if(e.time>.2f){e.pose=e.hp<=0?5:1;e.time=0;}continue;}
  if(e.pose==5){if(e.time>.7f)e.removed=true;continue;}
  if(e.pose==0){if(e.time>.5f)e.pose=1;continue;}
  float d=dist(x,y,e.x,e.y);
  if(e.pose==3){
   if(e.time>.58f){if(d<(sword?90.0f:125.0f)&&hurt<=0){--hp;hurt=.75f;}e.pose=1;e.time=0;}
   continue;
  }
  if(d<(sword?70.0f:100.0f)){e.pose=3;e.time=0;}
  else {
   float speed=sword?42.0f:78.0f;
   if(sword){
    // Reality 3 ghosts travel down the physical aisle. They do not fly directly through screen space.
    float targetX=clamp(x,755.0f,915.0f);
    float dx=targetX-e.x;
    e.x+=clamp(dx,-45.0f*dt,45.0f*dt);
    float dy=y-e.y;
    if(std::fabs(dy)>2)e.y+=(dy>0?1.0f:-1.0f)*std::min(std::fabs(dy),speed*dt);
    e.x=clamp(e.x,745.0f,925.0f);e.y=clamp(e.y,320.0f,850.0f);
   }else{
    float tx=x,ty=y;float dx=tx-e.x,dy=ty-e.y;d=std::max(1.0f,std::sqrt(dx*dx+dy*dy));
    e.x+=dx/d*speed*dt;e.y+=dy/d*speed*dt;
   }
   e.pose=1+int(e.time*5)%2;
  }
 }
 for(size_t j=0;j<effects.size();){effects[j].time+=dt;if(effects[j].time>.65f)effects.erase(effects.begin()+j);else ++j;}
 if(hp<=0&&hurt<=.1f)fail(sword?R3:R1);
}

void Game::update(float dt,const Input& in){
 dt=clamp(dt,0,.05f);time+=dt;clock+=dt;hurt=std::max(0.0f,hurt-dt);fireCooldown=std::max(0.0f,fireCooldown-dt);moving=false;
 switch(scene){
 case MENU:if(in.click&&button(in,650))start(ARRIVAL);break;
 case ARRIVAL:if(time>1.6f)start(SHIP_A);break;
 case SHIP_A:move(dt,in,Tuning::walk(),440,1450,560,750);if(x>1400)start(SHIP_B);break;
 case SHIP_B:move(dt,in,Tuning::walk(),440,1430,510,770);if(in.enter&&dist(x,y,1130,620)<145)start(PICKUP);break;
 case PICKUP:if(time>=1.0f)start(COUNTDOWN);break;
 case COUNTDOWN:if(time>=3)start(ARENA);break;
 case ARENA:
  navigatePlayer(dt,in,170,arenaNavigation());if(time>2)guideActive=true;
  if(time>3&&distance(Point(x,y),GUIDE_FOOT)<72)start(DIALOGUE);
  break;
 case DIALOGUE:if(in.enter){++line;if(line>=GUIDE_DIALOGUE_COUNT)start(CHOICE);}break;
 case CHOICE:if(in.click){if(button(in,550))start(R1);else if(button(in,685))start(COWARD);}break;
 case COWARD:if(in.enter)start(MENU);break;
 case R1:{
  updateSpawner(dt);
  float minY=section==2?500.0f:590.0f;
  if(hp>0&&action<0&&hurt<=0)move(dt,in,Tuning::street(),180,1490,minY,810);
  fight(dt,in,false);if(scene!=R1)break;
  if(section<2){if(x>1430)start(R1,section+1);}
  else if(in.enter&&dist(x,y,836,555)<145)start(R2);
  break;}
 case R2:updateReality2(dt,in);break;
 case R3:{
  // Continuous cabin pressure: new ghosts keep entering from the far end until Issac reaches the connector.
  spawnTimer-=dt;
  if(spawnTimer<=0&&y>385){
   int active=0;for(size_t j=0;j<enemies.size();++j)if(!enemies[j].removed)++active;
   if(active<3){
    float gx=770.0f+float(random()%135);
    float gy=325.0f+float(random()%70);
    Enemy g(gx,gy,int(random()%2));
    bool reused=false;for(size_t j=0;j<enemies.size();++j)if(enemies[j].removed){enemies[j]=g;reused=true;break;}
    if(!reused)enemies.push_back(g);
   }
   spawnTimer=1.25f+float(random()%950)/1000.0f;
  }
  if(hp>0&&action<0)move(dt,in,210,710,970,315,840);
  fight(dt,in,true);
  if(scene==R3&&y<355)start(CONNECTOR,section);
  break;}
 case CONNECTOR:
  move(dt,in,185,760,910,305,840);
  if(y<355){if(section==0)start(R3,1);else start(COCKPIT);}break;
 case COCKPIT:if(in.w||in.enter)start(DRIVE);break;
 case DRIVE:
  if(in.w)progress+=dt;
  if(progress>=Tuning::train())start(RAIL_EXIT);
  break;
 case RAIL_EXIT:
  if(in.w)progress+=dt;
  if(progress>=22.2f)start(CONGRATS);
  break;
 case CONGRATS:if(in.enter)start(COMPLETE);break;
 case COMPLETE:if(time>=2.1f)start(REWARD);break;
 case REWARD:if(in.click&&button(in,720)&&!hasSpaceStone)start(CLAIM_FLASH);break;
 case CLAIM_FLASH:if(time>=.6f){mission1Complete=true;hasSpaceStone=true;start(CLAIMED);}break;
 case CLAIMED:break;
 case RETRY:if(in.enter)start(retryScene,retryPart);break;
 }
}
}
