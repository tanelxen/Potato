//
//  Q3BspCollision.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>

class Quake3BSP;

//struct AABB
//{
//    glm::vec3 mins;
//    glm::vec3 maxs;
//};

struct HitResult
{
    glm::vec3 endpos;
    glm::vec3 normal;
    float fraction;
    
    bool startsolid;
    bool allsolid;
};

class Q3BspCollision
{
public:
    ~Q3BspCollision();
    
    void initFromBsp(Quake3BSP* bsp);
    
    void trace(HitResult& result, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &mins, const glm::vec3 &maxs) const;
//    void check(const glm::vec3 &start, const glm::vec3 &end, const AABB &aabb) const;
    
private:
    struct Impl;
    Impl* pImpl;
};
