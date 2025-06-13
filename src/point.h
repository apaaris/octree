#ifndef POINT_H
#define POINT_H

struct Point { 
    float x, y, z; 
    
    Point() : x(0), y(0), z(0) {}
    Point(float x, float y, float z) : x(x), y(y), z(z) {}
};

#endif // POINT_H 