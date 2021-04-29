#include "goblin.h"
#include <iostream>
#include <QDateTime>
#include <stdlib.h>
#include <queue>
#include <fstream>

Goblin::Goblin(OpenGLContext *context, Terrain &t)
    : Entity(), ctx(context), terrain(t), root(nullptr), state(0),
      moving(false), jump_height(0.0f), jump_acceleration(0.0f), path_index(0)
{
    buildGeometry();
    srand(QDateTime::currentMSecsSinceEpoch());
}

Goblin::Goblin(OpenGLContext *context, Terrain &t, glm::vec3 pos)
    : Entity(pos), ctx(context), terrain(t), root(nullptr), state(0),
      moving(false), jump_height(0.0f), jump_acceleration(0.0f), path_index(0)
{
    buildGeometry();
    srand(QDateTime::currentMSecsSinceEpoch());
}

Goblin::~Goblin() {}

Goblin::Goblin(const Goblin &g) :
    Entity(g), ctx(g.ctx), terrain(g.terrain), state(g.state),
    moving(g.moving), jump_height(g.jump_height), jump_acceleration(g.jump_acceleration), path_index(g.path_index)
{
    srand(QDateTime::currentMSecsSinceEpoch());
    root = mkU<EntityNode>(*g.root);
}

EntityNode* Goblin::getRoot() {
    return root.get();
}

