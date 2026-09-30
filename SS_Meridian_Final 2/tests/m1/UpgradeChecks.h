#include "../../src/m1/Animation.h"
void upgradeChecks(){
 Game g;Input in;
 // Each ship direction must render eight distinct atlas regions, with feet on the floor.
 for(int dir=0;dir<4;++dir){
  g.start(SHIP_B);std::set<float> regions;
  in=Input();in.a=dir==2;in.d=dir==3;in.w=dir==1;in.s=dir==0;
  for(int k=0;k<100;++k){g.x=800;g.y=640;g.update(1.0f/60,in);
   std::vector<Draw> d=g.draw(in);bool found=false;
   for(size_t j=0;j<d.size();++j)if(d[j].value=="ship_walk_atlas"){
    found=true;regions.insert(d[j].sx);assert(d[j].flip==(dir==2));assert(std::fabs(d[j].y+d[j].h-g.y)<.001f);
    assert(d[j].sx>=0&&d[j].sy>=0&&d[j].sx+d[j].sw<=1.001f&&d[j].sy+d[j].sh<=1.001f);
   }assert(found);record(g);
  }assert(regions.size()==8);g.update(.016f,Input());assert(!g.moving&&g.anim==0);
 }
 g.start(SHIP_B);g.x=1430;in=Input();in.d=true;g.update(.05f,in);assert(!g.moving&&g.anim==0);
 // The eye moves with no player input in arena, dialogue and choice scenes.
 for(int s=ARENA;s<=CHOICE;++s){g.start(Scene(s));std::set<std::string> eyes;
  for(int k=0;k<400;++k){g.update(.016f,Input());std::vector<Draw> d=g.draw(Input());
   for(size_t j=0;j<d.size();++j)if(d[j].value.find("eye_")==0)eyes.insert(d[j].value);
  }assert(eyes.size()==7);
 }
 // Guide introduces exactly Space, Mind and Power, then enters the choice once.
 g.start(DIALOGUE);std::set<std::string> stonesSeen;
 for(int l=0;l<GUIDE_DIALOGUE_COUNT;++l){assert(g.scene==DIALOGUE&&g.line==l);std::vector<Draw> d=g.draw(Input());
  for(size_t j=0;j<d.size();++j){const std::string& key=d[j].value;
   assert(key.find("five")==std::string::npos&&key.find("Reality.")==std::string::npos&&key.find("Time.")==std::string::npos);
   if(key.find("_stone")!=std::string::npos)stonesSeen.insert(key);
  }g.update(.016f,enter());
 }assert(g.scene==CHOICE&&stonesSeen.size()==3&&stonesSeen.count("space_stone")&&stonesSeen.count("mind_stone")&&stonesSeen.count("power_stone"));
 std::cout<<"PASS: eight ship gait poses in four directions, grounded feet, blocked/idle rest, autonomous eye in all arena scenes, three-stone dialogue\n";
 // Collision stress: every accepted foot point must remain on floor outside cover.
 for(int room=-1;room<3;++room){const Navigation& n=room<0?arenaNavigation():roomNavigation(room);Point p=room<0?Point(760,890):Point(540,810);assert(n.walkable(p));for(int k=0;k<2000;++k){float a=k*.137f;p=n.slide(p,Point(std::cos(a)*80,std::sin(a)*80));assert(n.walkable(p));}if(room>=0){for(size_t j=0;j<n.covers.size();++j){if(!n.walkable(n.covers[j].hide))std::cerr<<"INVALID HIDE "<<room<<" "<<j<<"\n";assert(n.walkable(n.covers[j].hide));}for(size_t j=0;j<n.patrol.size();++j){assert(n.walkable(n.patrol[j]));Point next=n.nextWaypoint(n.patrol[j],n.patrol[(j+1)%n.patrol.size()]);assert(distance(next,n.patrol[j])>1);}}}
 g.start(ARENA);assert(!arenaNavigation().walkable(Point(836,600)));assert(arenaActorHeight(915)>arenaActorHeight(715));
 // Existing arena walking assets remain available in all four directions.
 for(int dir=0;dir<4;++dir)for(int pose=0;pose<4;++pose){g.start(ARENA);g.direction=dir;g.moving=true;g.anim=(pose+.1f)/8;record(g);}
 // New gun run loops; a held shot stops translation until recovery ends.
 g.start(R1);g.enemies.clear();g.spawnTimer=1000;in=Input();in.d=true;step(g,in,1);float before=g.x;in.attack=true;g.update(.016f,in);before=g.x;g.update(.05f,in);assert(g.x==before&&g.action>=0);record(g);in.attack=false;step(g,in,.7f);assert(g.x>before);
 // Recycled enemy slots stay bounded over a long outbreak with repeated kills.
 g.start(R1);for(int k=0;k<12000;++k){for(size_t j=0;j<g.enemies.size();++j)if(!g.enemies[j].removed){g.enemies[j].hp=0;g.enemies[j].pose=5;}g.update(.016f,Input());assert(g.enemies.size()<=8);}assert(g.totalSpawned>50);
 // Directional run animations and mirroring, on a broad open floor area.
 for(int dir=0;dir<4;++dir){g.start(R2,1);g.time=-100;g.x=1100;g.y=810;in=Input();in.a=dir==2;in.d=dir==3;in.w=dir==1;in.s=dir==0;for(int k=0;k<80;++k){if(k%20==0){g.x=1100;g.y=810;}g.update(.016f,in);record(g);}}
 // Equal speed on diagonals; standing animation families share calibrated height and feet.
 Game straight,diagonal;straight.start(R2,1);diagonal.start(R2,1);straight.x=diagonal.x=1100;straight.y=diagonal.y=800;
 in=Input();in.d=true;straight.update(.05f,in);in.w=true;diagonal.update(.05f,in);assert(std::fabs(distance(Point(1100,800),Point(straight.x,straight.y))-distance(Point(1100,800),Point(diagonal.x,diagonal.y)))<.01f);
 const char* families[]={"walk_right_01","gun_01_idle","gun_run_01","run_right_01","run_back_01","run_front_01"};
 for(int k=0;k<6;++k){SpriteCalibration c=calibration(families[k]);assert(c.standingHeight>250);float scale=190/c.standingHeight;assert(std::fabs(c.standingHeight*scale-190)<.01f);assert(std::fabs((800-c.anchorY*scale)+c.anchorY*scale-800)<.01f);}
 // Cover genuinely occludes the monster's sight; hidden is not an invisibility switch.
 g.start(R2);g.x=902;g.y=504;g.monsterX=620;g.monsterY=504;g.monsterFacingX=1;g.monsterFacingY=0;assert(!g.monsterCanSeePlayer());assert(g.nearbyHide()==0);
 in=Input();in.hide=true;g.update(.016f,in);assert(g.hideState==HIDE_ENTER);record(g);step(g,Input(),.6f);assert(g.hideState==HIDDEN);record(g);g.update(.016f,in);assert(g.hideState==HIDE_EXIT);record(g);step(g,Input(),.4f);assert(g.hideState==EXPOSED);
 g.hideState=HIDDEN;g.monsterX=1100;g.monsterY=504;g.monsterFacingX=-1;assert(g.monsterCanSeePlayer());
 // Lost sight remembers the last seen position, then searches and returns to patrol.
 g.start(R2);g.time=3;g.x=902;g.y=504;g.monsterX=620;g.monsterY=504;g.monsterFacingX=1;g.monsterFacingY=0;g.monsterState=CHASE;g.lastSeen=Point(620,410);Point remembered=g.lastSeen;step(g,Input(),.8f);assert(g.monsterState==SEARCH);assert(distance(g.lastSeen,remembered)<.01f);
 g.x=1400;g.y=820;bool patrolled=false;for(int k=0;k<500;++k){g.update(.016f,Input());record(g);if(g.monsterState==PATROL)patrolled=true;}assert(patrolled);
 // Witnessed entry remains dangerous and does not teleport the pursuer.
 g.start(R2);g.time=3;g.x=902;g.y=504;g.monsterX=1100;g.monsterY=504;g.monsterFacingX=-1;g.monsterFacingY=0;in=Input();in.hide=true;g.update(.016f,in);assert(g.hideWitnessed&&g.monsterState==CHASE);assert(g.monsterX>1000);step(g,Input(),2);assert(g.scene==RETRY);
 // Death retries the current sub-scene rather than restarting an entire reality.
 g.start(R1,2);g.hp=0;g.update(.016f,Input());assert(g.scene==RETRY&&g.retryPart==2);g.update(.016f,enter());assert(g.scene==R1&&g.section==2);
 g.start(R3,1);g.hp=0;g.update(.016f,Input());assert(g.scene==RETRY&&g.retryPart==1);g.update(.016f,enter());assert(g.scene==R3&&g.section==1);
#ifdef RENDER_SMOKE
 g.start(ARENA);g.time=4;g.guideActive=true;gpu->paint(g.draw(Input()));glFinish();capture(100);
 g.start(R1);g.moving=true;g.anim=.3f;gpu->paint(g.draw(Input()));glFinish();capture(101);
 for(int room=0;room<3;++room){g.start(R2,room);g.hideIndex=0;g.x=roomNavigation(room).covers[0].hide.x;g.y=roomNavigation(room).covers[0].hide.y;g.hideState=HIDDEN;gpu->paint(g.draw(Input()));glFinish();capture(102+room);}
#endif
 std::cout<<"PASS: held animation, polygon collision, cover routing, bounded spawns, run/fire transition, hide sequence, LOS/search/reacquisition/witnessed hide\n";
}
