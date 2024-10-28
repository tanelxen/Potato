//
//  CubeGeometry.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.10.24.
//

#pragma once

#include <glm/glm.hpp>

class Camera;

namespace CubeGeometry {

void init();
void deinit();
void draw(const Camera& camera, const glm::vec3& position, const glm::vec3& scale);

}
