#pragma once

// Forward declaration for Terrain class
class Player;

#include "entity.h"
#include "camera.h"
#include "terrain.h"


class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Terrain &mcr_terrain;
    bool inFlight;
    bool jumping;
    bool createBlock;
    bool destroyBlock;

   // bool triggerFlight;
    bool triggerCreate;
    bool triggerDestroy;

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, Terrain &terrain);

public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    Player(glm::vec3 pos, Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

//    static bool gridMarch(glm::vec3 rayOrigin, int axis, float length,
//                       const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    static bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDir,
                       const Terrain &ter, float *oDist, glm::ivec3 *oHitBlock, int *interfaceAxis);

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

    glm::vec3 getVelocity();
    void setVelocity(glm::vec3 v);

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;

};
