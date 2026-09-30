// Host-only loader scheduling/PNG decode test; GL uploads are instrumented stubs.
// This does not claim to validate the target GPU driver.
#include "Mission3Assets.h"
#include <cassert>
#include <cstdio>
static unsigned int nextId=1;static int uploads=0,deletes=0;
extern "C" {
void glGetIntegerv(GLenum,GLint* v){*v=16384;}
const GLubyte* glGetString(GLenum){return reinterpret_cast<const GLubyte*>("GL_ARB_texture_non_power_of_two");}
GLenum glGetError(void){return GL_NO_ERROR;}
void glGenTextures(GLsizei n,GLuint* ids){for(int i=0;i<n;++i)ids[i]=nextId++;}
void glBindTexture(GLenum,GLuint){}
void glTexParameteri(GLenum,GLenum,GLint){}
void glTexImage2D(GLenum,GLint,GLint,GLsizei w,GLsizei h,GLint,GLenum,GLenum,const GLvoid* p){assert(w>0&&h>0&&p);++uploads;}
void glDeleteTextures(GLsizei n,const GLuint*){deletes+=n;}
}
int main(){
 m3::Assets a;a.beginLoad("");assert(uploads==0&&a.loading());
 a.loadNext();assert(uploads==1&&a.get(m3::A_FightBG).id!=0&&a.get(m3::A_Ship).id==0);
 while(a.loading()){int before=uploads;a.loadNext();assert(uploads-before<=1);}
 assert(a.errors.empty()&&uploads==m3::SequenceCount*6+m3::ArtCount);
 assert(a.loadedPercent()==100);a.release();assert(deletes==uploads);
 std::puts("PASS: zero startup uploads, entry background first, <=1 upload per step, all PNGs decoded, all textures released");
 return 0;
}