void Goblin::buildGeometry() {
    // Use the cube drawable utilized by the base program
    // to draw the entity

    // Root node has no shape; moves full entity
    root = mkU<EntityNode>(EntityNode(ctx, glm::vec3()));
    root->draw = false;
    root->t_x = m_position.x;
    root->t_y = m_position.y;
    root->t_z = m_position.z;
    root->s_x = .3f;
    root->s_y = .3f;
    root->s_z = .3f;

    // First add the main body (torso + clothes)
    root->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* body = root->children.back().get();
    body->s_y = 1.5f;

    root->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(.6f,.3f,.15f))));
    EntityNode* jacket_left = root->children.back().get();
    jacket_left->t_x = 0.3f;
    jacket_left->t_y = 0.4f;
    jacket_left->s_x = 0.5f;
    jacket_left->s_y = 0.85f;
    jacket_left->s_z = 1.05f;

    root->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(.6f,.3f,.15f))));
    EntityNode* jacket_right = root->children.back().get();
    jacket_right->t_x = -0.3f;
    jacket_right->t_y = 0.4f;
    jacket_right->s_x = 0.5f;
    jacket_right->s_y = 0.85f;
    jacket_right->s_z = 1.05f;

    root->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(.6f,.3f,.15f))));
    EntityNode* pants = root->children.back().get();
    pants->t_x = 0.0f;
    pants->t_y = -0.8f;
    pants->t_z = 0.0f;
    pants->s_x = 1.1f;
    pants->s_y = 0.3f;
    pants->s_z = 1.1f;

    // DEBUG: block used to check effects
    root->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(1.0f,0.0f,0.0f))));
    EntityNode* debug = root->children.back().get();
    debug->draw = false;
    debug->t_y = 2.0f;

    // Add arms / legs
    body->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* l_arm_r = body->children.back().get();
    l_arm_r->draw = false;
    l_arm_r->t_x = 0.6f;
    l_arm_r->t_y = .2f;

    l_arm_r->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* l_arm = l_arm_r->children.back().get();
    l_arm->t_y = -0.2f;
    l_arm->s_x = .3f;
    l_arm->s_y = .7;
    l_arm->s_z = .2f;

    body->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* r_arm_r = body->children.back().get();
    r_arm_r->draw = false;
    r_arm_r->t_x = -0.6f;
    r_arm_r->t_y = .2f;

    r_arm_r->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* r_arm = r_arm_r->children.back().get();
    r_arm->t_y = -0.2f;
    r_arm->s_x = .3f;
    r_arm->s_y = .7;
    r_arm->s_z = .2f;

    body->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* l_leg_r = body->children.back().get();
    l_leg_r->draw = false;
    l_leg_r->t_x = 0.3f;
    l_leg_r->t_y = -.6f;

    l_leg_r->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* l_leg = l_leg_r->children.back().get();
    l_leg->t_y = -0.2f;
    l_leg->s_x = .3f;
    l_leg->s_y = .7;
    l_leg->s_z = .2f;

    body->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* r_leg_r = body->children.back().get();
    r_leg_r->draw = false;
    r_leg_r->t_x = -0.3f;
    r_leg_r->t_y = -.6f;

    r_leg_r->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* r_leg = r_leg_r->children.back().get();
    r_leg->t_y = -0.2f;
    r_leg->s_x = .3f;
    r_leg->s_y = .7;
    r_leg->s_z = .2f;

    // Add the neck / head
    body->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* neck = body->children.back().get();
    neck->t_y = 0.5f;
    neck->s_x = .2f;
    neck->s_y = .5f;
    neck->s_z = .2f;

    neck->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* head = neck->children.back().get();
    head->t_y = 0.8f;
    head->s_x = 4.0f;
    head->s_y = 1.0f;
    head->s_z = 4.0f;

    head->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* l_ear = head->children.back().get();
    l_ear->t_x = 0.55f;
    l_ear->t_y = 0.2f;
    l_ear->t_z = -0.2f;
    l_ear->r_x = -1.0f;
    l_ear->r_z = -.2f;
    l_ear->s_x = .2f;
    l_ear->s_y = 1.0f;
    l_ear->s_z = .3f;

    head->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* r_ear = head->children.back().get();
    r_ear->t_x = -0.55f;
    r_ear->t_y = 0.2f;
    r_ear->t_z = -0.2f;
    r_ear->r_x = -1.0f;
    r_ear->r_z = .2f;
    r_ear->s_x = .2f;
    r_ear->s_y = 1.0f;
    r_ear->s_z = .3f;

    head->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 0.0f, 0.0f))));
    EntityNode* l_eye = head->children.back().get();
    l_eye->t_x = .3f;
    l_eye->t_y = .2f;
    l_eye->t_z = .5f;
    l_eye->s_x = .1f;
    l_eye->s_y = .1f;
    l_eye->s_z = .1f;

    head->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 0.0f, 0.0f))));
    EntityNode* r_eye = head->children.back().get();
    r_eye->t_x = -.3f;
    r_eye->t_y = .2f;
    r_eye->t_z = .5f;
    r_eye->s_x = .1f;
    r_eye->s_y = .1f;
    r_eye->s_z = .1f;

    head->addChild(mkU<EntityNode>(EntityNode(ctx, glm::vec3(0.0f, 1.0f, 0.0f))));
    EntityNode* nose = head->children.back().get();
    nose->t_z = .5;
    nose->r_x = -1.0f;
    nose->s_x = .2f;
    nose->s_y = .5f;
    nose->s_z = .2f;

    // Make sure goblin starts "standing" on a block;
    // Check from the top for the first non-empty block
    int entity_x = int(m_position.x);
    int entity_z = int(m_position.z);
    for(int i = 255; i >= 0; --i) {
        if (terrain.getBlockAt(entity_x, i, entity_z) != EMPTY) {
            m_position.y = i + 1.5f;
            root->t_y = i + 1.5f;
            break;
        }
    }
}

