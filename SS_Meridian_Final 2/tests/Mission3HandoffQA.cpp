// Targeted macOS offline-GL test of the actual production input/preload handoff.
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

static std::vector<unsigned char> pixels(){
 iDraw();glFinish();assert(glGetError()==GL_NO_ERROR);
 std::vector<unsigned char> p(1280*720*3);glPixelStorei(GL_PACK_ALIGNMENT,1);
 glReadPixels(0,0,1280,720,GL_RGB,GL_UNSIGNED_BYTE,&p[0]);return p;
}
static void unlock(){
 // Use the real victory, reward, and claim actions; no production shortcuts.
 mission2::game.set(m2::M2_DRAGON_FIGHT);
 for(int h=0;h<3;++h)for(int i=0;i<10;++i)assert(mission2::game.hit(h,m2::head(h)));
 step(80);assert(mission2::game.state==m2::M2_MIND_STONE_REWARD);
 m2::Rect b=mission2::view.buttonBox(mission2::game.state);
 clickLogical(b.x+b.w/2,720-(b.y+b.h/2));
 assert(host::active==2&&mission2::game.state==m2::M2_MISSION3_UNLOCK);
 assert(host::session.hasMindStone&&host::unlockPreloading&&!host::unlockEnterPending);
}
int main(){
 CGLPixelFormatAttribute attrs[]={kCGLPFAAllowOfflineRenderers,(CGLPixelFormatAttribute)0};CGLPixelFormatObj format=0;GLint count=0;
 assert(CGLChoosePixelFormat(attrs,&format,&count)==kCGLNoError&&format);CGLContextObj ctx=0;assert(CGLCreateContext(format,0,&ctx)==kCGLNoError);CGLDestroyPixelFormat(format);CGLSetCurrentContext(ctx);
 GLuint fbo,color;glGenFramebuffersEXT(1,&fbo);glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fbo);glGenRenderbuffersEXT(1,&color);glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,color);glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_RGBA8,1280,720);glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_RENDERBUFFER_EXT,color);assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)==GL_FRAMEBUFFER_COMPLETE_EXT);
 // Earlier ENTER is still physically held at the reward/claim transition.
 host::launch(2,true);iKeyboard(13,0,0);unlock();
 std::vector<unsigned char> original=pixels();
 int iterations=0;
 while(host::assets3.loading()){
  iKeyboard(13,0,0); // OS repeat must not queue a start.
  assert(pixels()==original);step(1);assert(host::active==2&&!host::unlockEnterPending);++iterations;
 }
 assert(iterations>1&&host::assets3.errors.empty());
 step(60);assert(host::active==2); // Ready alone must never start the mission.
 std::puts("PASS: earlier held ENTER rejected; all preload frames pixel-identical; ready waits for fresh input.");
 iKeyboardUp(13,0,0);iKeyboard(13,0,0);step(1);
 assert(host::active==3&&host::game3.state==m3::Intro&&!host::assets3.loading());
 assert(!host::keys[13]&&!host::edges[13]);
 for(int i=0;i<60;++i){iKeyboard(13,0,0);step(1);}assert(host::game3.state==m3::Walk);iKeyboardUp(13,0,0);
 std::puts("PASS: one fresh ENTER with assets ready starts M3 immediately; repeats do not leak.");
 // Fresh ENTER before any upload must remain latched even after release/focus loss.
 host::launch(2,true);unlock();assert(host::assets3.loading());iKeyboard(13,0,0);step(1);iKeyboardUp(13,0,0);
 assert(host::active==2&&host::unlockEnterPending);
 host::visibility(GLUT_NOT_VISIBLE);step(1);assert(host::unlockEnterPending);host::visibility(GLUT_VISIBLE);
 original=pixels();iterations=0;
 while(host::active==2){assert(pixels()==original);step(1);assert(++iterations<400);}
 assert(host::active==3&&host::game3.state==m3::Intro&&host::assets3.errors.empty()&&!host::assets3.loading());
 assert(host::game3.progress.hasSpaceStone&&host::game3.progress.hasMindStone&&!host::game3.progress.hasPowerStone);
 step(40);assert(host::game3.state==m3::Walk);
 std::puts("PASS: one early ENTER accepted, survives release/focus change, finishes loading invisibly and enters M3 without another action.");
 host::release();glDeleteRenderbuffersEXT(1,&color);glDeleteFramebuffersEXT(1,&fbo);CGLSetCurrentContext(0);CGLDestroyContext(ctx);
}
