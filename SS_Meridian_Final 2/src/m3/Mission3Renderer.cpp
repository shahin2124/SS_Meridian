#include "Mission3Renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>
namespace m3 {
void Renderer::quad(const Texture& t,float x,float y,float w,float h,float alpha) const {
 if(!t.id)return;
 glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,t.id);glColor4f(1,1,1,alpha);
 glBegin(GL_QUADS);glTexCoord2f(0,0);glVertex2f(x,y);glTexCoord2f(t.u,0);glVertex2f(x+w,y);glTexCoord2f(t.u,t.v);glVertex2f(x+w,y+h);glTexCoord2f(0,t.v);glVertex2f(x,y+h);glEnd();glDisable(GL_TEXTURE_2D);
}
void Renderer::fit(Art a,float x,float y,float w,float h) const {const Texture& t=assets.get(a);if(!t.id||t.width<=0||t.height<=0)return;float scale=std::min(w/t.width,h/t.height);quad(t,x+(w-t.width*scale)*.5f,y+(h-t.height*scale)*.5f,t.width*scale,t.height*scale);}
void Renderer::background(Art a) const {fit(a,0,0,1280,720);}
void Renderer::rect(float x,float y,float w,float h,float r,float g,float b,float alpha) const {glDisable(GL_TEXTURE_2D);glColor4f(r,g,b,alpha);glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();}
static float textWidth(const char* s,float size){float w=0;for(;*s;++s)w+=glutStrokeWidth(GLUT_STROKE_ROMAN,*s);return w*size/119.05f;}
void Renderer::text(const char* s,float x,float y,float size,float r,float g,float b) const {
 glDisable(GL_TEXTURE_2D);glColor4f(r,g,b,1);glPushMatrix();glTranslatef(x,y,0);glScalef(size/119.05f,-size/119.05f,1);glLineWidth(1.5f);for(;*s;++s)glutStrokeCharacter(GLUT_STROKE_ROMAN,*s);glPopMatrix();
}
void Renderer::center(const char* s,float x,float y,float size) const {text(s,x-textWidth(s,size)*.5f,y,size);}
void Renderer::wrap(const char* s,float x,float y,float width,float size) const {
 std::istringstream words(s);std::string word,line;
 while(words>>word){std::string next=line.empty()?word:line+" "+word;if(textWidth(next.c_str(),size)>width&&!line.empty()){text(line.c_str(),x,y,size,.16f,.085f,.035f);y+=size*1.45f;line=word;}else line=next;}
 if(!line.empty())text(line.c_str(),x,y,size,.16f,.085f,.035f);
}
struct Registration {float x,y,bodyPixels;};
// Source sheets were authored at different body sizes. These fixed, hand-inspected
// sequence registrations map torso/feet into common character units. No per-frame
// bounds-fitting, no mirroring, no anchor on an effect/weapon/spine tip. Scale is
// fixed for each character in world units; poses retain their genuine silhouette.
static Registration registration(Sequence s){
 switch(s){
 case S_PlayerIdle:{Registration r={165,656,626};return r;}
 case S_PlayerForward:{Registration r={165,648,607};return r;}
 case S_PlayerBackward:{Registration r={190,700,684};return r;}
 case S_PlayerJump:{Registration r={195,673,620};return r;}
 case S_PlayerGuard:{Registration r={200,668,622};return r;}
 case S_PlayerKick:{Registration r={215,668,620};return r;}
 case S_PlayerHit:{Registration r={300,643,535};return r;}
 case S_PlayerFire:{Registration r={235,666,524};return r;}
 case S_PlayerWeapon:{Registration r={270,650,400};return r;}
 case S_Monster1Advance:{Registration r={270,573,422};return r;}
 case S_Monster1AttackA:{Registration r={335,590,306};return r;}
 case S_Monster1AttackB:{Registration r={225,664,434};return r;}
 case S_Monster1Hit:{Registration r={235,665,561};return r;}
 case S_Monster1Dead:{Registration r={265,568,380};return r;}
 case S_Monster1Cast:{Registration r={215,610,486};return r;}
 case S_Monster2Advance:{Registration r={275,637,528};return r;}
 case S_Monster2AttackA:{Registration r={420,602,410};return r;}
 case S_Monster2AttackB:{Registration r={310,637,528};return r;}
 case S_Monster2Hit:{Registration r={255,650,593};return r;}
 case S_Monster2Dead:{Registration r={330,638,493};return r;}
 case S_Monster2Cast:{Registration r={290,655,608};return r;}
 case S_Monster3Idle:{Registration r={200,658,560};return r;}
 case S_Monster3Advance:{Registration r={270,543,375};return r;}
 case S_Monster3AttackA:{Registration r={310,644,400};return r;}
 case S_Monster3AttackB:{Registration r={375,652,480};return r;}
 case S_Monster3Hit:{Registration r={245,658,520};return r;}
 case S_Monster3Dead:{Registration r={265,541,370};return r;}
 case S_Monster3Cast:{Registration r={315,650,430};return r;}
 default:{Registration r={0,0,1};return r;}
 }
}
static Sequence sequence(const Fighter& f,int who){
 if(who==0){switch(f.action){case Forward:return S_PlayerForward;case Backward:return S_PlayerBackward;case Jump:return S_PlayerJump;case Guard:return S_PlayerGuard;case Kick:return S_PlayerKick;case Weapon:return S_PlayerWeapon;case Fire:return S_PlayerFire;case Hit:return S_PlayerHit;default:return S_PlayerIdle;}}
 const Sequence advance[]={S_Monster1Advance,S_Monster2Advance,S_Monster3Advance};
 const Sequence a[]={S_Monster1AttackA,S_Monster2AttackA,S_Monster3AttackA};
 const Sequence b[]={S_Monster1AttackB,S_Monster2AttackB,S_Monster3AttackB};
 const Sequence hit[]={S_Monster1Hit,S_Monster2Hit,S_Monster3Hit};
 const Sequence dead[]={S_Monster1Dead,S_Monster2Dead,S_Monster3Dead};
 const Sequence cast[]={S_Monster1Cast,S_Monster2Cast,S_Monster3Cast};int i=who-1;
 switch(f.action){case AttackA:return a[i];case AttackB:return b[i];case Hit:return hit[i];case Dead:return dead[i];case Telegraph:return cast[i];case Advance:return advance[i];default:return who==3?S_Monster3Idle:advance[i];}
}
void Renderer::fighter(const Mission& g,const Fighter& f,int who,float x,float ground,float zoom) const {
 Sequence s=sequence(f,who);int frame=g.frame(f,who!=0);
 // Intentional supplied-asset fallback: Monster 1/2 have NO idle packages.
 // Hold advance_01 stationary. Never animate the walk cycle at rest.
 if((who==1||who==2)&&(f.action==Idle||f.action==Recovery))frame=0;
 Registration r=registration(s);
 // Hand-registered ground contacts for the collapse; no body rescaling.
 if(s==S_Monster1Dead){const float feet[6]={562,562,548,576,575,603};r.y=feet[frame];}
 float bodyHeight=who==0?370.0f:(who==3?370.0f:390.0f);
 float scale=bodyHeight/r.bodyPixels*zoom;const Texture& t=assets.get(s,frame);
 quad(t,x-r.x*scale,ground-r.y*scale,t.width*scale,t.height*scale);
}
void Renderer::cages(const Mission& g,float shift,bool release) const {
 // Single combined three-cage image; original silhouette order is preserved.
 float x=550+shift,y=295,w=640,h=w*941.0f/1672;
 quad(assets.get(A_Cages),x,y,w,h);
 for(int i=0;i<3;++i)if(!g.progress.defeated[i]){Fighter f=g.monsters[i];f.action=Idle;f.time=0;fighter(g,f,i+1,Layout::cageX(i)+shift,588,.40f);}
 // The combined supplied cage art has opaque interiors as well as bars. A second
 // translucent pass preserves its complete foreground and keeps each occupant legible.
 quad(assets.get(A_Cages),x,y,w,h,.58f);
 if(release){int f=std::min(5,static_cast<int>(g.stateTime/1.2f*6));
  // Replace only the selected cage region: clear its bars using arena pixels,
  // then re-render its occupant behind the supplied single-door opening sequence.
  float cx=Layout::cageX(g.current)+shift;
  glEnable(GL_SCISSOR_TEST);
  // draw() sets a logical viewport; scissor conversion is derived from it.
  GLint vp[4];glGetIntegerv(GL_VIEWPORT,vp);
  glScissor(vp[0]+static_cast<int>((cx-88)*vp[2]/1280),vp[1]+static_cast<int>((720-628)*vp[3]/720),static_cast<int>(176*vp[2]/1280),static_cast<int>(326*vp[3]/720));
  background(A_FightBG);Fighter still=g.monsters[g.current];still.action=Idle;fighter(g,still,g.current+1,cx,588,.40f);glDisable(GL_SCISSOR_TEST);
  const Texture& t=assets.get(S_Cage,f);float scale=.60f;quad(t,cx-183*scale,628-500*scale,t.width*scale,t.height*scale);
 }
}
void Renderer::hud(const Mission& g) const {
 // Fill is beneath the transparent bar apertures in the original HUD frame.
 rect(147,72,393,20,.10f,.035f,.025f);rect(739,72,393,20,.10f,.035f,.025f);
 rect(147,72,393*g.player.health/g.config.playerHealthForFight(g.current),20,.26f,.69f,.30f);
 float ratio=g.monsters[g.current].health/g.config.monsterHealth[g.current];rect(1132-393*ratio,72,393*ratio,20,.73f,.16f,.12f);
 background(A_HUD);
 char line[96];std::sprintf(line,"ISSAC  %.0f / %.0f",g.player.health,g.config.playerHealthForFight(g.current));text(line,158,58,17);
 std::sprintf(line,"MONSTER %d  %.0f / %.0f",g.current+1,g.monsters[g.current].health,g.config.monsterHealth[g.current]);text(line,770,58,17);
 if(g.config.showTimer){std::sprintf(line,"%02d",static_cast<int>(std::ceil(g.remaining)));center(line,640,89,25);}
 rect(0,679,1280,41,.02f,.018f,.02f,.82f);center("A / D  MOVE     W  JUMP     SHIFT  GUARD     J  KICK     K  SPACE WEAPON     L  MIND FIRE",640,706,16);
 if(g.monsters[g.current].action==Telegraph)center(g.monsters[g.current].variant==0?"HEAVY ATTACK - MAKE SPACE":"SECONDARY ATTACK - EVADE",640,171,20);
}
void Renderer::button(const Mission& g) const {
 Art id=A_ClaimNormal;
 if(g.buttonDown&&g.buttonHover)id=A_ClaimPressed;else if(g.buttonHover)id=A_ClaimHover;
 // Transparent padding differs across the supplied button states. Proportional
 // fitting keeps all artwork visible; click geometry is stable across states.
 Rect r=g.buttonRect();const Texture& t=assets.get(id);
 float s=std::min(r.w/(t.right-t.left),r.h/(t.bottom-t.top));
 quad(t,r.x+(r.w-(t.right-t.left)*s)*.5f-t.left*s,r.y+(r.h-(t.bottom-t.top)*s)*.5f-t.top*s,t.width*s,t.height*s);
 center("CLAIM",640,554,27);
}
void Renderer::draw(const Mission& g,int width,int height) const {
 glViewport(0,0,width,height);glClearColor(.025f,.018f,.025f,1);glClear(GL_COLOR_BUFFER_BIT);
 float scale=std::min(width/1280.0f,height/720.0f);int vw=static_cast<int>(1280*scale),vh=static_cast<int>(720*scale);
 glViewport((width-vw)/2,(height-vh)/2,vw,vh);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,1280,720,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
 glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 if(!assets.errors.empty()){center("ASSET LOAD FAILED",640,200,34);text(assets.errors[0].c_str(),30,260,13);center("See asset_errors.txt for exact paths. Close and restore the files.",640,330,19);return;}
 if(g.state>=FinalArenaEnter){ending(g);return;}
 if(g.state==Enter){background(A_FightBG);rect(0,0,1280,720,0,0,0,.60f);center("SS MERIDIAN",640,215,28);center("MISSION 3",640,325,65);center("SPACE STONE + MIND STONE OWNED",640,388,20);Rect r=g.buttonRect();rect(r.x,r.y,r.w,r.h,g.buttonHover?.48f:.28f,.12f,.055f);center("ENTER",640,554,31);center("Click ENTER or press Enter",640,630,18);return;}
 if(g.state==Intro||g.state==Walk||g.state==TriggerReady){
  const Texture& t=assets.get(A_IntroBG);float w=t.width*(720.0f/t.height);float progress=g.walkTravel/g.config.walkDistance;
  quad(t,-progress*(w-1280),0,w,720);float shift=(1-progress)*1300;
  cages(g,shift,false);
  fit(A_Trigger,Layout::TriggerX-115+shift,603,230,100);
  fighter(g,g.player,0,g.player.x,Layout::Ground,.8f);
  rect(0,0,1280,61,0,0,0,.64f);center(g.state==TriggerReady?"STEP ON THE RED CIRCLE":"A / D  -  WALK FORWARD TO THE CAGES",640,40,23);
  if(g.state==Intro)rect(0,0,1280,720,0,0,0,1-g.stateTime/g.config.introDuration);return;
 }
 if(g.state==Guide){background(static_cast<Art>(A_Guide1+g.current));const GuideData& d=Guides[g.current];wrap(d.power,767,108,445,22);wrap(d.attack,767,250,445,21);wrap(d.dodge,767,393,445,21);wrap(d.defeat,767,543,445,21);return;}
 if(g.state==CageOpen){background(A_FightBG);cages(g,0,true);fighter(g,g.player,0,240,Layout::Ground,.8f);return;}
 if(g.state==Fight||g.state==Defeat||g.state==Failed){
  background(A_FightBG);fighter(g,g.player,0,g.player.x,Layout::Ground-g.jumpOffset());fighter(g,g.monsters[g.current],g.current+1,g.monsters[g.current].x,Layout::Ground);
  for(int i=0;i<4;++i)if(g.projectiles[i].active){const Projectile& p=g.projectiles[i];const Texture& t=assets.get(S_Projectile,static_cast<int>(p.time/.08f)%6);quad(t,p.x-107,p.y-60,168,120);}
  const Sequence effectSeq[3]={S_Spark,S_Impact,S_Dust};
  for(int i=0;i<8;++i)if(g.effects[i].active){const Effect& e=g.effects[i];const Texture& t=assets.get(effectSeq[e.kind],std::min(5,static_cast<int>(e.time/.09f)));float s=.45f;quad(t,e.x-t.width*s*.5f,e.y-(e.kind==2?t.height*s:t.height*s*.5f),t.width*s,t.height*s);}
  hud(g);
  if(g.state==Failed){rect(0,0,1280,720,0,0,0,.72f);center("FIGHT FAILED",640,320,48);center("PRESS ENTER TO RETRY",640,398,26);}return;
 }
 if(g.state==Complete){background(A_CompleteBG);rect(530,160,700,270,0,0,0,.5f);center("MISSION 3 COMPLETE",880,272,36);center("ALL THREE MONSTERS DEFEATED",880,335,20);return;}
 if(g.state==Reward||g.state==Claimed){background(A_RewardBG);rect(0,0,1280,170,0,0,0,.5f);center("POWER STONE",640,107,44);
  const Texture& t=assets.get(S_Aura,static_cast<int>(g.stateTime/.12f)%6);float s=.45f;quad(t,640-t.width*s*.5f,315-t.height*s*.5f,t.width*s,t.height*s);
  // Proportionally center the supplied stone's visible content within the aura.
  const Texture& stone=assets.get(A_PowerStone);
  float stoneScale=std::min(62.0f/(stone.right-stone.left),78.0f/(stone.bottom-stone.top));
  quad(stone,640-(stone.left+stone.right)*.5f*stoneScale,315-(stone.top+stone.bottom)*.5f*stoneScale,stone.width*stoneScale,stone.height*stoneScale);
  if(g.state==Reward){button(g);center("Click CLAIM to receive the Power Stone",640,644,21);}else center("POWER STONE CLAIMED",640,554,29);return;
 }
}
}

