#ifndef GOBLIN_H
#define GOBLIN_H

#include "scene/entity.h"
#include "entitynode.h"
#include "scene/cube.h"
#include "scene/terrain.h"

class Goblin : public Entity {
private:
    OpenGLContext *ctx;
    Terrain &terrain;
    // Stores the geometry of the entity
    uPtr<EntityNode> root;

    // Determines what action is being taken
    // 0 = Idle
    // 1 = Moving
    // 2 = Pathing
    int state;

    // Determine whether the entity should move
    bool moving;

    float jump_height;
    float jump_acceleration;

    // Stores last position's tick to tell if moving
    glm::vec3 last_position;

    // Builds the model for the goblin
    void buildGeometry();

    // Determine how the entity can move
    glm::vec3 moveEntity();

    // Updates the tranformations of the model with its positional data
    void updatePosition();

    // Returns the y coordinate of the highest block at that coordinate
    int getTopBlock(int x, int z);

    // Variables for tracking the blocks already checked
    int visited_blocks[31][31];
    // Tracks the path that needs to be taken
    std::vector<glm::ivec3> path;
    uint path_index;

public:
    Goblin(OpenGLContext* context, Terrain &t);
    Goblin(OpenGLContext* context, Terrain &t, glm::vec3 pos);
    Goblin(const Goblin &g);
    ~Goblin();

    EntityNode* getRoot();

    // Entity will try to path to stand on the block at (x, y, z)
    bool findPath(int x, int y, int z);

    // To be called by MyGL::tick()
    void tick(float dT, InputBundle &input) override;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;
};

#endif // GOBLIN_H
