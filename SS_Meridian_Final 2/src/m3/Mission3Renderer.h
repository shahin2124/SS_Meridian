#pragma once
#include "Mission3Assets.h"
#include "Mission3.h"
namespace m3 {
class Renderer {
 const Assets& assets;
 // Reference-backed renderer is deliberately non-copyable (VS2013 C4512).
 Renderer(const Renderer&);
 Renderer& operator=(const Renderer&);
 void quad(const Texture& t,float x,float y,float w,float h,float alpha=1) const;
 void fit(Art a,float x,float y,float w,float h) const;
 void background(Art a) const;
 void rect(float x,float y,float w,float h,float r,float g,float b,float alpha=1) const;
 void text(const char* s,float x,float y,float size,float r=1,float g=.94f,float b=.78f) const;
 void center(const char* s,float x,float y,float size) const;
 void wrap(const char* s,float x,float y,float width,float size) const;
 void fighter(const Mission& game,const Fighter& f,int who,float x,float ground,float zoom=1) const;
 void cages(const Mission& game,float shift,bool release) const;
 void hud(const Mission& game) const;
 void button(const Mission& game) const;
 void ending(const Mission& game) const;
public:
 explicit Renderer(const Assets& a):assets(a){}
 void draw(const Mission& game,int width,int height) const;
};
}