namespace m3 {
void Renderer::ending(const Mission& g) const {
 const float t=g.stateTime;
 if(g.state<=FinalBlackFade){
  background(A_FinalArena);quad(assets.get(A_FinalEye),594,34,92,181);
  // All Hand/VFX canvases use one rectangle, including 1535/1536 pixel heights.
  const float hx=510,hy=195,hw=260,hh=390;
  Art hand=A_HandEmpty;
  if(g.state==FinalSpace&&t>=1.2f)hand=A_HandSpace;
  if(g.state==FinalMind)hand=t<1.2f?A_HandSpace:A_HandMind;
  if(g.state==FinalPower)hand=t<1.2f?A_HandMind:A_HandComplete;
  if(g.state>=FinalGlow)hand=A_HandComplete;
  quad(assets.get(hand),hx,hy,hw,hh);
  quad(assets.get(A_FinalIssac),295,260,210,315);quad(assets.get(A_FinalGuide),790,240,223,335);
  background(A_FinalArenaFG);
  if(g.state==FinalDialogue){
   quad(assets.get(A_FinalPanel),90,490,1100,240);
   const char* lines[]={"I collected the three stones.","Then your journey is nearly complete.","Submit them to the Hand of Three.","Space. Mind. Power. One by one."};
   center(g.finalDialogue==0?"ISSAC":"GUIDE",640,625,19);center(lines[g.finalDialogue],640,658,24);center("PRESS ENTER",640,685,15);
  }
  if(g.state>=FinalSpace&&g.state<=FinalPower){
   Sequence fx=static_cast<Sequence>(S_SpaceInsert+g.state-FinalSpace);
   if(t<1.2f)quad(assets.get(fx,std::min(5,static_cast<int>(t/.2f))),hx,hy,hw,hh);
   const char* labels[]={"SPACE STONE","MIND STONE","POWER STONE"};center(labels[g.state-FinalSpace],640,653,25);
  }
  if(g.state==FinalGlow||g.state==FinalBlackFade)quad(assets.get(S_HandGlow,g.state==FinalBlackFade?5:std::min(5,static_cast<int>(t/.2f))),hx,hy,hw,hh);
  if(g.state==FinalArenaEnter)rect(0,0,1280,720,0,0,0,std::max(0.0f,1-t));
  if(g.state==FinalBlackFade)rect(0,0,1280,720,0,0,0,std::min(1.0f,t/1.5f));return;
 }
 if(g.state==FinalShipCount){rect(0,0,1280,720,0,0,0);char n[2]={static_cast<char>('1'+std::min(2,static_cast<int>(t))),0};center(n,640,405,100);return;}
 if(g.state>=FinalShipWalk&&g.state<=FinalDoorWait){
  background(A_Ship);
  // Overlay the existing opening sequence on the actual LEFT-side doorway.
  int door=g.state==FinalShipWalk?0:(g.state==FinalDoorWait?5:std::min(5,static_cast<int>(t/.2f)));
  quad(assets.get(S_Door,door),45,20,205,371);
  text("HOMECOMING",147.5f-textWidth("HOMECOMING",12)*.5f,85,12,.20f,.09f,.035f);
  float travel=std::max(0.0f,std::min(1.0f,g.shipProgress));
  int walk=static_cast<int>(g.shipWalkTime/.13f)%6;
  // Alpha>8 boot-bottom rows measured on the supplied 362x724 canvases.
  // Horizontal flips preserve every pixel. Fixed scale; compensate only Y.
  const float feet[6]={660,665,659,662,666,663};
  const float scale=.34f;
  // Two connected deck segments pass below the left pillar before the door.
  float leg=travel<.8f?travel/.8f:(travel-.8f)/.2f;
  float footX=travel<.8f?920-640*leg:280-87*leg;
  float footY=travel<.8f?585-160*leg:425-75*leg;
  quad(assets.get(S_ShipWalk,walk),footX-181*scale,footY-feet[walk]*scale,362*scale,724*scale);
  if(g.state==FinalShipWalk)center("HOLD W TO WALK TO HOMECOMING",640,657,22);
  if(g.state==FinalDoorWait){rect(440,619,400,57,0,0,0,.65f);center("PRESS ENTER",640,657,27);}
  if(g.state==FinalShipWalk)rect(0,0,1280,720,0,0,0,std::max(0.0f,1-t));return;
 }
 if(g.state>=FinalBeachWalk&&g.state<=FinalBeachFade){
  background(A_Beach);quad(assets.get(A_Sun),856,338-24.0f*(1.0f-std::exp(-g.beachTime/20.0f)),128,96);
  int loop=static_cast<int>(g.beachTime/.22f)%6;
  quad(assets.get(S_Reflection,loop),0,0,1280,720);quad(assets.get(S_Waves,loop),0,0,1280,720,.12f);
  float x=180+490*std::max(0.0f,std::min(1.0f,g.beachProgress));
  if(g.state==FinalBeachWalk)quad(assets.get(S_BeachWalk,static_cast<int>(g.beachWalkTime/.14f)%6),x,455,95,180);
  else quad(assets.get(A_Watching),x-12,451,120,180);
  if(g.state==FinalBeachWalk)rect(0,0,1280,720,0,0,0,std::max(0.0f,1-t));
  if(g.state==FinalBeachWalk)center("HOLD W TO WALK TO THE SHORE",640,690,19);
  if(g.state==FinalReflection){
   static const char* lines[]={
    "So much happened before I reached this shore.",
    "I crossed impossible places and survived what waited there.",
    "The Space Stone carried me through broken realities.",
    "The dragon guarded the Mind Stone. I survived it.",
    "The colosseum demanded everything I had for the Power Stone.",
    "Three stones. Three trials. One journey.",
    "And now... I am finally home."
   };
   const char* line=lines[std::max(0,std::min(6,g.reflectionLine))];
   float tx=750-textWidth(line,20)*.5f;
   text(line,tx+1.5f,131.5f,20,.18f,.12f,.10f);text(line,tx,130,20,1,.96f,.85f);
   text("PRESS ENTER",750-textWidth("PRESS ENTER",14)*.5f+1,161,14,.18f,.12f,.10f);
   text("PRESS ENTER",750-textWidth("PRESS ENTER",14)*.5f,160,14,1,.96f,.85f);
  }
  if(g.state==FinalBeachWatch){char n[2]={static_cast<char>('1'+std::min(2,static_cast<int>(t))),0};center(n,640,230,80);}
  if(g.state==FinalBeachFade)rect(0,0,1280,720,0,0,0,std::min(1.0f,t/.8f));return;
 }
 if(g.state==FinalJourney){background(A_Journey);text("THE JOURNEY OF ISSAC",640-textWidth("THE JOURNEY OF ISSAC",25)*.5f,92,25,.20f,.09f,.035f);center("END",640,695,22);rect(0,0,1280,720,0,0,0,std::max(0.0f,1-t/.8f));}
}
}
