#include "../../src/m1/Game.h"
#include "../../src/m1/Assets.h"
#include <set>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <cmath>
#define STBI_ONLY_PNG
#include "../../vendor/stb_image.h"
#ifdef RENDER_SMOKE
#include "../../src/m1/PlatformGL.h"
#include "../../src/m1/Renderer.h"
Renderer* gpu=0;
std::set<int> captures;
void capture(int scene){
 std::vector<unsigned char> data(1280*720*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,1280,720,GL_RGB,GL_UNSIGNED_BYTE,&data[0]);
 char name[80];std::sprintf(name,"visual_%02d.ppm",scene);std::ofstream f(name,std::ios::binary);f<<"P6\n1280 720\n255\n";for(int y=719;y>=0;--y)f.write((char*)&data[y*1280*3],1280*3);
}
#endif
using namespace Meridian;
std::set<std::string> seen;
void record(const Game& g){size_t old=seen.size();std::vector<Draw> d=g.draw(Input());for(size_t i=0;i<d.size();++i)if(d[i].kind==Draw::IMAGE||d[i].kind==Draw::REGION)seen.insert(d[i].value);
#ifdef RENDER_SMOKE
 bool snap=captures.count(g.scene)==0&&(g.time>.3f||g.scene==MENU||g.scene==CLAIMED);
 if(old!=seen.size()||snap){glClear(GL_COLOR_BUFFER_BIT);gpu->paint(d);glFinish();assert(glGetError()==GL_NO_ERROR);if(snap){capture(g.scene);captures.insert(g.scene);}}
#else
 (void)old;
#endif
}
void step(Game& g,const Input& in,float seconds){for(int i=0;i<int(seconds*60);++i){g.update(1.0f/60,in);record(g);}}
Input enter(){Input i;i.enter=true;return i;}
Input click(float y){Input i;i.click=true;i.mx=836;i.my=y+50;return i;}
#include "UpgradeChecks.h"
int main(int argc,char** argv){
#ifdef RENDER_SMOKE
 glutInit(&argc,argv);glutInitDisplayMode(GLUT_RGBA|GLUT_SINGLE);glutInitWindowSize(1280,720);glutCreateWindow("SS Meridian - automated visual verification");glViewport(0,0,1280,720);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,WIDTH,HEIGHT,0,-1,1);glMatrixMode(GL_MODELVIEW);gpu=new Renderer;
#else
 (void)argc;(void)argv;
