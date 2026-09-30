#ifndef SS_MERIDIAN_GAMEHOST_H
#define SS_MERIDIAN_GAMEHOST_H

// Implementation header: include only through iMain.cpp (one host translation unit).
#include "m3/Platform.h"
#include "m1/Renderer.h"
#include "m2/Presentation.h"
#include "m3/Mission3Renderer.h"
#include "Integration.h"
#include "stb_image.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <fstream>
#ifdef _WIN32
#include <direct.h>
#include <mmsystem.h>
#endif
#include "Menu.h"
#include "Mission2Host.h"
namespace host {
static int active=0,width=1280,height=720,lastTime=0;
static bool keys[256]={false},edges[256]={false},blocked[256]={false};
static bool visible=true,framePresented=false,startRequested=false,fullscreen=false;
static double accumulator=0;
static bool unlockPreloading=false,unlockEnterPending=false;
static app::Session session;
static Meridian::Game game1;
static Meridian::Input input1;
static Renderer* renderer1=0;
static m3::Mission game3;
static m3::Assets assets3;
static m3::Renderer renderer3(assets3);
static void clearInput(){
 for(int i=0;i<256;++i){blocked[i]=blocked[i]||keys[i];keys[i]=edges[i]=false;}
 std::fill(menu::keyPressed,menu::keyPressed+512,false);std::memset(menu::gPrevKeys,0,sizeof(menu::gPrevKeys));
 input1=Meridian::Input();mission2::input=m2::Input();mission2::pressed=mission2::view.down=false;
 game3.cancelPointer();startRequested=false;accumulator=0;
}
static void release(){
 unlockPreloading=unlockEnterPending=false;
 menu::release();delete renderer1;renderer1=0;mission2::release();assets3.release();
#ifdef _WIN32
 PlaySoundA(0,0,0);
#endif
}
static void launch(int mission,bool direct){
 release();clearInput();framePresented=false;
 if(direct)session.begin(mission);
 active=mission;
 if(mission==0)menu::initialize();
 if(mission==1){game1=Meridian::Game();game1.start(Meridian::ARRIVAL);renderer1=new Renderer;}
 if(mission==2){mission2::initialize();mission2::clientW=width;mission2::clientH=height;mission2::viewport.resize(width,height);}
 if(mission==3){game3=m3::Mission();session.apply(game3);assets3.beginLoad("");}
 lastTime=glutGet(GLUT_ELAPSED_TIME);
}
static void fatal(const char* message){
 std::ofstream f("startup_error.txt");f<<message<<"\n";f.close();std::fprintf(stderr,"%s\n",message);
#ifdef _WIN32
 MessageBoxA(0,message,"SS Meridian",MB_OK|MB_ICONERROR);
#endif
 release();std::exit(1);
}
static void projection(float w,float h,bool top){
 float scale=std::min(width/w,height/h);int vw=std::max(1,int(w*scale)),vh=std::max(1,int(h*scale));
 glViewport((width-vw)/2,(height-vh)/2,vw,vh);glMatrixMode(GL_PROJECTION);glLoadIdentity();
 glOrtho(0,w,top?h:0,top?0:h,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
}
static void reshape(int w,int h){width=std::max(1,w);height=std::max(1,h);mission2::clientW=width;mission2::clientH=height;mission2::viewport.resize(width,height);}
static void keyDown(unsigned char key,int,int){
 int k=std::toupper(key);if(blocked[k])return;if(!keys[k])edges[k]=true;keys[k]=true;
 if(k==27){if(active){try{launch(0,false);}catch(const std::exception&e){fatal(e.what());}}else{menu::keyPressed[27]=true;}}
}
static void keyUp(unsigned char key,int,int){int k=std::toupper(key);keys[k]=blocked[k]=false;if(k==27)menu::keyPressed[27]=false;}
static void special(int key,int,int){if(key==GLUT_KEY_F11){fullscreen=!fullscreen;clearInput();if(fullscreen)glutFullScreen();else{glutReshapeWindow(1280,720);glutPositionWindow(60,60);}}}
// Keep M2's existing unlock artwork on screen while M3 loads incrementally.
static void prepareMission3Unlock(){
 if(unlockPreloading)return;
 assets3.beginLoad("");unlockPreloading=true;unlockEnterPending=false;
 // A key held/queued before the unlock screen must be released and pressed again.
 bool enterHeld=keys[13];
#ifdef _WIN32
 enterHeld=enterHeld||(GetAsyncKeyState(VK_RETURN)&0x8000)!=0;
#endif
 edges[13]=false;blocked[13]=blocked[13]||enterHeld;keys[13]=false;
 framePresented=false;
}
static void enterMission3FromUnlock(){
 // Preserve the completed preload instead of launch(), which releases all assets.
 mission2::release();clearInput();game3=m3::Mission();session.apply(game3);
 active=3;unlockPreloading=unlockEnterPending=false;framePresented=false;
 m3::Input confirm;confirm.confirm=true;game3.update(1.f/60,confirm);
 lastTime=glutGet(GLUT_ELAPSED_TIME);
}
static void pointer(int x,int y,bool down,bool up){
 if(active==2){
  if(unlockEnterPending)return;
  mission2::pointer(x,y,down,up);
  if(mission2::game.state==m2::M2_MISSION3_UNLOCK)prepareMission3Unlock();
  if(app::requestMission3(mission2::game)){
   prepareMission3Unlock();unlockEnterPending=true;
   // Do not expose the standalone handoff-end presentation while finishing loads.
   mission2::game.set(m2::M2_MISSION3_UNLOCK);
  }
  return;
 }
 float w=active==1?Meridian::WIDTH:1280,h=active==1?Meridian::HEIGHT:720;
 float scale=std::min(width/w,height/h),lx=(x-(width-w*scale)*.5f)/scale,ly=(y-(height-h*scale)*.5f)/scale;
 if(active==0){menu::iPassiveMouseMove(int(lx),int(h-ly));if(down)menu::iMouse(GLUT_LEFT_BUTTON,GLUT_DOWN,int(lx),int(h-ly));}
 if(active==1){input1.mx=lx;input1.my=ly;if(down)input1.click=true;}
 if(active==3){
 if(assets3.loading()){game3.buttonHover=game3.buttonRect().contains(lx,ly);if(down)game3.buttonDown=game3.buttonHover;if(up){if(game3.buttonDown&&game3.buttonHover)startRequested=true;game3.buttonDown=false;}}
 else game3.pointer(lx,ly,down,up);
 }
}
static void mouse(int button,int state,int x,int y){if(button==GLUT_LEFT_BUTTON)pointer(x,y,state==GLUT_DOWN,state==GLUT_UP);}
static void motion(int x,int y){pointer(x,y,false,false);}
static void visibility(int state){visible=state==GLUT_VISIBLE;clearInput();lastTime=glutGet(GLUT_ELAPSED_TIME);}
static bool focused(){
#ifdef _WIN32
 DWORD process=0;GetWindowThreadProcessId(GetForegroundWindow(),&process);return visible&&process==GetCurrentProcessId();
#else
 return visible;
#endif
}
// Called by the timer; deterministic fixed steps for M2/M3, original capped M1 step.
static void advance(double elapsed){
 if(!focused()){clearInput();return;}
#ifdef _WIN32
 // GLUT 3.7 key-up delivery varies on Windows; poll release and held controls too.
 const char* controls="WASDE JKL R\r";
 for(const char* p=controls;*p;++p){int k=*p;bool down=(GetAsyncKeyState(k==' '?VK_SPACE:k)&0x8000)!=0;
  if(!down){keys[k]=blocked[k]=false;}else if(!blocked[k]){if(!keys[k])edges[k]=true;keys[k]=true;}}
#endif
 if(active==0){menu::fixedUpdate();if(menu::requestedMission){int m=menu::requestedMission;launch(m,true);}return;}
 if(active==1){
 input1.w=keys['W'];input1.a=keys['A'];input1.s=keys['S'];input1.d=keys['D'];input1.attack=keys[' '];input1.enter=edges[13];input1.hide=edges['E'];
 // Inspect the existing CLAIMED button before updating, so the claim click cannot leak.
 bool next=app::requestMission2(game1,input1);Meridian::Scene previous=game1.scene;
 game1.update(float(std::min(.05,elapsed)),input1);session.observe(game1);
#ifdef _WIN32
 if(menu::gAudioEnabled&&previous!=Meridian::COUNTDOWN&&game1.scene==Meridian::COUNTDOWN)PlaySoundA("assets/m1/audio/countdown.wav",0,SND_FILENAME|SND_ASYNC|SND_NODEFAULT);
 if(previous==Meridian::COUNTDOWN&&game1.scene!=Meridian::COUNTDOWN)PlaySoundA(0,0,0);
#endif
 input1.click=input1.enter=input1.hide=false;std::fill(edges,edges+256,false);
 if(next)launch(2,false);return;
 }
 if(active==2){
 session.observe(mission2::game);
 if(mission2::game.state==m2::M2_MISSION3_UNLOCK||app::requestMission3(mission2::game)){
  prepareMission3Unlock();
  if(app::requestMission3(mission2::game)||edges[13])unlockEnterPending=true;
  mission2::game.state=m2::M2_MISSION3_UNLOCK;
  std::fill(edges,edges+256,false);accumulator=0;
  if(assets3.loading()&&framePresented){
   framePresented=false;assets3.loadNext();
   if(!assets3.errors.empty())throw std::runtime_error(assets3.errors.back());
  }
  if(unlockEnterPending&&!assets3.loading())enterMission3FromUnlock();
  lastTime=glutGet(GLUT_ELAPSED_TIME);return;
 }
 mission2::input.left=keys['A'];mission2::input.right=keys['D'];mission2::input.fire=keys[' '];mission2::input.reload=keys['R'];
 accumulator+=elapsed;while(accumulator>=1.0/120){mission2::game.tick(1.f/120,mission2::input);accumulator-=1.0/120;}
 std::fill(edges,edges+256,false);return;
 }
 if(assets3.loading()){
 if(edges[m3::Keys::Confirm])startRequested=true;std::fill(edges,edges+256,false);
 if(framePresented){framePresented=false;assets3.loadNext();if(!assets3.errors.empty())throw std::runtime_error(assets3.errors.back());}
 accumulator=0;lastTime=glutGet(GLUT_ELAPSED_TIME);return;
 }
 accumulator+=elapsed;while(accumulator>=1.0/60){
 m3::Input in;in.left=keys[m3::Keys::Left];in.right=keys[m3::Keys::Right];in.jump=edges[m3::Keys::Jump];in.kick=keys[m3::Keys::Kick];in.weapon=keys[m3::Keys::Weapon];in.fire=keys[m3::Keys::Fire];in.confirm=edges[m3::Keys::Confirm]||startRequested;in.walkHeld=keys[m3::Keys::Jump];
#ifdef _WIN32
 in.guard=(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0;
#else
 in.guard=keys['G'];
#endif
 game3.update(1.f/60,in);session.observe(game3);startRequested=false;std::fill(edges,edges+256,false);accumulator-=1.0/60;
 }
}
} // namespace host

#endif // SS_MERIDIAN_GAMEHOST_H
