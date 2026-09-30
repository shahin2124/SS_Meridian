#include "Mission3Assets.h"
#define STBI_ONLY_PNG
#include "stb_image.h"
#include <cstdio>
#include <algorithm>
#include <cstring>
namespace m3 {
#include "AssetCatalog.inl"
static int powerOfTwo(int n){int p=1;while(p<n)p*=2;return p;}
bool Assets::loadTexture(Texture& t,const std::string& path){
 int w=0,h=0,channels=0;unsigned char* pixels=stbi_load(path.c_str(),&w,&h,&channels,4);
 if(!pixels){errors.push_back(path+" : "+(stbi_failure_reason()?stbi_failure_reason():"PNG decode failed"));return false;}
 t.left=w;t.top=h;t.right=0;t.bottom=0;
 for(int y=0;y<h;++y)for(int x=0;x<w;++x)if(pixels[(y*w+x)*4+3]>8){t.left=std::min(t.left,x);t.top=std::min(t.top,y);t.right=std::max(t.right,x+1);t.bottom=std::max(t.bottom,y+1);}
 if(t.right<=t.left||t.bottom<=t.top){t.left=t.top=0;t.right=w;t.bottom=h;}
 GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
 const char* extensions=reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
 bool npot=extensions&&std::strstr(extensions,"GL_ARB_texture_non_power_of_two")!=NULL;
 int tw=npot?w:powerOfTwo(w),th=npot?h:powerOfTwo(h);
 if(tw>limit||th>limit){stbi_image_free(pixels);errors.push_back(path+" : GPU maximum texture size too small (needs "+std::to_string(std::max(tw,th))+")");return false;}
 // Native sizes reduce VRAM when the optional NPOT extension exists.
 // Otherwise power-of-two backing supports legacy OpenGL 1.x.
 // Only one decoded CPU image exists at a time; original files stay untouched.
 std::vector<unsigned char> backing(static_cast<size_t>(tw)*th*4,0);
 for(int y=0;y<h;++y)std::copy(pixels+y*w*4,pixels+(y+1)*w*4,&backing[static_cast<size_t>(y)*tw*4]);
 stbi_image_free(pixels);
 while(glGetError()!=GL_NO_ERROR){}
 glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,&backing[0]);
 if(glGetError()!=GL_NO_ERROR){errors.push_back(path+" : OpenGL texture upload failed");glDeleteTextures(1,&t.id);t.id=0;return false;}
 t.width=w;t.height=h;t.u=static_cast<float>(w)/tw;t.v=static_cast<float>(h)/th;return true;
}
void Assets::beginLoad(const std::string& root){
 release();errors.clear();loadRoot=root;loadCursor=0;
}
void Assets::loadNext(){
 if(!loading())return;
 size_t before=errors.size();
 // The entry screen's only image comes first. One texture per visible frame.
 if(loadCursor==0)loadTexture(art[A_FightBG],loadRoot+ArtPaths[A_FightBG]);
 else if(loadCursor<=ArtCount){
  int i=loadCursor-1;if(i!=A_FightBG)loadTexture(art[i],loadRoot+ArtPaths[i]);
 }else{
  int n=loadCursor-1-ArtCount;char suffix[16];std::sprintf(suffix,"_%02d.png",n%6+1);
  loadTexture(animations[n/6][n%6],loadRoot+SequencePaths[n/6]+suffix);
 }
 ++loadCursor;
 for(size_t i=before;i<errors.size();++i)std::fprintf(stderr,"ASSET ERROR: %s\n",errors[i].c_str());
}
bool Assets::load(const std::string& root){
 // Synchronous entry retained for offscreen QA. The game uses beginLoad/loadNext.
 beginLoad(root);while(loading())loadNext();return errors.empty();
}
void Assets::release(){
 loadCursor=1+ArtCount+SequenceCount*6;
 for(int i=0;i<SequenceCount;++i)for(int f=0;f<6;++f)if(animations[i][f].id){glDeleteTextures(1,&animations[i][f].id);animations[i][f]=Texture();}
 for(int i=0;i<ArtCount;++i)if(art[i].id){glDeleteTextures(1,&art[i].id);art[i]=Texture();}
}
}
