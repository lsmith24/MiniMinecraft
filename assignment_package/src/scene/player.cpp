#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera), inFlight(false), jumping(false), createBlock(false),
      destroyBlock(false),
      triggerCreate(false), triggerDestroy(false)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs) {

    //Trigger Flight
    if (inputs.fPressed){
        inFlight = !inFlight;
        m_velocity = glm::vec3();
    }

    //Z Axis
    bool zActive = false;
    if(inputs.wPressed){
        //Forward
        m_acceleration.z = inFlight ? 25.f : 20.f;
    } else if (inputs.sPressed){
        //Backwards
        m_acceleration.z = inFlight ? -25.f : -20.f;
    } else if (!inputs.wPressed && !inputs.sPressed) {
        //Both False
        m_acceleration.z = 0.f;
        m_velocity.z = 0.f;
    } else {
        //None
        m_acceleration.z = 0.f;
        m_velocity.z = 0.f;
    }

    //X Axis
    if(inputs.dPressed){
        //Right
        m_acceleration.x = inFlight ? -25.f : -20.f;
    } else if (inputs.aPressed) {
        //Left
        m_acceleration.x = inFlight ? 25.f : 20.f;
    } else if (!inputs.dPressed && !inputs.aPressed){
        //Both False
        m_acceleration.x = 0.f;
        m_velocity.x = 0.f;
    } else {
        //None
        m_acceleration.x = 0.f;
        m_velocity.x = 0.f;
    }

    //YAxis
    if(inFlight){
        //In-Flight
        if(inputs.qPressed){
            //Up
            m_acceleration.y = 25.f;
        }else if(inputs.ePressed){
            //Down
            m_acceleration.y = -25.f;
        }else if (!inputs.qPressed && !inputs.ePressed){
            //Both False
            m_acceleration.y = 0.f;
            m_velocity.y = 0.f;
        } else {
            //None
            m_acceleration.y = 0.f;
            m_velocity.y = 0.f;
        }

    } else {
        //Non-Flight
        if(inputs.spacePressed){
            jumping = true;
        }
    }

    //Blocks
    if (inputs.rightClick) {
        triggerCreate = true;
    }
    if (inputs.leftClick) {
        triggerDestroy = true;
    }
    if (!inputs.rightClick && triggerCreate) {
        triggerCreate = false;
        createBlock = true;
    }
    if (!inputs.leftClick && triggerDestroy) {
        triggerDestroy = false;
        destroyBlock = true;
    }

    //Mouse
    m_camera.setThetaPhi(inputs.mouseX, inputs.mouseY);
}

