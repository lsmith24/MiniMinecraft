#pragma once
#ifndef ENTITYNODE_H
#define ENTITYNODE_H

#include "drawable.h"
#include "smartpointerhelp.h"
#include "scene/cube.h"

class EntityNode
{
private:

public:
    EntityNode(OpenGLContext* context, glm::vec3 c);
    EntityNode(const EntityNode &n2);

    OpenGLContext* ctx;
    glm::vec3 color;
    bool draw; // Determines whether the node should be drawn

    // For time constraints, make member variables public
    // to avoid implementing required setter / getter structure
    std::vector< uPtr<EntityNode> > children;
    Cube square;

    // Variables for transformations
    // T for translation, R for rotation, S for scale
    float t_x, t_y, t_z;
    float r_x, r_y, r_z;
    float s_x, s_y, s_z;

    glm::mat4 transformationMatrix();
    EntityNode& addChild(uPtr<EntityNode> n2);
};

#endif // ENTITYNODE_H
