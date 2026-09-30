#include "Game.h"
#include <cmath>
#include <algorithm>
namespace Meridian {
unsigned int Game::random(){randomState^=randomState<<13;randomState^=randomState>>17;randomState^=randomState<<5;return randomState;}

void Game::spawnZombie(){
 Enemy e((random()%3)?1800.0f+float(random()%900):-400.0f-float(random()%400),590.0f+float(random()%220),int(random()%3));
 for(size_t i=0;i<enemies.size();++i)if(enemies[i].removed){enemies[i]=e;++totalSpawned;return;}
 enemies.push_back(e);++totalSpawned;
}

void Game::updateSpawner(float dt){
 spawnTimer-=dt;
 if(spawnTimer<=0){
  int count=0;for(size_t i=0;i<enemies.size();++i)if(!enemies[i].removed)++count;
  if(count<8)spawnZombie();
  spawnTimer=.8f+float(random()%1201)/1000.0f;
 }
}

void Game::navigatePlayer(float dt,const Input& in,float speed,const Navigation& nav){
 float dx=float(in.d)-float(in.a),dy=float(in.s)-float(in.w);
 if(dx&&dy){dx*=.70710678f;dy*=.70710678f;}
 if(dx)direction=dx<0?2:3;else if(dy)direction=dy<0?1:0;
 Point before(x,y),after=nav.slide(before,Point(dx*speed*dt,dy*speed*dt));x=after.x;y=after.y;
 moving=distance(before,after)>.01f;if(moving)anim+=dt;else anim=0;
}

int Game::nearbyHide()const{
 const Navigation& n=roomNavigation(section);
 for(size_t i=0;i<n.covers.size();++i)if(distance(Point(x,y),n.covers[i].hide)<80)return int(i);
 return -1;
}

bool Game::monsterCanSeePlayer()const{
 float dx=x-monsterX,dy=y-monsterY,d=std::sqrt(dx*dx+dy*dy);
 if(d>480)return false;
 if(d>100&&(dx*monsterFacingX+dy*monsterFacingY)/d<.15f)return false;
 return roomNavigation(section).clearSight(Point(monsterX,monsterY),Point(x,y));
}

void Game::moveMonster(float dt,Point target,float speed){
 const Navigation& n=roomNavigation(section);repath-=dt;
 if(repath<=0||distance(Point(monsterX,monsterY),monsterWaypoint)<12){monsterWaypoint=n.nextWaypoint(Point(monsterX,monsterY),target);repath=.25f;}
 float dx=monsterWaypoint.x-monsterX,dy=monsterWaypoint.y-monsterY,d=std::sqrt(dx*dx+dy*dy);
 if(d<2){monsterPose=0;return;}
 dx/=d;dy/=d;
 if(dx*monsterFacingX+dy*monsterFacingY<.1f)turnTime=.18f;
 monsterFacingX=dx;monsterFacingY=dy;
 Point before(monsterX,monsterY);
 Point q=n.slide(before,Point(dx*std::min(d,speed*dt),dy*std::min(d,speed*dt)));
 // A blocked AI actor must never freeze the player/game. Repath aggressively when stalled.
 if(distance(before,q)<.5f){
  repath=0;monsterWaypoint=n.nextWaypoint(before,target);
  float rx=monsterWaypoint.x-before.x,ry=monsterWaypoint.y-before.y,rl=std::sqrt(rx*rx+ry*ry);
  if(rl>1){rx/=rl;ry/=rl;q=n.slide(before,Point(rx*speed*dt,ry*speed*dt));}
 }
 monsterX=q.x;monsterY=q.y;
 monsterPose=turnTime>0?3:monsterState==CHASE?4:1+int(monsterTime*5)%2;
}

void Game::updateReality2(float dt,const Input& in){
 const Navigation& n=roomNavigation(section);monsterTime+=dt;turnTime=std::max(0.0f,turnTime-dt);aiTime+=dt;
 if(monsterState==CATCH){monsterPose=5;if(aiTime>.45f)fail(R2);return;}

 // Player input is always processed independently from monster pathfinding/state.
 if(hideState==EXPOSED){
  navigatePlayer(dt,in,Tuning::room(),n);
  if(in.hide&&(hideIndex=nearbyHide())>=0){hideWitnessed=monsterCanSeePlayer();if(hideWitnessed){monsterState=CHASE;lastSeen=Point(x,y);lostSight=0;aiTime=0;}hideState=HIDE_ENTER;hideTime=0;}
 }else{
  hideTime+=dt;moving=false;
  if(hideState==HIDE_ENTER){
   Point target=n.covers[hideIndex].hide;
   Point q=n.slide(Point(x,y),Point((target.x-x)*std::min(1.0f,dt*12),(target.y-y)*std::min(1.0f,dt*12)));x=q.x;y=q.y;
   if(hideTime>.32f){hideState=HIDDEN;hideTime=0;}
  }else if(hideState==HIDDEN&&(in.hide||in.w||in.a||in.s||in.d)){hideState=HIDE_EXIT;hideTime=0;}
  else if(hideState==HIDE_EXIT&&hideTime>.18f){hideState=EXPOSED;hideWitnessed=false;}
 }

 if(time>1.1f){
  bool visible=hideState==EXPOSED&&monsterCanSeePlayer();
  if(visible){monsterState=CHASE;lastSeen=Point(x,y);lostSight=0;aiTime=0;}
  else if(monsterState==CHASE){
   lostSight+=dt;
   if(lostSight>.6f){
    monsterState=SEARCH;aiTime=0;searchRoute.clear();
    for(size_t j=0;j<n.nodes.size();++j)if(distance(n.nodes[j],lastSeen)<360)searchRoute.push_back(n.nodes[j]);
   }
  }

  if(monsterState==PATROL){
   Point target=n.patrol[patrolIndex%n.patrol.size()];moveMonster(dt,target,85);
   if(distance(Point(monsterX,monsterY),target)<24)++patrolIndex;
  }else if(monsterState==CHASE)moveMonster(dt,lastSeen,160);
  else{
   Point target=lastSeen;
   if(aiTime>2&&!searchRoute.empty())target=searchRoute[int((aiTime-2)/1.5f)%searchRoute.size()];
   moveMonster(dt,target,100);
   if(aiTime>7){monsterState=PATROL;hideWitnessed=false;}
  }

  float d=distance(Point(x,y),Point(monsterX,monsterY));
  if(d<80&&hideState==EXPOSED&&monsterState!=CATCH){
   // Positional separation avoids an overlap deadlock. It never disables player input.
   float dx=x-monsterX,dy=y-monsterY;if(std::fabs(dx)<.01f&&std::fabs(dy)<.01f)dx=1;
   float len=std::sqrt(dx*dx+dy*dy);dx/=len;dy/=len;
   Point pushed=n.slide(Point(x,y),Point(dx*24,dy*24));x=pushed.x;y=pushed.y;
  }
  if(d<58&&(visible||hideWitnessed)){monsterState=CATCH;aiTime=0;monsterPose=5;}
 }

 if(hideState==EXPOSED){
  if(section==0&&x>1405)start(R2,1);
  else if(section==1&&x>1425)start(R2,2);
  else if(section==2&&distance(Point(x,y),Point(1380,780))<105&&in.enter)start(R3);
 }
}
}