void Player::computePhysics(float dT, Terrain &terrain) {

    //Block
    //Destory
    if(destroyBlock){
        float oDist = 0.f;
        glm::ivec3 blockHit = glm::ivec3();
        int inter = -1;
        if(gridMarch(m_position + glm::vec3(0.f, 1.5f, 0.f), m_camera.getForward() * 3.f,
                     terrain, &oDist, &blockHit, &inter)){
            terrain.setBlockAt(blockHit.x, blockHit.y, blockHit.z, EMPTY);
        }
        destroyBlock = false;
    }
    //Create
    if(createBlock){
        float oDist = 0.f;
        glm::ivec3 blockHit = glm::ivec3();
        int inter = -1;
        if(gridMarch(m_position + glm::vec3(0.f, 1.5f, 0.f), m_camera.getForward() * 3.f,
                     terrain, &oDist, &blockHit, &inter)){
            BlockType ty = terrain.getBlockAt(blockHit[0], blockHit[1], blockHit[2]);
            if(blockHit[inter] < m_position[inter]){
                blockHit[inter]++;
                terrain.setBlockAt(blockHit.x, blockHit.y, blockHit.z, ty);
            } else {
                blockHit[inter]--;
                terrain.setBlockAt(blockHit.x, blockHit.y, blockHit.z, ty);
            }
        }
        createBlock = false;
    }

    if(inFlight){
        m_velocity *= 0.95f;
        m_velocity += m_acceleration * dT * 0.001f;
        glm::vec3 movevec = glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_camera.theta), glm::vec3(0.f, 1.f, 0.f))
                                      * glm::rotate(glm::mat4(), glm::radians(m_camera.phi), glm::vec3(1.f, 0.f, 0.f))
                                      * glm::vec4((m_velocity * dT * 0.001f), 1.f));
        this->moveAlongVector(movevec);
    } else {
        glm::vec3 curPos = glm::floor(m_position);
        if(terrain.getBlockAt(curPos.x, curPos.y - 1, curPos.z) == EMPTY ||
                m_position.y - curPos.y >= 0.01f){
            m_velocity *= 0.95f;
            m_acceleration.y = -50.f;
        } else {
            m_velocity *= 0.9f;
        }

        m_velocity += m_acceleration * dT * 0.001f;
        for (int i = 0; i < 3; i++) {
            if (glm::abs(m_velocity[i]) <= 0.01f) {
                m_velocity[i] = 0.f;
            }
        }

        if(jumping && terrain.getBlockAt(curPos.x, curPos.y - 1, curPos.z) != EMPTY
                && m_position.y - curPos.y < 0.01f){
            m_velocity.y += 13.5f;
        }
        jumping = false;
        glm::vec3 movevec = glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_camera.theta), glm::vec3(0.f, 1.f, 0.f))
                                      * glm::vec4((m_velocity * dT * 0.001f), 1.f));

        glm::vec3 mVec = movevec;
        float oDist = 0.f;
        glm::ivec3 blockHit = glm::ivec3();
        int inter = -1;
        for(float i = 0.f; i <= 2.f; i++){
            for(int k = 0.f; k <= 2; k++){
                glm::vec3 raydir = glm::vec3();
                //#1
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(0.5f, i, 0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#2
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(0.5f, i, -0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#3
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(-0.5f, i, 0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#4
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(-0.5f, i, -0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#5
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(0.5f, i, 0.f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#6
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(0.f, i, 0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#7
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(-0.5f, i, 0.f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
                //#8
                raydir[k] = mVec[k];
                if(gridMarch(m_position + glm::vec3(0.f, i, -0.5f), raydir, terrain, &oDist, &blockHit, &inter)){
                    if (oDist > 0.001f) {
                        mVec[k] = glm::sign(mVec[k]) * (glm::min(glm::abs(mVec[k]), oDist) - 0.0001f);
                    } else {
                        mVec[k] = 0.f;
                    }
                }
            }
        }
        movevec = mVec;
        this->moveAlongVector(movevec);
    }
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

//bool Player::gridMarch(glm::vec3 rayOrigin, int axis, float length,
//                       const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit){

//}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDir, const Terrain &ter,
                       float *oDist, glm::ivec3 *oHitBlock, int *interfaceAxis){
    float maxL = glm::length(rayDir);
    glm::ivec3 curCell = glm::ivec3(glm::floor(rayOrigin));
    rayDir = glm::normalize(rayDir);

    float curT = 0.f;
    while(curT < maxL){
        float minT = glm::sqrt(3.f);
        *interfaceAxis = -1;
        for(int i = 0; i < 3; ++i){
            if(rayDir[i] != 0){
                float offset = glm::max(0.f, glm::sign(rayDir[i]));
                if(curCell[i] == rayOrigin[i] && offset == 0.f){
                    offset = -1.f;
                }
                int nextInter = curCell[i] + offset;
                float axisT = (nextInter - rayOrigin[i]) / rayDir[i];
                axisT = glm::min(axisT, maxL);
                if(axisT < minT){
                    minT = axisT;
                    *interfaceAxis = i;
                }
            }
        }
        if(*interfaceAxis == -1){
            throw std::out_of_range("error in gridMarch");
        }
        curT += minT;
        rayOrigin += rayDir * minT;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        offset[*interfaceAxis] = glm::min(0.f, glm::sign(rayDir[*interfaceAxis]));
        curCell = glm::ivec3(glm::floor(rayOrigin)) + offset;

        if(ter.hasChunkAt(curCell.x, curCell.z)){
            BlockType cellType = ter.getBlockAt(curCell.x, curCell.y, curCell.z);
            if(cellType != EMPTY){
                *oHitBlock = curCell;
                *oDist = glm::min(maxL, curT);
                return true;
            }
        }
    }
    *oDist = glm::min(maxL, curT);
    return false;
}


void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

glm::vec3 Player::getVelocity() {
    return m_velocity;
}
void Player::setVelocity(glm::vec3 v) {
    m_velocity = v;
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
