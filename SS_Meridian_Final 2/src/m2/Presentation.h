#pragma once
#include "Game.h"
#include "Assets.h"
#include <string>
#include <cstdio>
namespace m2 {
struct GlyphInk {int left,top,right,bottom;};
// Bounds come from the exact GDI glyph atlas used to render, in its 36px font space.
inline Point centeredInkOrigin(const std::string& text, Point center, int size,
                               const int* advances, const GlyphInk* ink, int ascent) {
 int pen=0,left=0,right=0,top=0,bottom=0;bool found=false;
 for(size_t i=0;i<text.size();++i){int n=static_cast<unsigned char>(text[i])-32;if(n<0||n>=96)continue;
  const GlyphInk& g=ink[n];
  if(g.right>g.left&&g.bottom>g.top){
   if(!found){left=pen+g.left;right=pen+g.right;top=g.top;bottom=g.bottom;found=true;}
   else{left=std::min(left,pen+g.left);right=std::max(right,pen+g.right);top=std::min(top,g.top);bottom=std::max(bottom,g.bottom);}
  }pen+=advances[n];
 }
 float scale=size/36.f;
 return Point(center.x-(left+right)*.5f*scale,
              center.y+((top+bottom)*.5f-ascent)*scale);
}

struct Draw {int asset;Rect box;float alpha,angle;bool crop;std::string text;int size;float r,g,b;bool centerInk;};
struct Presentation {
 std::vector<Draw> list;Point mouse;bool down;Presentation():down(false){}
 void image(int id,float x,float y,float w,float h,float alpha=1,bool crop=false,float angle=0){Draw d={id,{x,y,w,h},alpha,angle,crop,"",20,1,1,1,false};list.push_back(d);}
 void rect(float x,float y,float w,float h,float r,float g,float b,float a=1){Draw d={-1,{x,y,w,h},a,0,false,"",20,r,g,b,false};list.push_back(d);}
 void text(const std::string&s,float x,float y,int size=22,float r=1,float g=.88f,float b=.64f){Draw d={-2,{x,y,0,0},1,0,false,s,size,r,g,b,false};list.push_back(d);}
 void center(const std::string&s,float x,float y,int size=22){text(s,x-float(s.size())*size*.29f,y,size);}
 // Entry PNG frame bounds measured once at alpha > 16/255; lower-alpha fringe
 // is preserved in the full texture but excluded from the visible-frame center.
 // The user confirmed this is the button's OWN frame, not a background panel.
 Rect entrySourceFrame(int id)const{
  if(id==MISSION2_ENTER_BUTTON_HOVER){Rect r={104,44,301,73};return r;}
  if(id==MISSION2_ENTER_BUTTON_PRESSED){Rect r={153,56,282,56};return r;}
  Rect r={99,46,366,68};return r;
 }
 Point entryFrameCenter()const{return Point(640,90);}
 Rect entryVisibleFrame(int id)const{
  const AssetInfo&a=assetInfo[id];Rect ink=entrySourceFrame(id);Point c=entryFrameCenter();
  // Preserve the pre-patch per-state pixel scale; compensate padding by translation.
  float sx=360.f/(a.r-a.l),sy=62.f/(a.b-a.t);
  Rect r={c.x-ink.w*sx/2,c.y-ink.h*sy/2,ink.w*sx,ink.h*sy};return r;
 }
 Rect entryButtonRegion()const{return entryVisibleFrame(MISSION2_ENTER_BUTTON_NORMAL);}
 Rect entryCanvas(int id)const{
  const AssetInfo&a=assetInfo[id];Rect ink=entrySourceFrame(id),visible=entryVisibleFrame(id);
  float sx=360.f/(a.r-a.l),sy=62.f/(a.b-a.t);
  Rect r={visible.x-ink.x*sx,visible.y-ink.y*sy,a.w*sx,a.h*sy};return r;
 }
 Rect buttonBox(State s)const{Rect r={460,65,360,65};if(s==M2_MIND_STONE_REWARD||s==M2_MISSION3_UNLOCK){r.x=500;r.w=280;r.h=104;}if(s==M2_FIGHT_GUIDE){r.x=770;r.y=95;r.w=360;r.h=65;}if(s==M2_START)return entryButtonRegion();return r;}
 bool hasButton(State s)const{return s==M2_START||s==M2_FIGHT_GUIDE||s==M2_LOST||s==M2_MIND_STONE_REWARD||s==M2_MISSION3_UNLOCK;}
 void button(State s,int normal,int hover,int pressed,const char*label){Rect r=buttonBox(s);bool over=r.contains(mouse);int id=over?(down?pressed:hover):normal;
  if(s==M2_START){Rect canvas=entryCanvas(id);image(id,canvas.x,canvas.y,canvas.w,canvas.h,1,false);
   Point c=entryFrameCenter();text(label,c.x,c.y,24);list.back().centerInk=true;
  }else{image(id,r.x,r.y,r.w,r.h,1,true);center(label,r.x+r.w/2,r.y+r.h/2-8,24);}
 }
 void effect(int base,float age,float duration,float cx,float cy,float w,float h,bool loop=false){image(base+frame(age,duration,loop),cx-w/2,cy-h/2,w,h);}
 void battle(const Game&g){
 image(MISSION2_DRAGON_BATTLE_BACKGROUND,0,0,1280,720);
 float fade=g.state==M2_DRAGON_DEFEATED?std::max(0.f,1-g.stateTime/1.2f):1;
 image(MISSION2_DRAGON_BOSS_IDLE,590,174,650,376,fade);
 for(int i=0;i<3;++i)if(!g.heads[i]){Point p=head(i);effect(DRAGON_HEAD_DEFEATED_LOOP_01,g.elapsed, .6f,p.x,p.y,95,155,true);}
 for(size_t i=0;i<g.attacks.size();++i){const Attack&a=g.attacks[i];Point p=head(a.head);
 if(a.age<.35f)effect(DRAGON_MOUTH_CHARGE_VFX_01,a.age,.35f,p.x,p.y,85,125);
 if(a.age>=.05f&&a.age<.95f)effect(DRAGON_GROUND_WARNING_VFX_01,std::min(.449f,a.age-.05f),.45f,a.x,205,150,100);
 if(a.age>=.45f&&a.age<.95f){float u=(a.age-.45f)/.5f;image(DRAGON_FIREBALL_PROJECTILE,p.x+(a.x-p.x)*u-40,p.y+(210-p.y)*u-28,80,56);}
 if(a.age>=.95f)effect(DRAGON_FIRE_IMPACT_VFX_01,a.age-.95f,.36f,a.x,235,135,200);
 }
 int player=ISSAC_BOW_READY;if(g.move<0)player=ISSAC_BOW_STRAFE_LEFT_01+frame(g.anim,.54f,true);if(g.move>0)player=ISSAC_BOW_STRAFE_RIGHT_01+frame(g.anim,.54f,true);
 // Per-frame alpha bounds stabilize the feet without modifying supplied pixels.
 const AssetInfo&a=assetInfo[player];float scale=330.f/(a.b-a.t);float w=a.w*scale;float h=a.h*scale;
 image(player,g.x+250-w*.5f,175-(a.h-a.b)*scale,w,h);
 for(size_t i=0;i<g.arrows.size();++i){const Arrow&a=g.arrows[i];float angle=std::atan2(a.to.y-a.from.y,a.to.x-a.from.x)*57.29578f-29;image(ISSAC_ARROW_PROJECTILE,a.pos.x-65,a.pos.y-40,100,80,1,false,angle);}
 for(size_t i=0;i<g.effects.size();++i){const Effect&e=g.effects[i];effect(e.kind?ARROW_HEAD_HIT_VFX_01:BOW_RELEASE_VFX_01,e.age,.36f,e.pos.x,e.pos.y,95,110);}
 if(g.state==M2_DRAGON_DEFEATED)for(int i=0;i<3;++i)effect(DRAGON_DEATH_VFX_01,g.stateTime,1.2f,740+i*155,340,230,410);
 image(MISSION2_DRAGON_BATTLE_FOREGROUND,0,0,1280,720);
 rect(110,644,180.f*g.hp/ISSAC_MAX_HP,20,.55f,.1f,.08f);
 for(int i=0;i<3;++i)rect(431+i*224,618,120*g.heads[i]/100.f,20,.55f,.12f,.08f);
 image(MISSION2_COMBAT_HUD,0,0,1280,720);
 char buf[80];std::sprintf(buf,"ISSAC  %d / %d",g.hp,ISSAC_MAX_HP);text(buf,115,650,17);
 const char* labels[]={"LEFT","CENTER","RIGHT"};for(int i=0;i<3;++i){std::sprintf(buf,"%s %d/100",labels[i],g.heads[i]);text(buf,422+i*224,624,16);}
 int t=int(std::ceil(g.time));std::sprintf(buf,"%02d:%02d",t/60,t%60);text(buf,1090,650,23);
 std::sprintf(buf,"AMMO %d / 3%s",g.ammo,g.reload>0?"  RELOADING":"");rect(350,24,580,40,0,0,0,.7f);center(buf,640,37,22);
 text("A / D  MOVE + DODGE     SPACE  FIRE     R  RELOAD",335,90,20);
 if(g.state==M2_DRAGON_FIGHT&&g.target()>=0){Point p=head(g.target());image(MISSION2_AIM_RETICLE,p.x-40,p.y-40,80,80,1,true);}
 }
 void draw(const Game&g){list.clear();switch(g.state){
 case M2_START:image(MISSION2_ENTER_SCREEN,0,0,1280,720);rect(35,545,510,135,0,0,0,.65f);text("MISSION 2",60,620,36);text("THE THREE-HEADED DRAGON",60,573,27);button(g.state,MISSION2_ENTER_BUTTON_NORMAL,MISSION2_ENTER_BUTTON_HOVER,MISSION2_ENTER_BUTTON_PRESSED,"ENTER");break;
 case M2_BATTLE_INTRO:image(MISSION2_DRAGON_INTRO_BACKGROUND,0,0,1280,720);center("THE SANCTUM AWAITS",640,580,34);break;
 case M2_FIGHT_GUIDE:image(MISSION2_FIGHT_GUIDE_BACKGROUND,0,0,1280,720);text("HOW TO FIGHT",770,530,28,.12f,.1f,.07f);{
 const char* lines[]={"DESTROY ALL THREE HEADS","EACH HEAD: 100 HP","BODY SHOTS DO NOT COUNT","A / D - MOVE + DODGE","SPACE - FIRE     R - RELOAD","MAGAZINE: 3 ARROWS","TIME LIMIT: 03:00","Move left / middle / right","to target a living head."};for(int i=0;i<9;++i)text(lines[i],770,488-i*29,20,.12f,.1f,.07f);}
 button(g.state,START_FIGHT_BUTTON_NORMAL,START_FIGHT_BUTTON_HOVER,START_FIGHT_BUTTON_PRESSED,"START FIGHT");break;
 case M2_DRAGON_FIGHT:case M2_DRAGON_DEFEATED:case M2_LOST:battle(g);if(g.state==M2_LOST){rect(0,0,1280,720,0,0,0,.73f);center("MISSION FAILED",640,430,44);center(g.hp<=0?"Issac has fallen.":"The time has run out.",640,375,24);button(g.state,RETRY_BUTTON_NORMAL,RETRY_BUTTON_HOVER,RETRY_BUTTON_PRESSED,"RETRY");}break;
 case M2_MIND_STONE_REWARD:image(MIND_STONE_REWARD_BG,0,0,1280,720);effect(MIND_STONE_AURA_01,g.stateTime,.9f,640,360,360,470,true);image(MIND_STONE,525,267,230,186);center("THE MIND STONE",640,590,36);button(g.state,CLAIM_BUTTON_NORMAL,CLAIM_BUTTON_HOVER,CLAIM_BUTTON_PRESSED,"CLAIM");break;
 case M2_MISSION3_UNLOCK:case M2_HANDOFF_END:image(MISSION3_UNLOCK_BACKGROUND,0,0,1280,720);center("MISSION 3 UNLOCKED",640,590,36);if(g.state==M2_MISSION3_UNLOCK)button(g.state,ENTER_BUTTON_NORMAL,ENTER_BUTTON_HOVER,ENTER_BUTTON_PRESSED,"ENTER");else{rect(270,245,740,180,0,0,0,.8f);center("MISSION 2 COMPLETE",640,365,32);center("Mind Stone claimed. Mission 3 is ready.",640,310,24);center("Standalone handoff complete. ESC to close.",640,270,22);}break;
 }
 }
};
}
