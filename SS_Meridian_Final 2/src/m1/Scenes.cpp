#include "Game.h"
#include "Assets.h"
#include "Animation.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
namespace Meridian {
namespace {
std::string frame(const char* prefix,int n,const char* suffix=""){char b[128];std::sprintf(b,"%s%02d%s",prefix,n,suffix);return b;}
float clamp01(float v){return std::max(0.0f,std::min(1.0f,v));}
std::string arrow(Point from,Point to){float dx=to.x-from.x,dy=to.y-from.y;if(std::fabs(dx)>std::fabs(dy))return dx>=0?">>>":"<<<";return dy>=0?"vvv":"^^^";}
struct Painter {
 std::vector<Draw> out;
 void image(const std::string& a,float x,float y,float w,float h,bool flip=false,float opacity=1){out.push_back(Draw(Draw::IMAGE,a,x,y,w,h,opacity,flip));}
 void region(const std::string& a,float x,float y,float w,float h,float sx,float sy,float sw,float sh,float opacity=1){Draw d(Draw::REGION,a,x,y,w,h,opacity,false);d.sx=sx;d.sy=sy;d.sw=sw;d.sh=sh;out.push_back(d);}
 void bg(const std::string& a){image(a,0,0,WIDTH,HEIGHT);}
 void sprite(const std::string& a,float x,float feet,float height,bool flip=false){float w=height;for(int i=0;i<ASSET_COUNT;++i)if(a==ASSETS[i].key){w=height*ASSETS[i].width/ASSETS[i].height;break;}image(a,x-w/2,feet-height,w,height,flip);}
 void actor(const std::string& a,float x,float feet,float height,bool flip=false){for(int i=0;i<ASSET_COUNT;++i)if(a==ASSETS[i].key){SpriteCalibration c=calibration(a);float scale=height/c.standingHeight;image(a,x-(flip?ASSETS[i].width-c.anchorX:c.anchorX)*scale,feet-c.anchorY*scale,ASSETS[i].width*scale,ASSETS[i].height*scale,flip);return;}}
 // Atlas regions are measured from the alpha silhouettes; constant scale avoids breathing/shrinking.
 void walkActor(int direction,int frame,float x,float feet,float height){
  struct Pose {float x,y,w,h,anchor;};
  static const Pose poses[24]={
  {30,8,185,284,87.641f},
  {280,6,124,285,59.151f},
  {510,4,80,288,45.097f},
  {698,11,143,281,87.932f},
  {920,7,184,284,88.330f},
  {1177,5,124,287,61.661f},
  {1396,8,101,283,51.395f},
  {1576,9,187,283,97.456f},
  {61,298,111,287,49.167f},
  {284,297,112,286,49.495f},
  {502,297,108,294,50.778f},
  {724,297,109,288,53.190f},
  {948,298,108,293,51.771f},
  {1173,297,109,285,49.367f},
  {1392,297,112,294,53.780f},
  {1616,298,110,293,51.896f},
  {50,591,115,283,63.713f},
  {272,591,116,283,64.201f},
  {493,591,115,283,63.108f},
  {717,591,118,283,64.502f},
  {938,591,116,283,62.849f},
  {1159,591,115,283,64.269f},
  {1382,591,117,283,63.034f},
  {1602,591,124,283,67.994f}};
  int row=direction==0?1:direction==1?2:0;
  const Pose& a=poses[row*8+frame];bool flip=direction==2;
  float scale=height/285.0f;
  Draw d(Draw::REGION,"ship_walk_atlas",x-(flip?a.w-a.anchor:a.anchor)*scale,feet-a.h*scale,a.w*scale,a.h*scale,1,flip);
  d.sx=a.x/1774.0f;d.sy=a.y/887.0f;d.sw=a.w/1774.0f;d.sh=a.h/887.0f;out.push_back(d);
 }
 void shadow(float x,float y,float width){out.push_back(Draw(Draw::SHADOW,"",x-width/2,y-5,width,10,.25f));}
 void patch(const std::string& key,const Polygon& shape){Draw d(Draw::PATCH,key,0,0,WIDTH,HEIGHT);d.polygon=shape.points;out.push_back(d);}
 void text(const std::string& t,float x,float y,float size=28){out.push_back(Draw(Draw::TEXT,t,x,y,size,0));}
 void box(float x,float y,float w,float h,float a=.85f){out.push_back(Draw(Draw::BOX,"",x,y,w,h,a));}
 void center(const std::string& s,float y,float size=32){text(s,-1,y,size);}
 void hud(const std::string& title,const std::string& hint){box(30,24,1612,90,.8f);text(title,58,58,30);text(hint,58,93,22);}
 void button(const std::string& label,float y,const Input& in){bool hover=in.mx>=636&&in.mx<=1036&&in.my>=y&&in.my<=y+100;image("button_frame",636,y,400,100,false,hover?1:.85f);center(label,y+59,27);}
};
const char* walks[]={"walk_down_","walk_up_","walk_left_","walk_right_"};
const char* zombieSuffix[]={"_01_idle","_02_walk1","_03_walk2","_04_attack","_05_hit","_06_death"};
const char* ghostSuffix[]={"_01_idle","_02_advance1","_03_advance2","_04_attack","_05_hit","_06_dissolve"};
const char* gun[]={"gun_01_idle","gun_02_aim","gun_03_fire","gun_04_recovery","gun_06_aim_return"};
const char* sword[]={"sword_01_ready","sword_02_windup","sword_03_slash","sword_04_followthrough","sword_05_return"};
const char* monster[]={"monster_01_idle","monster_02_step1","monster_03_step2","monster_04_turn","monster_05_chase","monster_06_catch"};
const char* dialogue[]={"What is your name?","Issac.","Issac...","If you want to get out of this, you will need all three stones.","Space.","Mind.","Power.","One mission. One stone.","Complete three missions. Collect all three.","Then put them into the Hand of Three.","Are you ready?"};
static_assert(sizeof(dialogue)/sizeof(dialogue[0])==GUIDE_DIALOGUE_COUNT,"Guide dialogue count mismatch");
const char* stones[]={"space_stone","mind_stone","power_stone"};
}

std::vector<Draw> Game::draw(const Input& in)const{
 Painter p;int walkFrame=moving?1+int(anim*8)%4:1;
 switch(scene){
 case MENU:
  p.sprite("issac_fullbody_master",1220,865,750);p.text("SS MERIDIAN",245,345,65);p.text("MISSION 1 / THE SPACE STONE",250,400,25);p.text("WASD to move   |   ENTER to interact   |   SPACE to attack",250,465,22);p.button("START",650,in);break;
 case ARRIVAL:p.bg("ship_exterior");break;
 case SHIP_A:case SHIP_B:case PICKUP:{
  bool a=scene==SHIP_A;p.bg(a?"ship_interior_a":"ship_interior_b");if(!a&&scene!=PICKUP)p.sprite("hand_of_three_floor",1130,630,90);
  if(scene==PICKUP){p.actor(frame("hand_pickup_",std::min(4,1+int(time*4))),1130,640,185);p.sprite("hand_of_three_floor",1120,638-std::max(0.0f,time-.5f)*45,55);}else {p.shadow(x,y,42);p.walkActor(direction,moving?int(anim*Tuning::walk()/150.0f*8)%8:2,x,y,185);}
  p.bg(a?"ship_foreground_a":"ship_foreground_b");if(time>2&&scene!=PICKUP)p.hud("SS MERIDIAN",a?"WASD - walk across the deck to the right":"WASD - approach the empty Hand of Three");
  if(scene==SHIP_B&&std::sqrt((x-1130)*(x-1130)+(y-620)*(y-620))<145){p.box(525,825,622,70);p.center("PRESS ENTER TO PICK UP",870,29);}break;}
 case COUNTDOWN:p.center(frame("",1+int(time)).substr(1)+"...",500,72);break;
 case ARENA:case DIALOGUE:case CHOICE:{
  p.bg("main_arena_background");p.bg("main_arena_foreground");const char* eye[]={"eye_far_left","eye_left","eye_slight_left","eye_center","eye_slight_right","eye_right","eye_far_right"};float gaze=3.0f+3.0f*std::sin(clock*1.15f);int band=std::min(5,int(gaze));float blend=gaze-band;p.sprite(eye[band],836,470,360);if(blend>0)p.image(eye[band+1],836-360.0f*323/636/2,110,360.0f*323/636,360,false,blend);
  std::string guide=scene==ARENA&&time<3?"guide_arena_idle":frame("guide_idle_",1+int(clock*3)%4);
  std::string player=(scene==DIALOGUE||scene==CHOICE)?"issac_facing_guide":frame(walks[direction],walkFrame);
  p.shadow(x,y,45);if(guideActive)p.shadow(GUIDE_FOOT.x,GUIDE_FOOT.y,50);
  if(guideActive&&GUIDE_FOOT.y<=y)p.actor(guide,GUIDE_FOOT.x,GUIDE_FOOT.y,230);
  p.actor(player,x,y,arenaActorHeight(y));
  if(guideActive&&GUIDE_FOOT.y>y)p.actor(guide,GUIDE_FOOT.x,GUIDE_FOOT.y,230);
  Polygon edge;edge.points.push_back(Point(0,0));edge.points.push_back(Point(195,0));edge.points.push_back(Point(150,450));edge.points.push_back(Point(235,565));edge.points.push_back(Point(225,790));edge.points.push_back(Point(290,815));edge.points.push_back(Point(280,941));edge.points.push_back(Point(0,941));
  p.patch("main_arena_foreground",edge);for(size_t i=0;i<edge.points.size();++i)edge.points[i].x=WIDTH-edge.points[i].x;p.patch("main_arena_foreground",edge);
  if(scene==ARENA)p.hud("THE MAIN ARENA","WASD - stay on the floor. Approach the grounded Guide on the right.");
  if(scene==DIALOGUE){
   p.box(80,680,1512,240,.92f);p.image("dialogue_bubble",360,672,1200,240);p.sprite(line==1?"issac_dialogue_portrait":"guide_dialogue_portrait",235,912,240);
   p.text(std::string("~")+(line==1?"ISSAC":"GUIDE"),430,723,23);p.text(std::string("~")+dialogue[line],430,770,25);p.text("~ENTER - continue",430,818,20);
   if(line>=4&&line<=6)p.sprite(stones[line-4],836,600,180);
   if(line>=7)for(int stone=0;stone<3;++stone)p.sprite(stones[stone],636+stone*200.0f,600,160);
  }
  if(scene==CHOICE){p.box(550,470,572,350,.85f);p.center("ARE YOU READY?",515,32);p.button("HELL YEAHHH",550,in);p.button("HELL NAWWWWW",685,in);}break;}
 case COWARD:p.bg("hell_naw_elimination_background");p.box(420,300,832,380);p.center("U are a coward.",390,40);p.center("Cryyy, Lil Brooo.",455,40);p.center("Go Hit the Gym",520,40);p.center("PRESS ENTER",620,28);break;

 case R1:{
  const char* bg[]={"reality1_street_a","reality1_street_b","reality1_street_c"};p.bg(bg[section]);
  for(size_t j=0;j<enemies.size();++j){
   const Enemy& e=enemies[j];if(e.removed)continue;char type=char('a'+e.type);std::string key="zombie_";key+=type;key+=zombieSuffix[e.pose];
   // Supplied zombie art faces screen-right. Flip enemies on Issac's right so they face him instead of moonwalking.
   bool flip=e.x>x;p.shadow(e.x,e.y,45);p.sprite(key,e.x,e.y,220,flip);
  }
  int pose=action<0?0:std::min(4,1+int(action/.105f));std::string player=hurt>0?"gun_05_hit":gun[pose];
  if(moving&&action<0&&hurt<=0)player=frame("gun_run_",1+int(anim*10)%6);
  p.shadow(x,y,42);p.actor(player,x,y,190,direction==2);
  if(action>=.16f&&action<.34f)p.sprite(shots%2?"muzzle_flash_small":"muzzle_flash_large",x+(direction==2?-63:63),y-100,65,direction==2);
  for(size_t j=0;j<effects.size();++j){const Effect& e=effects[j];if(e.time<.25f)p.sprite(e.hit?"hit_flash_red":shots%2?"impact_concrete":"impact_dust",e.x,e.y+40,110);}
  char title[120];std::sprintf(title,"REALITY 1 / STREET %d     HEALTH %d",section+1,hp);
  if(section<2){p.hud(title,"KEEP MOVING >>>   |   SPACE - fire   |   Horde keeps spawning");p.center("EXIT  >>>",145,26);}
  else{
   p.hud(title,"REACH THE WHITE EXIT   |   SPACE - fire   |   ENTER at the door");
   p.shadow(836,515,34);p.sprite("reality1_exit_white",836,515,190);p.center("EXIT  ^^^",145,28);
   if(distance(Point(x,y),Point(836,555))<145)p.center("ENTER - EXIT",865,28);
  }
  break;}

 case R2:{
  const char* rooms[]={"reality2_room_a","reality2_room_b","reality2_room_c"};p.bg(rooms[section]);
  const Navigation& nav=roomNavigation(section);
  struct Layer {float depth;int kind,index;Layer(float d,int k,int i):depth(d),kind(k),index(i){}};
  std::vector<Layer> layers;layers.push_back(Layer(y,0,0));layers.push_back(Layer(monsterY,1,0));for(size_t i=0;i<nav.covers.size();++i)layers.push_back(Layer(nav.covers[i].depth,2,int(i)));
  std::stable_sort(layers.begin(),layers.end(),[](const Layer& a,const Layer& b){return a.depth<b.depth;});
  for(size_t i=0;i<layers.size();++i){
   if(layers[i].kind==2)p.patch(rooms[section],nav.covers[layers[i].index].silhouette);
   else if(layers[i].kind==1){p.shadow(monsterX,monsterY,62);p.sprite(monster[monsterPose],monsterX,monsterY,330,monsterFacingX>0);}
   else{
    std::string key=frame(walks[direction],1);bool flip=false;
    if(hideState!=EXPOSED)key=hideState==HIDE_ENTER?"hide_01_enter":hideState==HIDDEN?"hide_02_idle":"hide_03_exit";
    else if(moving){key=frame(direction==1?"run_back_":direction==0?"run_front_":"run_right_",1+int(anim*10)%6);flip=direction==2;}
    p.shadow(x,y,40);p.actor(key,x,y,190,flip);
   }
  }
  if(section==2){p.shadow(1385,850,42);p.sprite("reality2_exit_red",1385,850,275);p.text("EXIT",1340,535,28);}
  Point exitPoint=section==0?Point(1415,780):section==1?Point(1440,780):Point(1380,780);Point next=nav.nextWaypoint(Point(x,y),exitPoint);if(distance(next,Point(x,y))<8)next=exitPoint;
  std::string objective="OBJECTIVE  "+arrow(Point(x,y),next);
  const char* threat=monsterState==CHASE?"MONSTER CHASING - break line of sight and use cover":monsterState==SEARCH?"MONSTER SEARCHING - stay quiet behind cover":"FOLLOW THE OBJECTIVE ARROW   |   E - hide near cover";
  char roomTitle[80];std::sprintf(roomTitle,"REALITY 2 / ROOM %d",section+1);p.hud(roomTitle,threat);p.center(objective,145,29);
  if(hideState!=EXPOSED)p.center(hideWitnessed?"IT SAW YOU - LEAVE AND CHANGE COVER":"E - LEAVE COVER",865,25);else if(nearbyHide()>=0)p.center("E - HIDE",865,25);
  if(section==2&&distance(Point(x,y),Point(1380,780))<105)p.center("ENTER - EXIT",865,28);
  break;}

 case R3:{
  p.bg(section==0?"train_cabin_1":"train_cabin_2");
  if(section==1)p.out.push_back(Draw(Draw::BOX,"r3_ladder",798,232,76,340));
  for(size_t j=0;j<enemies.size();++j){
   const Enemy& e=enemies[j];if(e.removed)continue;char type=char('a'+e.type);std::string key="ghost_";key+=type;key+=ghostSuffix[e.pose];
   float depth=clamp01((e.y-315)/525.0f);float h=155+85*depth;bool flip=e.x>x;
   p.shadow(e.x,e.y,38+30*depth);p.sprite(key,e.x,e.y,h,flip);
   if(e.pose==5)p.sprite(frame("spectral_",std::min(4,1+int(e.time/.175f))),e.x,e.y,h*.88f);
  }
  std::string player;bool flip=false;
  if(hurt>0)player="sword_06_hit";
  else if(action>=0){int pose=std::min(4,1+int(action/.16f));player=sword[pose];}
  else if(moving){player=frame(direction==1?"run_back_":direction==0?"run_front_":"run_right_",1+int(anim*10)%6);flip=direction==2;}
  else player="sword_01_ready";
  p.shadow(x,y,42);
  if(action>=0||hurt>0||!moving)p.sprite(player,x,y,235,direction==2);else p.actor(player,x,y,190,flip);
  if(action>=.16f){p.sprite(frame("slash_",std::min(4,1+int((action-.16f)/.12f))),x+(direction==2?-80:80),y-55,175,direction==2);}
  for(size_t j=0;j<effects.size();++j){const Effect& e=effects[j];if(e.hit)p.sprite(frame("impact_",std::min(4,1+int(e.time/.1625f))),e.x,e.y+50,150);}
  char title[120];std::sprintf(title,"REALITY 3 / CABIN %d     HEALTH %d",section+1,hp);p.hud(title,"RUN TO THE END OF THE CABIN ^^^   |   SPACE - sword when a ghost gets close");p.center("NEXT CABIN  ^^^",145,27);
  break;}

 case CONNECTOR:{
  p.bg("train_connector");p.out.push_back(Draw(Draw::BOX,"r3_ladder",790,263,60,274));std::string key=frame("run_back_",moving?1+int(anim*10)%6:1);p.shadow(x,y,42);p.actor(key,x,y,190);p.hud("REALITY 3 / CONNECTOR","KEEP RUNNING ^^^ - the next cabin triggers automatically at the end");p.center("NEXT  ^^^",145,27);break;}

 case COCKPIT:
  // Fixed cab foreground + supplied railway behind it. The windshield remains the world view.
  p.bg("train_track_loop");p.bg("train_cockpit");
  p.hud("REALITY 3 / DRIVER CAB","HOLD W (or ENTER) TO START - the railway will move outside the windshield");break;

 case DRIVE:{
  // Keep the cockpit physically stable. Motion is confined to the world outside the windshield.
  p.bg("train_track_loop");
  const float horizon=.45f;
  const int bands=22;
  float phase=std::fmod(clock*.36f,1.0f);
  for(int i=0;i<bands;++i){
   float d0=float(i)/bands,d1=float(i+1)/bands;
   float y0=(horizon+d0*(1-horizon))*HEIGHT;
   float y1=(horizon+d1*(1-horizon))*HEIGHT;
   // Near bands flow faster than distant bands; wrapping continuously feeds new track from the horizon.
   float shift=phase*(.18f+.78f*d0);
   float src=std::fmod(d0+shift,1.0f);
   float srcNext=src+(d1-d0);
   if(srcNext<=1.0f)p.region("train_track_loop",0,y0,WIDTH,y1-y0,0,horizon+src*(1-horizon),1,(d1-d0)*(1-horizon));
   else{
    float first=(1.0f-src)/(d1-d0);float mid=y0+(y1-y0)*first;
    p.region("train_track_loop",0,y0,WIDTH,mid-y0,0,horizon+src*(1-horizon),1,(1.0f-src)*(1-horizon));
    p.region("train_track_loop",0,mid,WIDTH,y1-mid,0,horizon,1,(srcNext-1.0f)*(1-horizon));
   }
  }
  // A tiny world sway sells speed without making the driver's cab float.
  float q=clamp01(progress/Tuning::train());float streak=std::fmod(clock*1.8f,1.0f);
  for(int i=0;i<7;++i){float t=std::fmod(streak+i/7.0f,1.0f);float yy=520+t*t*410;p.out.push_back(Draw(Draw::BOX,"rail",815-260*t,yy,520*t,2,.08f+.16f*t));}
  p.bg("train_cockpit");
  p.hud("REALITY 3 / DRIVING","HOLD W - keep the train moving. Track and scenery move outside; cockpit stays fixed.");
  char b[80];std::sprintf(b,"TRACK PROGRESS  %d%%",int(q*100));p.center(b,880,25);break;}

 case RAIL_EXIT:{
  // Keep the cab present even during the final approach so this remains a driver's-eye sequence.
  float z=1+(progress-20)*.22f;p.image("reality3_railway_exit",836-836*z,470-470*z,1672*z,941*z);p.bg("train_cockpit");
  p.hud("REALITY 3 / EXIT AHEAD","HOLD W - stay in the cab and drive through the exit");break;}
 case CONGRATS:p.center("CONGRATS",450,60);p.center("PRESS ENTER",550,30);break;


 case COMPLETE:p.center("MISSION 1 COMPLETE",470,55);break;
 case REWARD:case CLAIM_FLASH:case CLAIMED:{
  p.bg("mission1_reward_background");if(scene==CLAIMED){p.bg("mission1_claimed_panel");p.sprite("space_stone",325,485,150);p.text("SPACE STONE: CLAIMED",620,450,36);p.text("MISSION 1 COMPLETE",620,505,25);p.button("MISSION 2 - ENTER",720,in);}else{p.center("SPACE STONE",200,52);const char* glow[]={"space_glow_01_faint","space_glow_02_medium","space_glow_03_bright"};p.image(scene==CLAIM_FLASH?"space_glow_04_claim_flash":glow[int(time*3)%3],466,245,740,475);p.sprite("space_stone",836,600,300);if(scene==REWARD)p.button("CLAIM",720,in);}break;}
 case RETRY:
  if(retryScene==R2)p.sprite("reality2_catch_pose",836,680,510);
  p.box(400,695,872,205);p.center("ELIMINATED",755,42);p.center("PRESS ENTER - RETRY THIS SCENE",835,28);break;
 }
 return p.out;
}
}
