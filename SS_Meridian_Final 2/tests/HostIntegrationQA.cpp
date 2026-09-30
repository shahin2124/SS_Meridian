// macOS offscreen QA only. Exercises the production host callbacks and renderer.
// Not part of either Windows configuration; no shortcuts enter the game binary.
#include "../src/m3/Platform.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/glext.h>
static int qaGet(GLenum){return 0;}
static void qaSwap(){}
#define glutGet qaGet
#define glutSwapBuffers qaSwap
#define main productionEntry
#include "../src/iMain.cpp"
#undef main
#undef glutGet
#undef glutSwapBuffers
#include <cassert>
static void shot(const char* name){
 iDraw();glFinish();assert(glGetError()==GL_NO_ERROR);
 std::vector<unsigned char> pixels(1280*720*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,1280,720,GL_RGB,GL_UNSIGNED_BYTE,&pixels[0]);
 std::ofstream f((std::string("qa_rendered/")+name+".ppm").c_str(),std::ios::binary);f<<"P6\n1280 720\n255\n";for(int y=719;y>=0;--y)f.write((char*)&pixels[y*1280*3],1280*3);
}
static void step(int frames){for(int i=0;i<frames;++i)host::advance(1.0/60);}
static void tapEnter(){iKeyboard(13,0,0);step(1);iKeyboardUp(13,0,0);}
static void clickLogical(float x,float y){iMouse(GLUT_LEFT_BUTTON,GLUT_DOWN,int(x),int(y));iMouse(GLUT_LEFT_BUTTON,GLUT_UP,int(x),int(y));step(1);}
static void load3(){while(host::assets3.loading()){iDraw();host::advance(1.0/60);}assert(host::assets3.errors.empty());}
int main(int argc,char**){
 CGLPixelFormatAttribute attrs[]={kCGLPFAAllowOfflineRenderers,(CGLPixelFormatAttribute)0};CGLPixelFormatObj format=0;GLint count=0;
 assert(CGLChoosePixelFormat(attrs,&format,&count)==kCGLNoError&&format);CGLContextObj ctx=0;assert(CGLCreateContext(format,0,&ctx)==kCGLNoError);CGLDestroyPixelFormat(format);CGLSetCurrentContext(ctx);
 GLuint fbo,color;glGenFramebuffersEXT(1,&fbo);glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fbo);glGenRenderbuffersEXT(1,&color);glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,color);glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_RGBA8,1280,720);glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_RENDERBUFFER_EXT,color);assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)==GL_FRAMEBUFFER_COMPLETE_EXT);
 host::launch(0,true);shot("01_menu");
 if(argc>1){clickLogical(950,350);assert(menu::gState==menu::STATE_MISSION_SELECT);shot("13_mission_select");host::release();glDeleteRenderbuffersEXT(1,&color);glDeleteFramebuffersEXT(1,&fbo);CGLSetCurrentContext(0);CGLDestroyContext(ctx);std::puts("PASS: final menu/card presentation and launch hitboxes rendered");return 0;}
 clickLogical(950,290);assert(host::active==1&&host::game1.scene==Meridian::ARRIVAL);assert(!host::session.hasSpaceStone);shot("02_m1_arrival");
 // Complete the real claim state before activating the original M1 ENTER hitbox.
 host::game1.start(Meridian::CLAIM_FLASH);step(40);assert(host::session.hasSpaceStone&&host::session.mission1Complete);shot("03_m1_claimed");
 clickLogical(30,30);assert(host::active==1);
 iKeyboard('w',0,0);clickLogical(836*1280.f/1672,770*720.f/941);assert(host::active==2&&mission2::game.state==m2::M2_START);assert(!host::keys['W']);assert(!mission2::pressed);iKeyboardUp('w',0,0);shot("04_m2_entry");
 m2::Rect b=mission2::view.buttonBox(mission2::game.state);clickLogical(b.x+b.w/2,720-(b.y+b.h/2));step(80);assert(mission2::game.state==m2::M2_FIGHT_GUIDE);shot("05_m2_guide");
 b=mission2::view.buttonBox(mission2::game.state);clickLogical(b.x+b.w/2,720-(b.y+b.h/2));assert(mission2::game.state==m2::M2_DRAGON_FIGHT);shot("06_m2_fight");
 // Real collision API: thirty valid head hits, then real death/reward/claim flow.
 for(int h=0;h<3;++h)for(int n=0;n<10;++n)assert(mission2::game.hit(h,m2::head(h)));step(80);assert(mission2::game.state==m2::M2_MIND_STONE_REWARD);
 b=mission2::view.buttonBox(mission2::game.state);clickLogical(b.x+b.w/2,720-(b.y+b.h/2));assert(host::session.hasMindStone&&mission2::game.state==m2::M2_MISSION3_UNLOCK);shot("07_m2_unlock");
 b=mission2::view.buttonBox(mission2::game.state);clickLogical(b.x+b.w/2,720-(b.y+b.h/2));assert(host::active==2&&host::unlockEnterPending);load3();assert(host::active==3&&host::game3.state==m3::Intro);assert(host::game3.progress.hasSpaceStone&&host::game3.progress.hasMindStone&&!host::game3.progress.hasPowerStone);assert(!host::game3.buttonDown);shot("08_m3_started");
 step(40);assert(host::game3.state==m3::Walk);shot("09_m3_walk");
 // Ending model is regression-tested in Mission3Tests; test actual host key edges here.
 host::game3.state=m3::Reward;host::game3.progress.defeated[0]=host::game3.progress.defeated[1]=host::game3.progress.defeated[2]=true;
 m3::Rect r=host::game3.buttonRect();clickLogical(r.x+r.w/2,r.y+r.h/2);assert(host::game3.progress.hasPowerStone);assert(host::session.hasPowerStone);step(240);assert(host::game3.state==m3::FinalDialogue);shot("10_m3_ending");
 iKeyboard(13,0,0);step(1);assert(host::game3.finalDialogue==1);
 for(int i=0;i<120;++i){iKeyboard(13,0,0);step(1);}assert(host::game3.finalDialogue==1);
 iKeyboardUp(13,0,0);tapEnter();assert(host::game3.finalDialogue==2);tapEnter();tapEnter();assert(host::game3.state==m3::FinalSpace);
 const m3::State ordered[]={m3::FinalMind,m3::FinalPower,m3::FinalGlow,m3::FinalBlackFade,m3::FinalShipCount,m3::FinalShipWalk};
 for(int i=0;i<6;++i){m3::State before=host::game3.state;for(int t=0;t<600&&host::game3.state==before;++t)step(1);assert(host::game3.state==ordered[i]);}
 iKeyboard('w',0,0);step(120);assert(host::game3.shipProgress==0);iKeyboardUp('w',0,0);step(1);
 iKeyboard('w',0,0);step(720);assert(host::game3.state==m3::FinalDoorWait);shot("11a_homecoming_door");
 tapEnter();assert(host::game3.state==m3::FinalBeachWalk);step(120);assert(host::game3.beachProgress==0);
 iKeyboardUp('w',0,0);step(1);iKeyboard('w',0,0);step(60);assert(host::game3.beachProgress>0);shot("11_beach_walk");
 step(660);assert(host::game3.state==m3::FinalReflection);iKeyboardUp('w',0,0);
 for(int i=0;i<7;++i)tapEnter();assert(host::game3.state==m3::FinalBeachWatch);
 step(240);assert(host::game3.state==m3::FinalJourney);tapEnter();step(60);assert(host::game3.state==m3::FinalJourney);shot("12_final_journey");

 for(int mission=1;mission<=3;++mission){
 host::launch(0,true);clickLogical(950,350);assert(menu::gState==menu::STATE_MISSION_SELECT);if(mission==1)shot("13_mission_select");
 menu::RectF card=menu::missionCard(mission-1);clickLogical(card.x+card.w/2,720-(card.y+card.h/2));assert(host::active==mission);
 assert(host::session.hasSpaceStone==(mission>=2));assert(host::session.hasMindStone==(mission>=3));assert(!host::session.hasPowerStone);
 if(mission==1)assert(host::game1.scene==Meridian::ARRIVAL);if(mission==2)assert(mission2::game.state==m2::M2_START);if(mission==3){assert(host::game3.state==m3::Enter);load3();}
 iKeyboard(27,0,0);assert(host::active==0);iKeyboardUp(27,0,0);
 }
 host::release();glDeleteRenderbuffersEXT(1,&color);glDeleteFramebuffersEXT(1,&fbo);CGLSetCurrentContext(0);CGLDestroyContext(ctx);
 std::puts("PASS: actual menu routes, all direct launches, both original handoffs, progress, input isolation, M3 loading/claim/ending controls, offscreen OpenGL rendering and resource release in one GL context.");
}
