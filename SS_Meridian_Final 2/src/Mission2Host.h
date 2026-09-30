#ifndef SS_MERIDIAN_MISSION2HOST_H
#define SS_MERIDIAN_MISSION2HOST_H

// Implementation header: include only through iMain.cpp (one host translation unit).
// Mission 2 original presentation renderer adapted to the shared GL context.
namespace mission2 {
using namespace m2;
static Game game;static Presentation view;static Viewport viewport;static Input input;
static GLuint textures[ASSET_COUNT]={0},fontTexture=0;
static int fontWidths[96]={0},fontAscent=36;
static GlyphInk fontInk[96]={{0,0,0,0}};
static int clientW=1280,clientH=720;static bool ready=false,pressed=false;
static State pressedState=M2_START;
static void loadAssets(){
 for(int i=0;i<ASSET_COUNT;++i){
  int w,h,c;const char* path=assetInfo[i].path;unsigned char* pixels=stbi_load(path,&w,&h,&c,4);
  if(!pixels)throw std::runtime_error(std::string("Cannot read required asset: ")+path);
  int tw=1,th=1;while(tw<w)tw*=2;while(th<h)th*=2;
  GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
  if(tw>limit||th>limit){stbi_image_free(pixels);throw std::runtime_error("Mission 2 texture exceeds GPU limit");}
  std::vector<unsigned char> padded(tw*th*4,0);
  for(int y=0;y<h;++y)std::copy(pixels+y*w*4,pixels+(y+1)*w*4,padded.begin()+y*tw*4);
  stbi_image_free(pixels);glGenTextures(1,&textures[i]);glBindTexture(GL_TEXTURE_2D,textures[i]);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,&padded[0]);
  if(glGetError()!=GL_NO_ERROR)throw std::runtime_error(std::string("Texture upload failed: ")+path);
 }
#ifdef _WIN32
 // Render code text once to a GDI glyph atlas; textured glyphs scale with the viewport.
 HDC mem=CreateCompatibleDC(0);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=1024;bi.bmiHeader.biHeight=-512;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
 void* bits=0;HBITMAP bitmap=CreateDIBSection(mem,&bi,DIB_RGB_COLORS,&bits,0,0);if(!mem||!bitmap||!bits)throw std::runtime_error("Cannot allocate UI font atlas.");HGDIOBJ oldBitmap=SelectObject(mem,bitmap);std::memset(bits,0,1024*512*4);
 HFONT font=CreateFontA(-36,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_SWISS,"Arial");HGDIOBJ oldFont=SelectObject(mem,font);SetTextColor(mem,RGB(255,255,255));SetBkColor(mem,RGB(0,0,0));SetBkMode(mem,OPAQUE);TEXTMETRICA metrics;GetTextMetricsA(mem,&metrics);fontAscent=metrics.tmAscent;
 for(int i=0;i<96;++i){char c=char(i+32);SIZE sz;GetTextExtentPoint32A(mem,&c,1,&sz);fontWidths[i]=sz.cx;TextOutA(mem,(i%16)*64,(i/16)*64,&c,1);}GdiFlush();
 std::vector<unsigned char> glyphs(1024*512*4);unsigned char* bgra=static_cast<unsigned char*>(bits);for(size_t i=0;i<glyphs.size();i+=4){glyphs[i]=glyphs[i+1]=glyphs[i+2]=255;glyphs[i+3]=std::max(bgra[i],std::max(bgra[i+1],bgra[i+2]));}
 // Measure actual visible glyph extents, including side bearings and baseline offset.
 for(int n=0;n<96;++n){GlyphInk& box=fontInk[n];box.left=box.top=64;box.right=box.bottom=0;
  for(int y=0;y<64;++y)for(int x=0;x<64;++x){size_t offset=((n/16*64+y)*1024+n%16*64+x)*4;
   if(glyphs[offset+3]){box.left=std::min(box.left,x);box.top=std::min(box.top,y);box.right=std::max(box.right,x+1);box.bottom=std::max(box.bottom,y+1);}
  }
 }
 glGenTextures(1,&fontTexture);glBindTexture(GL_TEXTURE_2D,fontTexture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,512,0,GL_RGBA,GL_UNSIGNED_BYTE,&glyphs[0]);
 SelectObject(mem,oldFont);SelectObject(mem,oldBitmap);DeleteObject(font);DeleteObject(bitmap);DeleteDC(mem);

#endif
}
static void draw(){
 if(!ready)return;view.draw(game);glViewport(0,0,clientW,clientH);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);glViewport(viewport.x,viewport.y,viewport.w,viewport.h);
 glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,1280,0,720,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 for(size_t i=0;i<view.list.size();++i){const Draw& d=view.list[i];glColor4f(d.r,d.g,d.b,d.alpha);

#ifdef _WIN32
 if(d.asset==-2){glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,fontTexture);Point origin(d.box.x,d.box.y);if(d.centerInk)origin=centeredInkOrigin(d.text,origin,d.size,fontWidths,fontInk,fontAscent);float scale=d.size/36.f,x=origin.x,top=origin.y+fontAscent*scale;for(size_t c=0;c<d.text.size();++c){int n=static_cast<unsigned char>(d.text[c])-32;if(n<0||n>=96)continue;float u=(n%16)*64/1024.f,v=(n/16)*64/512.f,w=64*scale;glBegin(GL_QUADS);glTexCoord2f(u,v);glVertex2f(x,top);glTexCoord2f(u+64/1024.f,v);glVertex2f(x+w,top);glTexCoord2f(u+64/1024.f,v+64/512.f);glVertex2f(x+w,top-w);glTexCoord2f(u,v+64/512.f);glVertex2f(x,top-w);glEnd();x+=fontWidths[n]*scale;}continue;}
