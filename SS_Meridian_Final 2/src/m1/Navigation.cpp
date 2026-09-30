#include "Navigation.h"
#include <cmath>
#include <algorithm>
#include <limits>
namespace Meridian {
float distance(Point a,Point b){float x=a.x-b.x,y=a.y-b.y;return std::sqrt(x*x+y*y);}
static Polygon polygon(const Point* p,int n){Polygon v;v.points.assign(p,p+n);return v;}
static Polygon rectangle(float l,float t,float r,float b){Point p[]={Point(l,t),Point(r,t),Point(r,b),Point(l,b)};return polygon(p,4);}
bool Polygon::contains(Point p)const{
 bool inside=false;for(size_t i=0,j=points.size()-1;i<points.size();j=i++){
  Point a=points[i],b=points[j];if((a.y>p.y)!=(b.y>p.y)&&p.x<(b.x-a.x)*(p.y-a.y)/(b.y-a.y)+a.x)inside=!inside;
 }return inside;
}
bool Navigation::walkable(Point p,float radius)const{
 const Point offsets[]={Point(0,0),Point(radius,0),Point(-radius,0),Point(0,radius*.45f),Point(0,-radius*.45f)};
 for(int k=0;k<5;++k){Point q(p.x+offsets[k].x,p.y+offsets[k].y);if(!floor.contains(q))return false;for(size_t i=0;i<covers.size();++i)if(covers[i].footprint.contains(q))return false;}return true;
}
bool Navigation::clearSight(Point a,Point b)const{
 int count=std::max(1,int(distance(a,b)/5));for(int n=0;n<=count;++n){float t=float(n)/count;Point q(a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t);if(!floor.contains(q))return false;for(size_t j=0;j<covers.size();++j)if(covers[j].footprint.contains(q))return false;}return true;
}
bool Navigation::clearPath(Point a,Point b,float radius)const{
 int count=std::max(1,int(distance(a,b)/5));for(int n=0;n<=count;++n){float t=float(n)/count;if(!walkable(Point(a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t),radius))return false;}return true;
}
Point Navigation::slide(Point p,Point delta,float radius)const{
 int steps=std::max(1,int(std::max(std::fabs(delta.x),std::fabs(delta.y))/3)+1);delta.x/=steps;delta.y/=steps;
 for(int n=0;n<steps;++n){Point full(p.x+delta.x,p.y+delta.y);if(walkable(full,radius)){p=full;continue;}Point a(p.x+delta.x,p.y);if(walkable(a,radius))p=a;Point b(p.x,p.y+delta.y);if(walkable(b,radius))p=b;}return p;
}
Point Navigation::nextWaypoint(Point from,Point target)const{
 if(clearPath(from,target,14))return target;
 std::vector<Point> points;points.push_back(from);points.push_back(target);points.insert(points.end(),nodes.begin(),nodes.end());
 const int n=int(points.size());std::vector<float> cost(n,1e20f);std::vector<int> parent(n,-1);std::vector<bool> done(n,false);cost[0]=0;
 for(int k=0;k<n;++k){int u=-1;for(int j=0;j<n;++j)if(!done[j]&&(u<0||cost[j]<cost[u]))u=j;if(u<0||cost[u]>=1e20f)break;if(u==1)break;done[u]=true;
  for(int v=0;v<n;++v)if(!done[v]&&clearPath(points[u],points[v],14)){float c=cost[u]+distance(points[u],points[v]);if(c<cost[v]){cost[v]=c;parent[v]=u;}}
 }
 if(parent[1]<0)return from;
 int next=1;while(parent[next]>0)next=parent[next];return points[next];
}
static void addCover(Navigation& n,float l,float t,float r,float b,Point hide,const Point* outline,int count){Cover c;c.footprint=rectangle(l,t,r,b);c.silhouette=polygon(outline,count);c.hide=hide;c.depth=b;n.covers.push_back(c);}
const Navigation& arenaNavigation(){
 static Navigation nav;if(nav.floor.points.empty()){
  // Only the central, level aisle. The upper edge stays in front of the throne stairs.
  Point p[]={Point(365,920),Point(595,803),Point(745,711),Point(925,711),Point(1075,803),Point(1307,920)};nav.floor=polygon(p,6);
 }return nav;
}
const Navigation& roomNavigation(int room){
 static Navigation rooms[3];static bool ready=false;if(!ready){ready=true;
  // Coordinates were traced from the supplied 1672x941 room artwork.
  Navigation& a=rooms[0];Point af[]={Point(295,350),Point(690,310),Point(945,400),Point(1320,350),Point(1485,440),Point(1445,850),Point(485,885),Point(410,790),Point(480,700),Point(380,490)};a.floor=polygon(af,10);
  Point ap[]={Point(716,0),Point(868,0),Point(868,514),Point(788,550),Point(714,520)};addCover(a,704,445,873,550,Point(902,504),ap,5);
  Point ac[]={Point(0,376),Point(192,378),Point(310,451),Point(365,561),Point(457,592),Point(491,679),Point(278,788),Point(0,799)};addCover(a,0,480,478,782,Point(505,681),ac,8);
  const Point an[]={Point(500,820),Point(600,700),Point(620,410),Point(930,590),Point(940,430),Point(1200,480),Point(1340,760),Point(1050,800),Point(902,504)};a.nodes.assign(an,an+9);Point at[]={Point(1240,475),Point(1130,780),Point(600,750),Point(520,410)};a.patrol.assign(at,at+4);
  Navigation& b=rooms[1];Point bf[]={Point(300,365),Point(460,365),Point(640,620),Point(1065,675),Point(1080,445),Point(1440,445),Point(1490,825),Point(1360,900),Point(140,900)};b.floor=polygon(bf,9);
  Point bp[]={Point(790,0),Point(1027,0),Point(1033,625),Point(860,649),Point(795,597)};addCover(b,795,480,1040,659,Point(1100,633),bp,5);
  Point bc[]={Point(638,389),Point(749,384),Point(800,432),Point(808,623),Point(675,580),Point(640,505)};addCover(b,620,420,805,625,Point(587,600),bc,6);
  const Point bn[]={Point(365,410),Point(440,560),Point(500,765),Point(850,760),Point(1130,760),Point(1280,550),Point(1400,770),Point(1070,680),Point(1100,633)};b.nodes.assign(bn,bn+9);Point bt[]={Point(1290,485),Point(1310,765),Point(500,775),Point(380,425)};b.patrol.assign(bt,bt+4);
  Navigation& c=rooms[2];Point cf[]={Point(340,465),Point(630,430),Point(900,485),Point(930,595),Point(1450,615),Point(1450,850),Point(350,900),Point(275,755)};c.floor=polygon(cf,8);
  Point cp[]={Point(670,0),Point(807,0),Point(813,536),Point(734,557),Point(668,532)};addCover(c,659,459,825,558,Point(854,518),cp,5);
  const Point cn[]={Point(375,800),Point(520,620),Point(590,475),Point(855,600),Point(854,518),Point(1080,690),Point(1380,790),Point(1020,810)};c.nodes.assign(cn,cn+8);Point ct[]={Point(565,478),Point(500,700),Point(1110,750),Point(860,620)};c.patrol.assign(ct,ct+4);
 }return rooms[std::max(0,std::min(2,room))];
}
}
