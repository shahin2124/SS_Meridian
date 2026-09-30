#include "../../src/m2/Presentation.h"
#include <cassert>
#include <iostream>
using namespace m2;
static void step(Game&g,float seconds,const Input&in=Input()){int n=int(seconds*120+.5f);for(int i=0;i<n;++i)g.tick(1.f/120,in);}
static Game fight(){Game g;g.activate();step(g,1.3f);assert(g.state==M2_FIGHT_GUIDE&&g.time==180);g.activate();return g;}
int main(){
 Game g;assert(g.state==M2_START);step(g,10);assert(g.time==180);g=fight();assert(g.hp==ISSAC_MAX_HP&&g.ammo==3&&g.heads[0]==100);
 Input in;in.right=true;step(g,1.4f,in);assert(g.x==440&&g.target()==2);in.right=false;in.left=true;step(g,1.4f,in);assert(g.x==20&&g.target()==0);in.left=false;step(g,.02f,in);assert(g.move==0&&g.anim==0);
 g=fight();g.x=180;assert(g.target()==1);g.heads[1]=0;assert(g.target()==2);g.heads[2]=0;assert(g.target()==0);
 g=fight();assert(!g.hit(0,Point(900,250)));assert(g.heads[0]==100);assert(g.hit(0,head(0)));assert(g.heads[0]==90);for(int i=0;i<9;++i)g.hit(0,head(0));assert(!g.hit(0,head(0))&&g.heads[0]==0);
 g=fight();in=Input();in.fire=true;step(g,.02f,in);assert(g.ammo==2&&g.arrows.size()==1&&g.heads[0]==100);step(g,.2f,in);assert(g.ammo==2&&g.arrows[0].pos.x>g.arrows[0].from.x);step(g,.2f,in);assert(g.heads[0]==90);step(g,.5f,in);assert(g.ammo==0);int hits=g.hits;step(g,.5f,in);assert(g.hits<=3&&g.hits>=hits);
 in.reload=true;step(g,.5f,in);assert(g.ammo==0&&g.reload>0);in.fire=false;in.reload=false;step(g,.6f,in);assert(g.ammo==3&&g.reload==0);
 g=fight();step(g,.49f);assert(g.attacks.empty());step(g,.02f);assert(g.attacksStarted==1&&g.attacks[0].head==0);step(g,.65f);assert(g.attacksStarted==2&&g.attacks.size()==2&&g.attacks[1].head==1);
 step(g,.30f);assert(g.hp==ISSAC_MAX_HP-25);step(g,.15f);assert(g.hp==ISSAC_MAX_HP-25);g.heads[0]=g.heads[1]=0;step(g,.2f);assert(g.attacks.back().head==2);step(g,.65f);assert(g.attacks.back().head==2&&g.attacks.size()<=3);
 g=fight();step(g,.6f);in=Input();in.right=true;step(g,.9f,in);assert(g.hp==ISSAC_MAX_HP); // first warned attack dodged
 g=fight();step(g,30);assert(g.state==M2_LOST&&g.hp==0);g.activate();assert(g.state==M2_DRAGON_FIGHT&&g.hp==ISSAC_MAX_HP&&g.time==180&&g.ammo==3&&g.x==20&&g.nextAttack==.5f&&g.attacks.empty()&&g.arrows.empty()&&g.effects.empty()&&g.elapsed==0&&g.hits==0);
 g.time=.001f;step(g,.02f);assert(g.state==M2_LOST);g.activate();assert(g.heads[0]==100&&g.heads[1]==100&&g.heads[2]==100);
 for(int h=0;h<3;++h)for(int n=0;n<10;++n)assert(g.hit(h,head(h)));assert(g.hits==30);g.time=.001f;g.hp=25;g.attacks.push_back(Attack(0,g.x+250));g.attacks.back().age=.949f;step(g,.02f);assert(g.state==M2_DRAGON_DEFEATED&&g.hp==25&&g.attacks.empty());step(g,1.3f);assert(g.state==M2_MIND_STONE_REWARD&&!g.claimed);g.activate();assert(g.claimed&&g.state==M2_MISSION3_UNLOCK);g.activate();assert(g.state==M2_HANDOFF_END);g.activate();assert(g.state==M2_HANDOFF_END);
 // Final projectile contact, timeout, and hostile impact in the very same update.
 g=fight();g.heads[0]=g.heads[1]=0;g.heads[2]=10;g.time=.001f;g.arrows.push_back(Arrow(g.bow(),head(2),2));g.arrows.back().age=.379f;g.attacks.push_back(Attack(2,g.x+250));g.attacks.back().age=.949f;step(g,.01f);assert(g.state==M2_DRAGON_DEFEATED&&g.hp==ISSAC_MAX_HP);
 for(int i=0;i<6;++i){assert(frame(i*.1f+.001f,.6f,true)==i);assert(frame(i*.1f+.001f,.6f,false)==i);}assert(frame(.601f,.6f,true)==0&&frame(2,.6f,false)==5);
 Viewport v;v.resize(1920,1200);assert(v.w==1920&&v.h==1080&&v.y==60);Point p=v.mouse(960,600,1200);assert(p.x==640&&p.y==360);v.resize(1280,720);assert(v.mouse(100,620,720).y==100);
 Presentation ui;for(int s=0;s<=8;++s){g.state=State(s);ui.draw(g);for(size_t i=0;i<ui.list.size();++i)assert(ui.list[i].asset<ASSET_COUNT);}
 // Exercise all 15 button-state asset selections and fullscreen hit coordinates.
 State states[]={M2_START,M2_FIGHT_GUIDE,M2_LOST,M2_MIND_STONE_REWARD,M2_MISSION3_UNLOCK};
 int buttons[5][3]={{MISSION2_ENTER_BUTTON_NORMAL,MISSION2_ENTER_BUTTON_HOVER,MISSION2_ENTER_BUTTON_PRESSED},{START_FIGHT_BUTTON_NORMAL,START_FIGHT_BUTTON_HOVER,START_FIGHT_BUTTON_PRESSED},{RETRY_BUTTON_NORMAL,RETRY_BUTTON_HOVER,RETRY_BUTTON_PRESSED},{CLAIM_BUTTON_NORMAL,CLAIM_BUTTON_HOVER,CLAIM_BUTTON_PRESSED},{ENTER_BUTTON_NORMAL,ENTER_BUTTON_HOVER,ENTER_BUTTON_PRESSED}};
 for(int j=0;j<5;++j){g.state=states[j];Rect r=ui.buttonBox(g.state);for(int k=0;k<3;++k){ui.mouse=k?Point(r.x+r.w/2,r.y+r.h/2):Point(-1,-1);ui.down=k==2;ui.draw(g);bool found=false;for(size_t i=0;i<ui.list.size();++i)if(ui.list[i].asset==buttons[j][k])found=true;assert(found);}v.resize(1920,1200);Point center(r.x+r.w/2,r.y+r.h/2);Point mapped=v.mouse(int(v.x+center.x*v.w/1280),int(1200-v.y-center.y*v.h/720),1200);assert(r.contains(mapped));}
 // Hotfix: centralized 1000 HP, HUD maximum, exact normalization, 25 damage and retry.
 assert(ISSAC_MAX_HP==1000);g=fight();assert(g.hp==1000);ui.draw(g);
 bool hpText=false,fullBar=false;for(size_t i=0;i<ui.list.size();++i){const Draw&d=ui.list[i];if(d.text=="ISSAC  1000 / 1000")hpText=true;if(d.asset==-1&&d.box.x==110&&d.box.y==644&&d.box.w==180)fullBar=true;}assert(hpText&&fullBar);
 g.attacks.push_back(Attack(0,g.x+250));g.attacks.back().age=.949f;step(g,.01f);assert(g.hp==975);ui.draw(g);
 hpText=false;bool scaledBar=false;for(size_t i=0;i<ui.list.size();++i){const Draw&d=ui.list[i];if(d.text=="ISSAC  975 / 1000")hpText=true;if(d.asset==-1&&d.box.x==110&&d.box.y==644&&std::fabs(d.box.w-180.f*975/1000)<.0001f)scaledBar=true;}assert(hpText&&scaledBar);
 g.attacks.push_back(Attack(1,g.x+250));g.attacks.back().age=.949f;step(g,.01f);assert(g.hp==950);
 g.set(M2_LOST);g.activate();assert(g.hp==1000);
 const int dimensions[4][2]={{1920,1080},{2560,1440},{1920,1200},{1280,1024}};
 for(int j=0;j<100;++j){v.resize(dimensions[j%4][0],dimensions[j%4][1]);assert(v.w<=dimensions[j%4][0]&&v.h<=dimensions[j%4][1]);Point c=v.mouse(dimensions[j%4][0]/2,dimensions[j%4][1]/2,dimensions[j%4][1]);assert(std::fabs(c.x-640)<1&&std::fabs(c.y-360)<1);v.resize(1280,720);assert(v.x==0&&v.y==0&&v.w==1280&&v.h==720);}
 std::cout<<"PASS hotfix: 1000 HP initial/retry, HUD 1000/1000 and 975/1000, normalized health bar, 1000 -> 975 -> 950, repeated logical viewport sizing. Native F11 runtime verification remains pending.\n";
 // Maximize patch: reject black bars and keep every button mapped across resize cycles.
 for(int j=0;j<100;++j){int cw=dimensions[j%4][0],ch=dimensions[j%4][1];v.resize(cw,ch);
  for(int k=0;k<5;++k){Rect b=ui.buttonBox(states[k]);int px=int(v.x+(b.x+b.w/2)*v.w/1280),py=int(ch-v.y-(b.y+b.h/2)*v.h/720);
   assert(v.containsPixel(px,py,ch)&&b.contains(v.mouse(px,py,ch)));}
  if(v.y>0){assert(!v.containsPixel(cw/2,0,ch));assert(!v.containsPixel(cw/2,ch-1,ch));}
  if(v.x>0){assert(!v.containsPixel(0,ch/2,ch));assert(!v.containsPixel(cw-1,ch/2,ch));}
  v.resize(1280,720);assert(v.containsPixel(0,0,720)&&v.containsPixel(1279,719,720));
 }
 // Deliberately unequal advances/bearings establish exact ink-centering math.
 int advances[96]={0};GlyphInk ink[96]={{0,0,0,0}};std::string label="ENTER";
 for(size_t i=0;i<label.size();++i){int n=label[i]-32;advances[n]=17+n%7;ink[n].left=1+n%3;ink[n].right=advances[n]-2;ink[n].top=5+n%2;ink[n].bottom=31;}
 g.state=M2_START;ui.draw(g);Rect b=ui.buttonBox(g.state);bool anchored=false;
 for(size_t i=0;i<ui.list.size();++i){const Draw&d=ui.list[i];if(d.text=="ENTER"){assert(d.centerInk&&d.box.x==b.x+b.w/2&&d.box.y==b.y+b.h/2);anchored=true;}}
 assert(anchored);Point c(b.x+b.w/2,b.y+b.h/2);Point origin=centeredInkOrigin(label,c,24,advances,ink,34);
 int pen=0;float l=10000,r=-10000,t=10000,bot=-10000;
 for(size_t i=0;i<label.size();++i){int n=label[i]-32;l=std::min(l,float(pen+ink[n].left));r=std::max(r,float(pen+ink[n].right));t=std::min(t,float(ink[n].top));bot=std::max(bot,float(ink[n].bottom));pen+=advances[n];}
 assert(std::fabs(origin.x+(l+r)*.5f*24/36-c.x)<.0001f);
 assert(std::fabs(origin.y+(34-(t+bot)*.5f)*24/36-c.y)<.0001f);
 for(int j=0;j<4;++j){v.resize(dimensions[j][0],dimensions[j][1]);float visibleCenter=origin.x+(l+r)*.5f*24/36;assert(std::fabs((v.x+visibleCenter*v.w/1280)-(v.x+c.x*v.w/1280))<.001f);}
 std::cout<<"PASS maximize patch: 100 resize/mouse cycles, all five button hitboxes, bar rejection, exact entry rectangle anchor, glyph ink/baseline centering and scaled alignment math. Native maximize/restore visual verification pending.\n";
 // UI cleanup: image AND measured-text anchors share the exact entry-region center.
 Rect region=ui.entryButtonRegion();Rect entry=ui.buttonBox(M2_START);
 assert(std::fabs(entry.w-366.f*360/425)<.001f&&entry.h==62);
 assert(entry.x+entry.w/2==region.x+region.w/2&&entry.y+entry.h/2==region.y+region.h/2);
 for(int mode=0;mode<3;++mode){g.state=M2_START;ui.mouse=mode?Point(entry.x+entry.w/2,entry.y+entry.h/2):Point(-1,-1);ui.down=mode==2;ui.draw(g);
  bool imageCentered=false,textCentered=false;for(size_t i=0;i<ui.list.size();++i){const Draw&d=ui.list[i];
   if(d.asset==buttons[0][mode]){Rect source=ui.entrySourceFrame(d.asset);const AssetInfo&a=assetInfo[d.asset];
    float cx=d.box.x+(source.x+source.w/2)*d.box.w/a.w,cy=d.box.y+(source.y+source.h/2)*d.box.h/a.h;
    assert(!d.crop&&std::fabs(cx-(entry.x+entry.w/2))<.001f&&std::fabs(cy-(entry.y+entry.h/2))<.001f);imageCentered=true;}
   if(d.text=="ENTER"){assert(d.centerInk&&d.box.x==region.x+region.w/2&&d.box.y==region.y+region.h/2);textCentered=true;}}
  assert(imageCentered&&textCentered);
 }
 for(int state=0;state<=M2_HANDOFF_END;++state){g.state=State(state);ui.draw(g);for(size_t i=0;i<ui.list.size();++i)assert(ui.list[i].text.find("F11")==std::string::npos);}
 std::cout<<"PASS UI cleanup: full entry PNG preserves pixel scale and centers visible frame in every state; measured text shares center; F11 label absent from every screen.\n";
 std::cout<<"PASS: startup/state flow, timer isolation, movement, lanes/fallback, head-only collision, 10 damage, single-contact arrows, magazine/cooldown/reload, attack timing/overlap/living-head scheduler, dodge/25-damage-once, HP/time loss, retry reset, 30 hits, victory priority, reward/handoff, frames 01-06, viewport transform, presentation asset bounds.\n";
}