void Goblin::tick(float dT, InputBundle &input) {
    // Check to see how the entity can move and move it
    moveEntity();

    // Update the model to represent the position
    updatePosition();


    // Utilize path
    if (state == 2) {
        moving = true;
        if (path_index < path.size()) {
            // Figure out which direction to go
            int new_x = path[path_index].x;
            int new_z = path[path_index].z;

            // Reached the center of the new block; move to the next one
            if (new_x + 0.49f <= m_position.x && new_x + 0.51f >= m_position.x
                    && new_z + 0.49f <= m_position.z && new_z + 0.51f >= m_position.z) {
                ++path_index;
            }
            // Change rotation based on direction
            else if (new_x + 0.49f > m_position.x) {
                root->r_y = 3.14f / 2.0f;
            }
            else if (new_z + 0.49f > m_position.z) {
                root->r_y = 0.0f;
            }
            else if (new_x + 0.51f < m_position.x) {
                root->r_y = -3.14f / 2.0f;
            }
            else if (new_z + 0.51f < m_position.z) {
                root->r_y = 3.14f;
            }
        }
        else {
            path.clear();
            path_index = 0;
            state = 0;
        }
    }
    else {
        // Potentially pick a random direction to move / decide whether or not to move
        // Should move?
        if (rand() % 200 == 1) {
            moving = !moving;
        }

        // Should change direction?
        if (rand() % 200 == 0) {
            root->r_y = (QDateTime::currentMSecsSinceEpoch() % 628) / 100.0f;
        }

        // Update moving state
        if (m_position != last_position) {
            state = 1; // Idle
        }
        else{
            state = 0; // Moving
        }
        last_position = m_position;
    }


    // Idle
    if (state == 0) {
        // Reset positions
        root->children[0]->children[0]->r_x = 0;
        root->children[0]->children[1]->r_x = 0;
        root->children[0]->children[2]->r_x = 0;
        root->children[0]->children[3]->r_x = 0;
        root->children[0]->children[2]->r_y = 0;
    }
    // Moving
    else if (state == 1 || state == 2) {
        // Function to turn time into usable number
        // 628 ~= 360 degrees (in radians)
        long time = QDateTime::currentMSecsSinceEpoch() % 628;
        float movement = cos(time / 100.0f);
        if (movement > 2.f) {
            movement = -movement - 2.28;
        }
        // Move arms
        root->children[0]->children[0]->r_x = movement;
        root->children[0]->children[1]->r_x = -movement;

        // Move legs
        root->children[0]->children[2]->r_x = movement;
        root->children[0]->children[3]->r_x = -movement;

        // Move head
        root->children[0]->children[4]->r_y = movement;
    }
}

glm::vec3 Goblin::moveEntity() {
    // Check forward and down rays for collisions
    float dist = -1;
    glm::ivec3 hit;
    glm::vec3 ray_forward(glm::vec3(glm::rotate(glm::mat4(), root->r_y, glm::vec3(0.f, 1.f, 0.f))
                               * glm::vec4(0.0f, 0.0f, .2f, 1.f)));
    glm::vec3 ray_down(0.0f, -0.5f, 0.0f);
    ray_forward.y = 0.0f;
    int axis = -1;
    bool blocked_forward = Player::gridMarch(m_position, ray_forward, terrain, &dist, &hit, &axis);
    bool blocked_forward_high = Player::gridMarch(m_position + glm::vec3(0.0f, 1.0f, 0.0f), ray_forward, terrain, &dist, &hit, &axis);
    bool blocked_down = Player::gridMarch(m_position + glm::vec3(0.0f, 1.0f, 0.0f), ray_down, terrain, &dist, &hit, &axis);

    if (jump_height == 1.0f) {
        if (jump_acceleration < -0.1f) {
            jump_height = 0.0f;
        }

        m_position.y += jump_acceleration;
        if(jump_acceleration > 0.0f) {
            moveAlongVector(0.1f * jump_acceleration * ray_forward);
        }
        jump_acceleration -= 0.05f;
    }

    // Entity can still fall while idle, but cannot move horizontally
    if (moving) {
        // Can't move forward; try jumping (going up)
        if (blocked_forward) {
            if (!blocked_forward_high) {
                // Start jumping
                jump_acceleration = 0.4f;
                jump_height = 1.0f;
            }
            // Workaround for pathing
            else if (state == 2 && blocked_forward_high) {
                moveAlongVector(.1f * ray_forward);
            }
            else if (state != 2){
                // Turn around
                root->r_y += 3.14;
            }
        }
        // Can move forward; do that
        else {
            moveAlongVector(.1f * ray_forward);
        }
    }

    // Floating in the air; go down
    if (!blocked_down && !blocked_forward) {
        m_position.y -= .1f;
    }

    return glm::vec3();
}
void Goblin::updatePosition() {
    root->t_x = m_position.x;
    root->t_y = m_position.y;
    root->t_z = m_position.z;
}

