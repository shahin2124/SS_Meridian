#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
namespace m2 {
static const int ISSAC_MAX_HP = 1000;
enum State { M2_START, M2_BATTLE_INTRO, M2_FIGHT_GUIDE, M2_DRAGON_FIGHT, M2_LOST, M2_DRAGON_DEFEATED, M2_MIND_STONE_REWARD, M2_MISSION3_UNLOCK, M2_HANDOFF_END };
struct Point { float x,y; Point(float a=0,float b=0):x(a),y(b){} };
struct Rect { float x,y,w,h; bool contains(Point p)const{return p.x>=x&&p.x<=x+w&&p.y>=y&&p.y<=y+h;} };
// All geometry is in 1280x720 bottom-left coordinates, measured from approved art.
inline Point head(int i){ const Point h[3]={Point(682,407),Point(817,440),Point(1008,398)};return h[i]; }
inline Rect skull(int i){Point p=head(i);Rect r={p.x-25,p.y-22,50,44};return r;}
inline float clamp(float v,float a,float b){return std::max(a,std::min(b,v));}
inline int frame(float t,float duration,bool loop){int f=int(t*6/duration);return loop?f%6:std::min(5,f);}
struct Arrow {Point from,to,pos; float age; int target; Arrow(Point a,Point b,int h):from(a),to(b),pos(a),age(0),target(h){} };
struct Attack {int head;float x,age;bool damaged;Attack(int h,float p):head(h),x(p),age(0),damaged(false){} };
struct Effect {int kind;Point pos;float age;Effect(int k,Point p):kind(k),pos(p),age(0){} };
struct Input {bool left,right,fire,reload;Input():left(false),right(false),fire(false),reload(false){} };
struct Viewport {
 int x,y,w,h; void resize(int cw,int ch){float s=std::min(cw/1280.f,ch/720.f);w=std::max(1,int(1280*s));h=std::max(1,int(720*s));x=(cw-w)/2;y=(ch-h)/2;}
 bool containsPixel(int px,int py,int ch)const{return px>=x&&px<x+w&&ch-1-py>=y&&ch-1-py<y+h;}
 Point mouse(int px,int py,int ch)const{return Point((px-x)*1280.f/w,(ch-py-y)*720.f/h);}
};
class Game {
public:
 State state;float stateTime,time,x,cooldown,reload,anim,elapsed,nextAttack;int hp,heads[3],ammo,roundRobin,move,hits,attacksStarted;bool claimed;
 std::vector<Arrow> arrows;std::vector<Attack> attacks;std::vector<Effect> effects;
 Game():state(M2_START),stateTime(0),claimed(false){resetData();}
 void resetData(){time=180;x=20;hp=ISSAC_MAX_HP;for(int i=0;i<3;++i)heads[i]=100;ammo=3;cooldown=reload=anim=elapsed=0;nextAttack=.5f;roundRobin=move=hits=attacksStarted=0;arrows.clear();attacks.clear();effects.clear();claimed=false;}
 void set(State s){state=s;stateTime=0;}
 void activate(){switch(state){case M2_START:set(M2_BATTLE_INTRO);break;case M2_FIGHT_GUIDE:case M2_LOST:resetData();set(M2_DRAGON_FIGHT);break;case M2_MIND_STONE_REWARD:if(!claimed){claimed=true;set(M2_MISSION3_UNLOCK);}break;case M2_MISSION3_UNLOCK:set(M2_HANDOFF_END);break;default:break;}}
 int target()const{int zone=std::min(2,int((x-20)/140));for(int d=0;d<3;++d){int i=(zone+d)%3;if(heads[i]>0)return i;}return -1;}
 Point bow()const{return Point(x+365,425);}
 bool won()const{return heads[0]==0&&heads[1]==0&&heads[2]==0;}
 bool hit(int i,Point p){if(state!=M2_DRAGON_FIGHT||i<0||i>2||heads[i]<=0||!skull(i).contains(p))return false;heads[i]=std::max(0,heads[i]-10);++hits;effects.push_back(Effect(1,p));return true;}
 void victory(){set(M2_DRAGON_DEFEATED);arrows.clear();attacks.clear();effects.clear();move=0;}
 void tick(float dt,const Input& in){
  stateTime+=dt;
  if(state==M2_BATTLE_INTRO){if(stateTime>=1.25f)set(M2_FIGHT_GUIDE);return;}
  if(state==M2_DRAGON_DEFEATED){if(stateTime>=1.2f)set(M2_MIND_STONE_REWARD);return;}
  if(state!=M2_DRAGON_FIGHT)return;
  if(won()){victory();return;}
  elapsed+=dt;time=std::max(0.f,time-dt);cooldown=std::max(0.f,cooldown-dt);
  int direction=(in.right?1:0)-(in.left?1:0);if(direction!=move)anim=0;move=direction;anim=move?anim+dt:0;x=clamp(x+move*300*dt,20,440);
  if(reload>0){reload=std::max(0.f,reload-dt);if(reload==0)ammo=3;}
  else if(in.reload&&ammo<3)reload=1;
  if(in.fire&&reload==0&&cooldown==0&&ammo>0){int t=target();if(t>=0){arrows.push_back(Arrow(bow(),head(t),t));effects.push_back(Effect(0,bow()));--ammo;cooldown=.4f;}}
  for(size_t i=0;i<effects.size();){effects[i].age+=dt;if(effects[i].age>=.36f)effects.erase(effects.begin()+i);else ++i;}
  for(size_t i=0;i<arrows.size();){Arrow&a=arrows[i];a.age+=dt;float u=std::min(1.f,a.age/.38f);a.pos=Point(a.from.x+(a.to.x-a.from.x)*u,a.from.y+(a.to.y-a.from.y)*u);if(u>=1){hit(a.target,a.pos);arrows.erase(arrows.begin()+i);}else ++i;}
  // Resolve all arrows before timeout or hostile damage: simultaneous final hit wins.
  if(won()){victory();return;}
  if(time<=0){set(M2_LOST);return;}
  if(elapsed+.00001f>=nextAttack){nextAttack+=.65f;if(attacks.size()<3){for(int n=0;n<3;++n){int h=(roundRobin+n)%3;if(heads[h]>0){attacks.push_back(Attack(h,x+250));roundRobin=(h+1)%3;++attacksStarted;break;}}}}
  for(size_t i=0;i<attacks.size();){Attack&a=attacks[i];a.age+=dt;if(a.age>=.95f&&!a.damaged){a.damaged=true;if(std::fabs(x+250-a.x)<66)hp=std::max(0,hp-25);}if(a.age>=1.31f)attacks.erase(attacks.begin()+i);else ++i;}
  if(hp<=0)set(M2_LOST);
 }
};
}
