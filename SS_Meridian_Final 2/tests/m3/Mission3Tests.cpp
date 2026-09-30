#include "Mission3.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace m3;
static int checks=0;
static void check(bool ok,const char* what){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",what);std::exit(1);}std::printf("PASS: %s\n",what);}
static void step(Mission& g,float seconds,Input in=Input()){int n=static_cast<int>(std::ceil(seconds*60));for(int i=0;i<n;++i)g.update(1.0f/60,in);}
static void enter(Mission& g){Input in;in.confirm=true;g.update(1.0f/60,in);}
static void start(Mission& g){enter(g);step(g,.6f);Input in;in.right=true;step(g,10,in);step(g,1.1f,in);}
static void fight(Mission& g){enter(g);step(g,1.25f);}
static void fixture(Mission& g,int n=0){g.current=n;g.state=Fight;g.remaining=60;g.player.x=250;g.player.health=100;g.monsters[n].x=490;g.monsters[n].health=g.config.monsterHealth[n];}
static void click(Mission& g,float x=640,float y=540){g.pointer(x,y,true,false);g.pointer(x,y,false,true);}
int main(){
 Mission g;check(g.state==Enter&&g.progress.mission1Complete&&g.progress.mission2Complete&&g.progress.hasSpaceStone&&g.progress.hasMindStone&&!g.progress.hasPowerStone&&!g.progress.mission3Complete,"initial progression and ENTER");
 enter(g);check(g.state==Intro,"ENTER begins intro");step(g,.6f);check(g.state==Walk,"intro transition");step(g,12);check(g.walkTravel==0&&g.state==Walk,"standing still does not skip required walk");Input right;right.right=true;step(g,9.8f,right);check(g.state==Walk,"walk lasts approximately 10 seconds");step(g,.25f,right);check(g.state==TriggerReady,"cages revealed after ten seconds of movement");step(g,1.1f,right);check(g.state==Guide&&g.triggerUsed&&g.current==0,"one-shot red-circle trigger -> first guide");
 float hp=g.player.health;step(g,90);check(g.player.health==hp&&g.remaining==60&&g.monsters[0].time==0,"guide freezes timer and AI");fight(g);check(g.state==Fight,"guide ENTER -> cage animation -> fight");
 Mission timeout;fixture(timeout);timeout.monsters[0].x=1200;step(timeout,60.1f);check(timeout.state==Failed&&!timeout.progress.defeated[0],"60-second timeout fails; survival is not victory");
 Mission attack;fixture(attack);attack.monsters[0].play(Recovery);attack.config.recoveryDuration=100;Input k;k.weapon=true;attack.update(1.0f/60,k);step(attack,.35f);check(attack.monsters[0].health==420,"weapon windup does no damage");step(attack,.15f);check(attack.monsters[0].health==398,"weapon active frame connects");step(attack,.25f);check(attack.monsters[0].health==398,"one melee swing registers only once");
 Mission kick;fixture(kick);kick.monsters[0].x=420;kick.monsters[0].play(Recovery);kick.config.recoveryDuration=100;Input j;j.kick=true;step(kick,.4f,j);check(kick.monsters[0].health==410,"kick collision and damage");
 Mission fire;fixture(fire);fire.monsters[0].x=800;fire.monsters[0].play(Recovery);fire.config.recoveryDuration=100;Input l;l.fire=true;fire.update(1.0f/60,l);step(fire,1.2f);check(fire.monsters[0].health==406,"animated Mind fire hits active target");int live=0;for(int i=0;i<4;++i)live+=fire.projectiles[i].active;check(live==0,"projectile removed after impact");check(fire.monsters[1].health==480&&fire.monsters[2].health==540,"inactive monsters cannot take projectile damage");
 Mission miss;fixture(miss);miss.monsters[0].x=3000;step(miss,5,l);live=0;for(int i=0;i<4;++i)live+=miss.projectiles[i].active;check(live<=4,"bounded projectile pool");step(miss,4);live=0;for(int i=0;i<4;++i)live+=miss.projectiles[i].active;check(live==0,"out-of-bounds projectiles expire");
 Mission guard;fixture(guard);guard.monsters[0].x=440;guard.monsters[0].play(AttackA);guard.monsters[0].time=.46f;guard.monsters[0].variant=0;Input sh;sh.guard=true;guard.update(1.0f/60,sh);check(std::fabs(guard.player.health-95.25f)<.001f,"guard reduces hit damage");guard.monsters[0].registered=false;guard.update(1.0f/60,sh);check(std::fabs(guard.player.health-95.25f)<.001f,"invulnerability prevents repeated contacts");
 Mission jump;fixture(jump);jump.monsters[0].x=1100;Input w;w.jump=true;jump.update(1.0f/60,w);step(jump,.45f);check(jump.jumpOffset()>100,"jump reaches apex");jump.monsters[0].x=440;jump.monsters[0].play(AttackA);jump.monsters[0].time=.46f;jump.update(1.0f/60,Input());check(jump.player.health==100,"jump clears low attack hurtbox");step(jump,.5f);bool dust=false;for(int i=0;i<8;++i)dust|=jump.effects[i].active&&jump.effects[i].kind==2;check(dust&&jump.player.jumpTime<0,"landing creates one-shot dust");step(jump,.6f);dust=false;for(int i=0;i<8;++i)dust|=jump.effects[i].active&&jump.effects[i].kind==2;check(!dust,"landing dust finishes");
 Mission death;fixture(death);death.player.health=1;death.monsters[0].x=440;death.monsters[0].play(AttackA);death.monsters[0].time=.46f;death.update(1.0f/60,Input());check(death.state==Failed&&death.player.health==0,"player zero health fails");
 Mission retry;start(retry);retry.current=1;retry.progress.defeated[0]=true;retry.monsters[0].health=0;retry.state=Failed;retry.monsters[1].health=5;retry.player.health=0;enter(retry);check(retry.state==Guide&&retry.current==1&&retry.progress.defeated[0]&&retry.monsters[0].health==0&&retry.player.health==100&&retry.monsters[1].health==480,"retry resets only current fight and preserves earlier victory");
 Mission flow;start(flow);
 for(int n=0;n<3;++n){check(flow.state==Guide&&flow.current==n,"ordered monster-specific guide");fight(flow);float inactiveTime[3];for(int i=0;i<3;++i)inactiveTime[i]=flow.monsters[i].time;
  int ticks=0;while(flow.state==Fight&&ticks<3601){Input bot;Fighter& m=flow.monsters[n];float gap=m.x-flow.player.x;
   // Ordinary input only: attack from range, guard telegraphs, punish recovery.
   if(m.action==Telegraph&&m.time>.35f)bot.guard=true;
   else if(m.action==AttackA||m.action==AttackB)bot.guard=true;
   else if(gap<290)bot.weapon=true;else bot.fire=true;
   if(gap>290&&m.action==Recovery)bot.right=true;
   flow.update(1.0f/60,bot);++ticks;
  }
  std::printf("Fight %d simulation: %.2fs, player HP %.2f, enemy HP %.2f, state %s\n",n+1,ticks/60.0f,flow.player.health,flow.monsters[n].health,flow.stateName());
  check(flow.state==Defeat&&flow.player.health>0&&ticks<3600,"input-driven fight won within one minute");
  for(int i=0;i<3;++i)if(i!=n)check(flow.monsters[i].time==inactiveTime[i],"inactive monster AI never ticks");
  check(!flow.progress.defeated[n],"victory waits for defeat animation");step(flow,1.3f);check(flow.progress.defeated[n],"defeat animation finishes once");
 }
 check(flow.state==Complete&&!flow.progress.mission3Complete&&!flow.progress.hasPowerStone,"complete screen precedes unclaimed reward");step(flow,3);check(flow.state==Reward,"Power Stone reward follows completion");enter(flow);check(flow.state==Reward&&!flow.progress.hasPowerStone,"keyboard cannot claim reward");
 flow.pointer(640,540,false,false);check(flow.buttonHover,"claim hover");flow.pointer(640,540,true,false);check(flow.buttonDown,"claim pressed");flow.pointer(0,0,false,true);check(!flow.progress.hasPowerStone,"release outside cancels claim");flow.pointer(0,0,true,false);flow.pointer(640,540,false,true);check(!flow.progress.hasPowerStone,"press outside cannot claim on release inside");click(flow);check(flow.state==Claimed&&flow.progress.hasPowerStone&&flow.progress.mission3Complete,"valid click claims exactly once");click(flow);step(flow,1);check(flow.state==Claimed,"claimed screen remains visible before ending");click(flow);step(flow,3);check(flow.state==FinalDialogue&&flow.progress.hasSpaceStone&&flow.progress.hasMindStone&&flow.progress.hasPowerStone&&flow.progress.defeated[0]&&flow.progress.defeated[1]&&flow.progress.defeated[2],"ending follows claimed hold and preserves all progression");

 check(flow.finalDialogue==0,"ending waits for fresh dialogue input");
 for(int i=0;i<3;++i){enter(flow);check(flow.state==FinalDialogue&&flow.finalDialogue==i+1,"one edge advances one line");step(flow,.1f);}
 enter(flow);check(flow.state==FinalSpace,"dialogue begins Space submission");
 const State ordered[]={FinalMind,FinalPower,FinalGlow,FinalBlackFade,FinalShipCount,FinalShipWalk};
 for(int i=0;i<6;++i){int n=0;State before=flow.state;while(flow.state==before&&n++<600)flow.update(1.0f/60,Input());check(flow.state==ordered[i],"ending timed states are ordered");}
 Input walk;walk.walkHeld=true;
 step(flow,2,walk);check(flow.shipProgress==0,"W held across ship entry cannot leak into movement");
 step(flow,10);check(flow.state==FinalShipWalk&&flow.shipProgress==0&&flow.shipWalkTime==0,"ship never auto-walks");
 step(flow,2,walk);float travel=flow.shipProgress,anim=flow.shipWalkTime;
 check(travel>0&&travel<1,"W moves ship path");step(flow,3);check(flow.shipProgress==travel&&flow.shipWalkTime==anim,"ship release immediately freezes movement and animation");
 step(flow,10,walk);check(flow.state==FinalDoorWait&&flow.shipProgress==1,"ship hard limit triggers existing door opening");
 anim=flow.shipWalkTime;step(flow,20,walk);check(flow.shipProgress==1&&flow.shipWalkTime==anim,"held W cannot pass open door");
 enter(flow);check(flow.state==FinalBeachWalk&&flow.beachProgress==0,"door Enter initializes beach once");
 step(flow,2,walk);check(flow.beachProgress==0,"W held across beach entry cannot leak");
 step(flow,10);check(flow.state==FinalBeachWalk&&flow.beachProgress==0&&flow.beachWalkTime==0&&flow.beachTime>11,"beach environment runs without automatic walking");
 step(flow,4,walk);travel=flow.beachProgress;anim=flow.beachWalkTime;float sea=flow.beachTime;
 step(flow,3);check(flow.beachProgress==travel&&flow.beachWalkTime==anim&&flow.beachTime>sea,"beach release freezes only character");
 step(flow,8,walk);check(flow.state==FinalReflection&&flow.beachProgress==1&&flow.reflectionLine==0,"shore clamp selects standing pose and first reflection");
 anim=flow.beachWalkTime;step(flow,40,walk);check(flow.beachProgress==1&&flow.beachWalkTime==anim&&flow.reflectionLine==0,"W cannot leave shore or advance reflections");
 for(int i=1;i<7;++i){enter(flow);check(flow.state==FinalReflection&&flow.reflectionLine==i,"fresh Enter advances exactly one reflection");step(flow,1);}
 step(flow,30);check(flow.state==FinalReflection&&flow.reflectionLine==6,"last reflection waits for Enter");
 enter(flow);check(flow.state==FinalBeachWatch,"last Enter starts countdown");
 step(flow,2.9f);check(flow.state==FinalBeachWatch,"countdown holds all three numbers");step(flow,.2f);check(flow.state==FinalBeachFade,"countdown precedes short fade");
 step(flow,.85f);check(flow.state==FinalJourney,"short fade immediately reaches manuscript");step(flow,60);enter(flow);click(flow);check(flow.state==FinalJourney,"journey remains terminal under input");
 Mission bounds;bounds.state=FinalReflection;bounds.beachProgress=1.5f;bounds.shipProgress=1.5f;bounds.update(1.0f/60,walk);check(bounds.beachProgress==1&&bounds.shipProgress==1,"clamps enforced even outside movement states");
 Mission rateA,rateB;rateA.state=rateB.state=FinalBeachWalk;rateA.update(.01f,Input());rateB.update(.01f,Input());
 for(int i=0;i<240;++i)rateA.update(1.0f/60,walk);for(int i=0;i<480;++i)rateB.update(1.0f/120,walk);
 check(std::fabs(rateA.beachProgress-rateB.beachProgress)<.0001f,"movement independent of update frequency");
 Mission invalid;invalid.state=Reward;click(invalid);check(!invalid.progress.hasPowerStone,"claim rejected without all three defeats");

 for(int n=0;n<3;++n){
  for(int variant=0;variant<2;++variant){Mission ai;fixture(ai,n);ai.player.x=350;ai.monsters[n].x=490;ai.monsters[n].attackCount=variant;ai.update(1.0f/60,Input());check(ai.monsters[n].action==Telegraph,"AI telegraphs both attack patterns");step(ai,.74f);check(ai.monsters[n].action==(variant==0?AttackA:AttackB),"each monster selects correct A/B attack");step(ai,.48f);float after=ai.player.health;check(after<100,"each monster attack damages in active frames");step(ai,.1f);check(ai.player.health==after,"enemy attack has one contact per swing");step(ai,.6f);check(ai.monsters[n].action==Recovery,"enemy attack finishes in recovery");}
 }
 Mission lock;fixture(lock);lock.monsters[0].x=1100;Input both;both.weapon=true;both.right=true;lock.update(1.0f/60,both);float root=lock.player.x;step(lock,.4f,both);check(lock.player.action==Weapon&&lock.player.x==root,"walking cannot interrupt weapon animation");lock.player.play(Hit);step(lock,.3f,both);check(lock.player.action==Hit&&lock.player.x==root,"walking cannot interrupt hit knockdown");
 Mission clamp;fixture(clamp);clamp.monsters[0].x=975;clamp.monsters[0].play(Recovery);clamp.config.recoveryDuration=100;Input left;left.left=true;step(clamp,4,left);check(clamp.player.x==Layout::PlayerMin,"left arena boundary");Input forward;forward.right=true;step(clamp,4,forward);check(clamp.player.x<=Layout::PlayerMax&&clamp.player.x<clamp.monsters[0].x,"right boundary preserves fighter facing order");
 Mission lethal;fixture(lethal);lethal.monsters[0].health=1;lethal.monsters[0].play(Recovery);lethal.config.recoveryDuration=100;step(lethal,.5f,k);check(lethal.state==Defeat&&lethal.monsters[0].health==0,"lethal melee clamps HP to zero");bool lethalSpark=false;for(int i=0;i<8;++i)lethalSpark|=lethal.effects[i].active&&lethal.effects[i].kind==0;check(lethalSpark,"lethal contact retains its one-shot hit effect");step(lethal,.3f,k);check(lethal.current==0&&lethal.monsters[0].action==Dead,"defeat cannot be interrupted by held attacks");
 Mission edge;fixture(edge);edge.remaining=.01f;edge.monsters[0].health=1;edge.player.action=Weapon;edge.player.time=.5f;edge.update(1.0f/60,Input());check(edge.state==Failed,"expired timer takes priority over a late lethal hit");
 std::printf("ALL %d CHECKS PASSED\n",checks);return 0;
}
