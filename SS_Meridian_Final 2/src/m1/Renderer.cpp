#include "Renderer.h"
#include "Assets.h"
#include "PlatformGL.h"
#define STBI_ONLY_PNG
#include "../../vendor/stb_image.h"
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>
namespace {
// Decorative rear-wall prop only; uses the existing scene coordinate system.
void ladderPatch(float x,float y,float w,float h,float r,float g,float b){
 glColor4f(r,g,b,1);glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();
}
void drawLadder(const Meridian::Draw& d){
 glPushMatrix();glTranslatef(d.x,d.y,0);glScalef(d.w,d.h,1);
 // Wall stand-offs, plates and dark bolt heads attach both rails to the structure.
 for(int side=0;side<2;++side)for(int mount=0;mount<3;++mount){
  float x=side?.87f:-.06f,y=.055f+mount*.43f;
  ladderPatch(x,y,.19f,.035f,.115f,.113f,.086f);
  ladderPatch(x+.025f,y+.01f,.045f,.014f,.045f,.047f,.037f);
 }
 // Each rail tapers slightly toward the more distant upper end.
 for(int side=0;side<2;++side){
  float bottom=side?.91f:0,top=side?.88f:.03f;
  glColor4f(.035f,.038f,.029f,1);glBegin(GL_QUADS);
  glVertex2f(top+.025f,.012f);glVertex2f(top+.125f,.012f);glVertex2f(bottom+.125f,1.012f);glVertex2f(bottom+.025f,1.012f);glEnd();
  glColor4f(.20f,.205f,.16f,1);glBegin(GL_QUADS);
  glVertex2f(top,0);glVertex2f(top+.085f,0);glVertex2f(bottom+.09f,1);glVertex2f(bottom,1);glEnd();
  for(int chip=0;chip<15;++chip){
   float y=(chip+.3f)/15,x=top+(bottom-top)*y;
   ladderPatch(x+.013f,y,.02f,.035f,.29f,.285f,.22f);
   if(chip%3!=1)ladderPatch(x+.035f,y+.013f,.043f,.017f,.22f,.135f,.073f);
  }
 }
 for(int rung=0;rung<10;++rung){
  float y=.075f+rung*.094f,left=.03f*(1-y),right=.97f+.03f*y;
  ladderPatch(left+.025f,y+.012f,right-left,.029f,.04f,.043f,.032f);
  ladderPatch(left,y,right-left,.022f,.22f,.225f,.175f);
  ladderPatch(left+.035f,y,right-left-.07f,.006f,.34f,.33f,.255f);
  ladderPatch(left+.12f+(rung%3)*.17f,y+.007f,.18f,.012f,.25f,.15f,.08f);
 }
 glPopMatrix();
}

float cross(Meridian::Point a,Meridian::Point b,Meridian::Point c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
std::vector<Meridian::Point> triangles(std::vector<Meridian::Point> p){std::vector<Meridian::Point> result;float area=0;for(size_t i=0;i<p.size();++i)area+=p[i].x*p[(i+1)%p.size()].y-p[(i+1)%p.size()].x*p[i].y;if(area<0)std::reverse(p.begin(),p.end());
 while(p.size()>2){bool cut=false;for(size_t i=0;i<p.size();++i){size_t a=(i+p.size()-1)%p.size(),b=(i+1)%p.size();if(cross(p[a],p[i],p[b])<=0)continue;bool inside=false;for(size_t j=0;j<p.size();++j)if(j!=a&&j!=i&&j!=b&&cross(p[a],p[i],p[j])>=0&&cross(p[i],p[b],p[j])>=0&&cross(p[b],p[a],p[j])>=0)inside=true;if(inside)continue;result.push_back(p[a]);result.push_back(p[i]);result.push_back(p[b]);p.erase(p.begin()+i);cut=true;break;}if(!cut)break;}return result;}
}
Renderer::Texture& Renderer::load(const std::string& key){
 Texture& t=textures[key];t.last=tick;if(t.id)return t;
 const AssetInfo* a=0;for(int i=0;i<ASSET_COUNT;++i)if(key==ASSETS[i].key){a=&ASSETS[i];break;}
 if(!a)throw std::runtime_error("Unmapped asset: "+key);
 int w,h,n;unsigned char* pixels=stbi_load(a->path,&w,&h,&n,4);
 if(!pixels)throw std::runtime_error(std::string("Cannot load ")+a->path+": "+stbi_failure_reason());
 // POT padding supports legacy OpenGL without distorting or cropping the supplied canvas.
 int pw=1,ph=1;while(pw<w)pw*=2;while(ph<h)ph*=2;
 GLint maximum=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum);
 if(pw>maximum||ph>maximum){stbi_image_free(pixels);throw std::runtime_error("OpenGL texture limit too small for "+key+". Enable VM 3D acceleration.");}
 std::vector<unsigned char> padded(size_t(pw)*ph*4,0);
 for(int y=0;y<h;++y)std::memcpy(&padded[size_t(y)*pw*4],pixels+size_t(y)*w*4,size_t(w)*4);
 stbi_image_free(pixels);glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,pw,ph,0,GL_RGBA,GL_UNSIGNED_BYTE,&padded[0]);
 if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("OpenGL upload failed: "+key);
 t.u=float(w)/pw;t.v=float(h)/ph;t.bytes=size_t(pw)*ph*4;bytes+=t.bytes;return t;
}
Renderer::~Renderer(){for(std::map<std::string,Texture>::iterator it=textures.begin();it!=textures.end();++it)if(it->second.id)glDeleteTextures(1,&it->second.id);}
void Renderer::paint(const std::vector<Meridian::Draw>& commands){
 ++tick;glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_DEPTH_TEST);
 for(size_t i=0;i<commands.size();++i){const Meridian::Draw& d=commands[i];
  if(d.kind==Meridian::Draw::PATCH){Texture& t=load(d.value);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,t.id);glColor4f(1,1,1,1);std::vector<Meridian::Point> vertices=triangles(d.polygon);glBegin(GL_TRIANGLES);for(size_t j=0;j<vertices.size();++j){glTexCoord2f(vertices[j].x/Meridian::WIDTH*t.u,vertices[j].y/Meridian::HEIGHT*t.v);glVertex2f(vertices[j].x,vertices[j].y);}glEnd();glDisable(GL_TEXTURE_2D);rendered.insert(d.value);
  }else if(d.kind==Meridian::Draw::SHADOW){glColor4f(0,0,0,d.alpha);glBegin(GL_TRIANGLE_FAN);glVertex2f(d.x+d.w/2,d.y+d.h/2);for(int j=0;j<=24;++j){float a=j*6.2831853f/24;glVertex2f(d.x+d.w/2+std::cos(a)*d.w/2,d.y+d.h/2+std::sin(a)*d.h/2);}glEnd();
  }else if(d.kind==Meridian::Draw::IMAGE||d.kind==Meridian::Draw::REGION){Texture& t=load(d.value);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,t.id);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);glColor4f(1,1,1,d.alpha);
   float u0=(d.kind==Meridian::Draw::REGION?d.sx:0)*t.u,u1=(d.kind==Meridian::Draw::REGION?(d.sx+d.sw):1)*t.u;
   float v0=(d.kind==Meridian::Draw::REGION?d.sy:0)*t.v,v1=(d.kind==Meridian::Draw::REGION?(d.sy+d.sh):1)*t.v;
   if(d.flip)std::swap(u0,u1);
   glBegin(GL_QUADS);glTexCoord2f(u0,v0);glVertex2f(d.x,d.y);glTexCoord2f(u1,v0);glVertex2f(d.x+d.w,d.y);glTexCoord2f(u1,v1);glVertex2f(d.x+d.w,d.y+d.h);glTexCoord2f(u0,v1);glVertex2f(d.x,d.y+d.h);glEnd();glDisable(GL_TEXTURE_2D);rendered.insert(d.value);
  }else if(d.kind==Meridian::Draw::BOX&&d.value=="r3_ladder"){drawLadder(d);
  }else if(d.kind==Meridian::Draw::BOX){if(d.value=="rail")glColor4f(.65f,.7f,.75f,d.alpha);else glColor4f(.01f,.015f,.025f,d.alpha);glBegin(GL_QUADS);glVertex2f(d.x,d.y);glVertex2f(d.x+d.w,d.y);glVertex2f(d.x+d.w,d.y+d.h);glVertex2f(d.x,d.y+d.h);glEnd();
  }else {std::string s=d.value;bool dark=!s.empty()&&s[0]=='~';if(dark)s.erase(0,1);float scale=d.w/119.05f;float width=0;for(size_t j=0;j<s.size();++j)width+=glutStrokeWidth(GLUT_STROKE_ROMAN,s[j])*scale;
   float x=d.x<0?(Meridian::WIDTH-width)/2:d.x;glColor4f(dark?.04f:.94f,dark?.04f:.95f,dark?.05f:1,1);glLineWidth(d.w>=45?2.3f:1.4f);glPushMatrix();glTranslatef(x,d.y,0);glScalef(scale,-scale,1);for(size_t j=0;j<s.size();++j)glutStrokeCharacter(GLUT_STROKE_ROMAN,s[j]);glPopMatrix();
  }
 }
 // Bound GPU residency without discarding the currently drawn frames.
 if(bytes>160*1024*1024){for(std::map<std::string,Texture>::iterator it=textures.begin();it!=textures.end();++it){Texture& t=it->second;if(t.id&&tick-t.last>120){glDeleteTextures(1,&t.id);bytes-=t.bytes;t.id=0;t.bytes=0;}}}
}
void Renderer::report()const{std::ofstream f("runtime_asset_coverage.tsv");f<<"asset\trendered\n";for(int i=0;i<ASSET_COUNT;++i)f<<ASSETS[i].path<<"\t"<<(rendered.count(ASSETS[i].key)?"yes":"not seen this session")<<"\n";}
