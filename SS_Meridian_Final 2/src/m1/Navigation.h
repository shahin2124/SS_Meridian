#pragma once
#include <vector>
namespace Meridian {
struct Point { float x,y; Point(float X=0,float Y=0):x(X),y(Y){} };
struct Polygon { std::vector<Point> points; bool contains(Point p) const; };
struct Cover { Polygon footprint, silhouette; Point hide; float depth; };
struct Navigation {
 Polygon floor; std::vector<Cover> covers; std::vector<Point> nodes,patrol;
 bool walkable(Point p,float radius=12) const;
 bool clearSight(Point from,Point to) const;
 bool clearPath(Point from,Point to,float radius=12) const;
 Point nextWaypoint(Point from,Point target) const;
 Point slide(Point from,Point displacement,float radius=12) const;
};
const Navigation& arenaNavigation();
const Navigation& roomNavigation(int room);
float distance(Point a,Point b);
const Point GUIDE_FOOT(950,790);
}
