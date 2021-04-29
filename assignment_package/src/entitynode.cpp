#include "entitynode.h"

EntityNode::EntityNode(OpenGLContext* context, glm::vec3 c) :
    ctx(context),
    color(c), draw(true), square(ctx, c),
    t_x(0), t_y(0), t_z(0),
    r_x(0), r_y(0), r_z(0),
    s_x(1), s_y(1), s_z(1)
{
   square.create();
}

EntityNode::EntityNode(const EntityNode &n2) :
    ctx(n2.ctx), draw(n2.draw), square(n2.square),
    t_x(n2.t_x), t_y(n2.t_y), t_z(n2.t_z),
    r_x(n2.r_x), r_y(n2.r_y), r_z(n2.r_z),
    s_x(n2.s_x), s_y(n2.s_y), s_z(n2.s_z)
{
    square.create();
    for(const uPtr<EntityNode> &c : n2.children) {
        children.push_back(mkU<EntityNode>(*c.get()));
    }
}

glm::mat4 EntityNode::transformationMatrix() {
    glm::mat4 translation = glm::mat4(
                glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                glm::vec4(t_x, t_y, t_z, 1.0f));
    glm::mat4 rotation_x = glm::mat4(
                glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, cos(r_x), sin(r_x), 0.0f),
                glm::vec4(0.0f, -sin(r_x), cos(r_x), 0.0f),
                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 rotation_y = glm::mat4(
                glm::vec4(cos(r_y), 0.0f, -sin(r_y), 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(sin(r_y), 0.0f, cos(r_y), 0.0f),
                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 rotation_z = glm::mat4(
                glm::vec4(cos(r_z), -sin(r_z), 0.0f, 0.0f),
                glm::vec4(-sin(r_z), cos(r_z), 0.0f, 0.0f),
                glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 scale = glm::mat4(
                glm::vec4(s_x, 0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, s_y, 0.0f, 0.0f),
                glm::vec4(0.0f, 0.0f, s_z, 0.0f),
                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    return translation * rotation_x * rotation_y * rotation_z * scale;

}
EntityNode& EntityNode::addChild(uPtr<EntityNode> n2) {
    children.push_back(std::move(n2));
    return *children.back();
}