#else
 if(d.asset==-2){glDisable(GL_TEXTURE_2D);glPushMatrix();float x=d.box.x;if(d.centerInk)x-=d.text.size()*d.size*.28f;glTranslatef(x,d.box.y,0);glScalef(d.size/119.f,d.size/119.f,1);for(size_t n=0;n<d.text.size();++n)glutStrokeCharacter(GLUT_STROKE_ROMAN,d.text[n]);glPopMatrix();continue;}
#endif

 float u0=0,v0=0,u1=1,v1=1;
 if(d.asset>=0){const AssetInfo&a=assetInfo[d.asset];int tw=1,th=1;while(tw<a.w)tw*=2;while(th<a.h)th*=2;u1=float(a.w)/tw;v1=float(a.h)/th;if(d.crop){u0=float(a.l)/tw;v0=float(a.t)/th;u1=float(a.r)/tw;v1=float(a.b)/th;}glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,textures[d.asset]);}else glDisable(GL_TEXTURE_2D);
 glPushMatrix();glTranslatef(d.box.x+d.box.w/2,d.box.y+d.box.h/2,0);glRotatef(d.angle,0,0,1);float w=d.box.w/2,h=d.box.h/2;
 glBegin(GL_QUADS);glTexCoord2f(u0,v1);glVertex2f(-w,-h);glTexCoord2f(u1,v1);glVertex2f(w,-h);glTexCoord2f(u1,v0);glVertex2f(w,h);glTexCoord2f(u0,v0);glVertex2f(-w,h);glEnd();glPopMatrix();
 }
}

static void release(){ready=false;glDeleteTextures(ASSET_COUNT,textures);std::fill(textures,textures+ASSET_COUNT,0);if(fontTexture)glDeleteTextures(1,&fontTexture);fontTexture=0;pressed=view.down=false;input=Input();}
static void initialize(){game=Game();view=Presentation();input=Input();pressed=false;loadAssets();ready=true;}
static void pointer(int x,int y,bool down,bool up){
 view.mouse=viewport.mouse(x,y,clientH);
 if(down){pressed=viewport.containsPixel(x,y,clientH)&&view.hasButton(game.state)&&view.buttonBox(game.state).contains(view.mouse);pressedState=game.state;view.down=pressed;}
 if(up){bool click=pressed&&viewport.containsPixel(x,y,clientH)&&pressedState==game.state&&view.buttonBox(game.state).contains(view.mouse);pressed=view.down=false;if(click){game.activate();input=Input();}}
}
} // namespace mission2

#endif // SS_MERIDIAN_MISSION2HOST_H