#endif
 for(int i=0;i<ASSET_COUNT;++i){int w,h,n;unsigned char* p=stbi_load(ASSETS[i].path,&w,&h,&n,4);assert(p&&w==ASSETS[i].width&&h==ASSETS[i].height);stbi_image_free(p);}std::cout<<ASSET_COUNT<<" PNGs decoded by the actual C++ runtime loader\n";
 Game g;record(g);g.update(.016f,click(650));Scene last=g.scene;int ticks=0;float seconds[25]={0};
 while(g.scene!=CLAIMED&&ticks++<36000){Input i;switch(g.scene){
 case SHIP_A:i.d=true;break;
 case SHIP_B:if(g.y>625)i.w=true;else if(g.x<1110)i.d=true;else i.enter=true;break;
 case ARENA:if(g.y>805)i.w=true;else if(g.x<950)i.d=true;break;
 case DIALOGUE:if(ticks%30==0)i.enter=true;break;
 case CHOICE:i=click(550);break;
 case R1:
  if(g.section<2)i.d=true;
  else {if(g.x<810)i.d=true;else if(g.x>860)i.a=true;if(g.y>570)i.w=true;i.enter=true;}
  for(size_t e=0;e<g.enemies.size();++e)if(!g.enemies[e].removed&&g.enemies[e].hp>0&&distance(Point(g.x,g.y),Point(g.enemies[e].x,g.enemies[e].y))<520)i.attack=true;
  break;
 case R2:{Point exitPoint=g.section==0?Point(1415,780):g.section==1?Point(1440,780):Point(1380,780);Point next=roomNavigation(g.section).nextWaypoint(Point(g.x,g.y),exitPoint);float dx=next.x-g.x,dy=next.y-g.y;if(std::fabs(dx)>8){if(dx>0)i.d=true;else i.a=true;}if(std::fabs(dy)>8){if(dy>0)i.s=true;else i.w=true;}i.enter=true;break;}
 case R3:{bool danger=false;for(size_t e=0;e<g.enemies.size();++e)if(!g.enemies[e].removed&&distance(Point(g.x,g.y),Point(g.enemies[e].x,g.enemies[e].y))<165)danger=true;if(danger)i.attack=true;else i.w=true;break;}
 case CONNECTOR:case COCKPIT:case DRIVE:case RAIL_EXIT:i.w=true;i.enter=true;break;
 case CONGRATS:i.enter=true;break;
 case REWARD:i=click(720);break;
 case RETRY:std::cerr<<"Unexpected failure in successful route: "<<g.retryScene<<"\n";return 2;
 default:break;}
 seconds[g.scene]+=1.0f/60;g.update(1.0f/60,i);record(g);if(last!=g.scene){std::cout<<g.sceneName()<<"\n";last=g.scene;}
 }
 assert(g.scene==CLAIMED&&g.hasSpaceStone&&g.mission1Complete);step(g,click(720),2);assert(g.scene==CLAIMED);std::cout<<"Input-driven full route PASS. R1="<<seconds[R1]<<"s R2="<<seconds[R2]<<"s R3="<<seconds[R3]+seconds[CONNECTOR]+seconds[DRIVE]+seconds[RAIL_EXIT]<<"s\n";
 // Every directional walk frame, eye band and Guide first idle remains reachable before approach.
 g.start(ARENA);Input i;i.d=true;step(g,i,5.7f);i=Input();i.a=true;step(g,i,5.7f);i=Input();i.w=true;step(g,i,.55f);i=Input();i.s=true;step(g,i,.55f);
 g.start(DIALOGUE);for(int n=0;n<GUIDE_DIALOGUE_COUNT;++n){record(g);g.update(.016f,enter());}assert(g.scene==CHOICE);g.update(.016f,click(685));record(g);assert(g.scene==COWARD);g.update(.016f,enter());assert(g.scene==MENU&&!g.hasSpaceStone);
 // Combat failure / retry and hit/death reactions for every supplied enemy family.
 for(int sw=0;sw<2;++sw){g.start(sw?R3:R1);step(g,Input(),40);assert(g.scene==RETRY);record(g);g.update(.016f,enter());assert(g.scene==(sw?R3:R1));}
 g.start(R2);g.monsterX=g.x+45;g.monsterY=g.y;g.monsterFacingX=-1;g.monsterFacingY=0;step(g,Input(),4);assert(g.scene==RETRY);record(g);g.update(.016f,enter());assert(g.scene==R2);
 g.start(DRIVE);step(g,Input(),2);assert(g.progress==0);i=Input();i.w=true;step(g,i,10);float before=g.progress;step(g,Input(),2);assert(g.progress==before);
 // Isolate enemy encounter fixtures to exercise each attack pose before defeating it through input.
 for(int sw=0;sw<2;++sw)for(int type=0;type<(sw?2:3);++type){g.start(sw?R3:R1);g.enemies.clear();g.spawnTimer=1000;g.enemies.push_back(Enemy(g.x+80,g.y,type));step(g,Input(),1.6f);i=Input();i.attack=true;step(g,i,3);assert(g.enemies[0].removed);}
 // Deliberate environment misses alternate both impact and muzzle effects.
 g.start(R1);g.enemies.clear();i=Input();i.attack=true;step(g,i,2);
 // Reward anticipation should show all three glow stages before the player claims.
 g.start(REWARD);step(g,Input(),2);
 upgradeChecks();
 std::ofstream f("smoke_asset_coverage.tsv");int count=0;for(int j=0;j<ASSET_COUNT;++j){bool ok=seen.count(ASSETS[j].key)!=0;count+=ok;f<<ASSETS[j].path<<"\t"<<(ok?"render command exercised":"loaded but optional/unused in current gameplay")<<"\n";}std::cout<<"Asset runtime render-command coverage "<<count<<"/"<<ASSET_COUNT<<" (all "<<ASSET_COUNT<<" PNGs decoded successfully)\n";assert(count>=ASSET_COUNT-8);
 #ifdef RENDER_SMOKE
 gpu->report();assert(gpu->rendered.size()==ASSET_COUNT);std::cout<<"OpenGL upload and draw coverage verified; no GL errors\n";
#endif
 std::cout<<"PASS: complete route, alternate choice, scene-local retries, train input gating, reward ownership, endpoint lock\n";
}