int Goblin::getTopBlock(int x, int z) {
    for(int i = 255; i >= 0; --i) {
        if (terrain.getBlockAt(x, i, z) != EMPTY) {
            return i;
        }
    }
    return -1;
}

bool Goblin::findPath(int x, int y, int z) {
    // Assert that the NPC is in range of the goal ( < 10 blocks x and z)
    if (abs(x - m_position.x) > 15.0f || abs(z - m_position.z) > 15.0f) {
        return false;
    }
    path.clear();
    state = 2;
    // Init the nearby blocks (10 block radius around the start)
    for(int i = 0; i < 31; ++i) {
        for(int j = 0; j < 31; ++j) {
            visited_blocks[i][j] = -1;
        }
    }
    std::queue<glm::ivec3> queue;
    // Init starting space to entity's coordinates
    glm::ivec3 start_pos = glm::ivec3(int(m_position.x), int(m_position.y), int(m_position.z));
    queue.push(start_pos);
    int center_x = 15;
    int center_z = 15;
    visited_blocks[center_x][center_z] = 1;
    while(!queue.empty()) {
        glm::ivec3 node = queue.front();
        queue.pop();

        if (node.x == x && node.z == z) {
            int new_x = x;
            int new_z = z;
            int index_x = (new_x - start_pos.x) + center_x;
            int index_z = (new_z - start_pos.z) + center_z;
            int min = visited_blocks[index_x][index_z];
            int min_x = 0;
            int min_z = 0;
            // Generate the path from the destination
            while(new_x != start_pos.x || new_z != start_pos.z) {
                for(int i = -1; i <= 1; ++i) {
                    for(int j = -1; j <= 1; ++j) {
                        // Skip when one is not 0 (should change)
                        if ((i != 0 && j != 0) || (i == 0 && j == 0)) {
                            continue;
                        }
                        if (index_x + i > 30 || index_x + i < 0 || index_z + j > 30 || index_z + j < 0) {
                            continue;
                        }
                        // Only add the minimum element to the path
                        if (visited_blocks[index_x + i][index_z + j] > 0 && visited_blocks[index_x + i][index_z + j] < min) {
                            // y is just 0 because it's not used
                            min_x = new_x + i;
                            min_z = new_z + j;
                            min = visited_blocks[index_x + i][index_z + j];
                        }
                    }
                }
                new_x = min_x;
                new_z = min_z;
                index_x = (new_x - start_pos.x) + center_x;
                index_z = (new_z - start_pos.z) + center_z;
                path.push_back(glm::ivec3(min_x, 0, min_z));
            }

            // Build the final path
            std::reverse(path.begin(), path.end());

            // Move to center of block
            return true;
        }

        for(int i = -1; i <= 1; ++i) {
            for(int j = -1; j <= 1; ++j) {
                // Skip when one is not 0 (should change)
                if ((i != 0 && j != 0) || (i == 0 && j == 0)) {
                    continue;
                }
                // Unencountered
                int new_x = node.x + i;
                int new_z = node.z + j;
                int index_x = (new_x - start_pos.x) + center_x;
                int index_z = (new_z - start_pos.z) + center_z;
                if (index_x > 30 || index_x < 0 || index_z > 30 || index_x < 0) {
                    continue;
                }
                if (visited_blocks[index_x][index_z] == -1) {
                    // If wall
                    int new_y = getTopBlock(new_x, new_z);
                    if (new_y > node.y + 1) {
                        visited_blocks[index_x][index_z] = 0;
                    }
                    else {
                        visited_blocks[index_x][index_z] = visited_blocks[(node.x - start_pos.x) + center_x][(node.z - start_pos.z) + center_z] + 1;
                        queue.push(glm::ivec3(new_x, new_y, new_z));
                    }
                }
            }
        }
    }
    return false;
}

// Player overrides all of Entity's movement
// functions so that it transforms its camera
// by the same amount as it transforms itself.
void Goblin::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
}
void Goblin::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
}
void Goblin::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
}
void Goblin::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
}
void Goblin::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
}
void Goblin::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
}
void Goblin::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
}
void Goblin::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
}
void Goblin::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
}
void Goblin::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
}
void Goblin::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
}
void Goblin::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
}
void Goblin::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
}
