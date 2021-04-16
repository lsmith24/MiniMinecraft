#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
      x_offset(0), z_offset(0), generating(false), generated(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

// Check each "face" of the block at <x,y,z> and return a 6 length
// array that indicates which faces should be drawn.
// Order of vector: +x, -x, +y, -y, +z, -z
std::array<bool, 6> Chunk::checkBlockFaces(int x, int y, int z) {
    // Init the output to all true
    std::array<bool, 6> output;
    for(int i = 0; i < 6; ++i) {
        output[i] = true;
    }

    // Perform bounds checking on all indices
    // even though at() is used
    if (x < 15 && getBlockAt(x + 1, y, z) != EMPTY) {
        output[0] = false;
    }
    if (x > 0 && getBlockAt(x - 1, y, z) != EMPTY) {
        output[1] = false;
    }
    if (y < 255 && getBlockAt(x, y + 1, z) != EMPTY) {
        output[2] = false;
    }
    if (y > 0 && getBlockAt(x, y - 1, z) != EMPTY) {
        output[3] = false;
    }
    if (z < 15 && getBlockAt(x, y, z + 1) != EMPTY) {
        output[4] = false;
    }
    if (z > 0 && getBlockAt(x, y, z - 1) != EMPTY) {
        output[5] = false;
    }

    return output;
}

std::vector<glm::vec4> Chunk::createFaces(std::array<bool, 6> faces, int x, int y, int z){
    // Get face color based on block type.
    // All faces will use this color
    BlockType block = getBlockAt(x, y, z);
    glm::vec4 color;
    glm::vec4 uvTop;
    glm::vec4 uvBot;
    switch(block) {
        case GRASS:
            color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
            uvTop  = glm::vec4(3.f, 16.f, 0.f, 0.f);
            uvBot = glm::vec4(2.f, 16.f, 0.f, 0.f);
            break;
        case DIRT:
            color = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
            uvTop  = glm::vec4(3.f, 16.f, 0.f, 0.f);
            uvBot = glm::vec4(2.f, 16.f, 0.f, 0.f);
            break;
        case STONE:
            color = glm::vec4(0.5f);
            uvTop  = glm::vec4(3.f, 16.f, 0.f, 0.f);
            uvBot = glm::vec4(2.f, 16.f, 0.f, 0.f);
            break;
        case SNOW:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            uvTop  = glm::vec4(3.f, 16.f, 0.f, 0.f);
            uvBot = glm::vec4(2.f, 16.f, 0.f, 0.f);
        default:
            // Other block types are not yet handled, so we default to black
            color = glm::vec4(0.f);
            break;
    }

    // Create a translation matrix based on the x, y, z components
    glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(x, y, z));

    // Copied coordinates from cube initialization.
    // Could be done in less lines
    std::vector<glm::vec4> face_vbo;

    // Right face
    if (faces[0]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        //face_vbo.push_back(uvTop);

        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        //UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        //face_vbo.push_back(uvTop);
    }
    // Left face
    if (faces[1]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        //face_vbo.push_back(uvTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        //face_vbo.push_back(uvTop);
    }
    // Top face
    if (faces[2]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        //face_vbo.push_back(uvTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        //face_vbo.push_back(uvTop);
    }
    // Bottom face
    if (faces[3]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        //face_vbo.push_back(uvTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        //face_vbo.push_back(uvBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        //face_vbo.push_back(uvTop);
    }
    // Front face
    if (faces[4]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        //face_vbo.push_back(uvTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        //face_vbo.push_back(uvBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        //face_vbo.push_back(uvTop);
    }
    // Back face
    if (faces[5]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        //face_vbo.push_back(uvTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        //face_vbo.push_back(uvBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        //face_vbo.push_back(uvBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        //face_vbo.push_back(uvTop);
    }

    return face_vbo;
}

std::vector<glm::vec4> Chunk::createFacesWithUV(std::array<bool, 6> faces, int x, int y, int z) {
    // Get face color based on block type.
    // All faces will use this color
    BlockType block = getBlockAt(x, y, z);
    glm::vec4 color;
    glm::vec4 top;
    glm::vec4 bot;
    glm::vec4 sideTop;
    glm::vec4 sideBot;
    switch(block) {
        case GRASS:
            //color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
            top = glm::vec4(8.f/16.f, 13.f/16.f, 0.f, 0.f);
            bot = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideTop = glm::vec4(3.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideBot = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            break;
        case DIRT:
            color = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
            top = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            bot = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideTop = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideBot = glm::vec4(2.f/16.f, 15.f/16.f, 0.f, 0.f);
            break;
        case STONE:
            color = glm::vec4(0.5f);
            top = glm::vec4(1.f/16.f, 15.f/16.f, 0.f, 0.f);
            bot = glm::vec4(1.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideTop = glm::vec4(1.f/16.f, 15.f/16.f, 0.f, 0.f);
            sideBot = glm::vec4(1.f/16.f, 15.f/16.f, 0.f, 0.f);
            break;
        case SNOW:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            top = glm::vec4(2.f/16.f, 11.f/16.f, 0.f, 0.f);
            bot = glm::vec4(2.f/16.f, 11.f/16.f, 0.f, 0.f);
            sideTop = glm::vec4(2.f/16.f, 11.f/16.f, 0.f, 0.f);
            sideBot = glm::vec4(2.f/16.f, 11.f/16.f, 0.f, 0.f);
            break;
        case ICE:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            top = glm::vec4(3.f/16.f, 11.f/16.f, 0.f, 0.f);
            bot = glm::vec4(3.f/16.f, 11.f/16.f, 0.f, 0.f);
            sideTop = glm::vec4(3.f/16.f, 11.f/16.f, 0.f, 0.f);
            sideBot = glm::vec4(3.f/16.f, 11.f/16.f, 0.f, 0.f);
            break;
        case LAVA:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            top = glm::vec4(14.f/16.f, 1.f/16.f, 0.f, 1.f);
            bot = glm::vec4(14.f/16.f, 0.f/16.f, 0.f, 1.f);
            sideTop = glm::vec4(15.f/16.f, 1.f/16.f, 0.f, 1.f);
            sideBot = glm::vec4(15.f/16.f, 0.f/16.f, 0.f, 1.f);
            break;
        case WATER:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            top = glm::vec4(14.f/16.f, 3.f/16.f, 0.f, 1.f);
            bot = glm::vec4(14.f/16.f, 2.f/16.f, 0.f, 1.f);
            sideTop = glm::vec4(15.f/16.f, 3.f/16.f, 0.f, 1.f);
            sideBot = glm::vec4(15.f/16.f, 2.f/16.f, 0.f, 1.f);
            break;
        default:
            // Other block types are not yet handled, so we default to black
            color = glm::vec4(0.f);
            break;
    }

    // Create a translation matrix based on the x, y, z components
    glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(x, y, z));

    // Copied coordinates from cube initialization.
    // Could be done in less lines
    std::vector<glm::vec4> face_vbo;

    // Right face
    if (faces[0]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        face_vbo.push_back(sideTop);

        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        face_vbo.push_back(sideBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        face_vbo.push_back(sideBot);
        //UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        face_vbo.push_back(sideTop);
    }
    // Left face
    if (faces[1]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        face_vbo.push_back(sideTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        face_vbo.push_back(sideBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        face_vbo.push_back(sideBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        face_vbo.push_back(sideTop);
    }
    // Top face
    if (faces[2]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        face_vbo.push_back(top);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        face_vbo.push_back(top);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        face_vbo.push_back(top);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        face_vbo.push_back(top);
    }
    // Bottom face
    if (faces[3]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        face_vbo.push_back(bot);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        face_vbo.push_back(bot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        face_vbo.push_back(bot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        face_vbo.push_back(bot);
    }
    // Front face
    if (faces[4]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        face_vbo.push_back(sideTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        face_vbo.push_back(sideBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        face_vbo.push_back(sideBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        face_vbo.push_back(sideTop);
    }
    // Back face
    if (faces[5]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        face_vbo.push_back(sideTop);
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        face_vbo.push_back(sideBot);
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        face_vbo.push_back(sideBot);
        // UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        face_vbo.push_back(sideTop);
    }

    return face_vbo;
}

// Buffering function as specified in the project specs.
// Might need to be moved elsewhere when it's needed for future milestones.
void Chunk::bufferData(const std::vector<glm::vec4> &interleaved, const std::vector<GLuint> &idx) {
    m_count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // Utilizes just the position buffer for the only VBO;
    // should probably create a separate one if it ends up matterings
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(glm::vec4), interleaved.data(), GL_STATIC_DRAW);
}

void Chunk::bufferDataTrans(const std::vector<glm::vec4> &interleaved, const std::vector<GLuint> &idx){
    m_count_trans = idx.size();

    generateIdxTrans();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTrans);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count_trans * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateTrans();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTrans);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(glm::vec4), interleaved.data(), GL_STATIC_DRAW);
}

void Chunk::create() {
    // Interleaved takes the form pos0col0nor0pos1col1nor1...
    std::vector<glm::vec4> interleaved;
    std::vector<glm::vec4> interleavedTrans;

    // Loop through each block in the chunk array;
    // check each neighbor of the block, and add a face
    // for each neighbor that is EMPTY
    for(int x = 0; x < 16; ++x) {
        for(int y = 0; y < 256; ++y) {
            for(int z = 0; z < 16; ++z) {
                // Skip all empty blocks; they won't have faces to draw
                if (getBlockAt(x, y, z) == EMPTY) {
                    continue;
                }
                // Check which faces need to be drawn
                std::array<bool, 6> faces = checkBlockFaces(x, y, z);
                // Generate a vector of all the interleaved face data
                //std::vector<glm::vec4> new_faces = createFaces(faces, x, y, z);
                std::vector<glm::vec4> new_faces = createFacesWithUV(faces, x, y, z);
                // Append this vector to the current interleaved
                if(getBlockAt(x, y, z) == WATER || getBlockAt(x, y, z) == LAVA){
                    interleavedTrans.insert(std::end(interleavedTrans), std::begin(new_faces), std::end(new_faces));
                } else {
                    interleaved.insert(std::end(interleaved), std::begin(new_faces), std::end(new_faces));
                }

            }
        }
    }
    // Create the index buffer from the interleaved VBO
    std::vector<GLuint> idx;
    std::vector<GLuint> idxTrans;

    for(uint i = 0; i < interleavedTrans.size(); i +=4) {
        idxTrans.push_back(i);
        idxTrans.push_back(i+1);
        idxTrans.push_back(i+2);
        idxTrans.push_back(i);
        idxTrans.push_back(i+2);
        idxTrans.push_back(i+3);
    }

    for(uint i = 0; i < interleaved.size(); i +=4) {
        idx.push_back(i);
        idx.push_back(i+1);
        idx.push_back(i+2);
        idx.push_back(i);
        idx.push_back(i+2);
        idx.push_back(i+3);
    }

    // Upload this data to the VBO
    bufferDataTrans(interleavedTrans, idxTrans);
    bufferData(interleaved, idx);

    // Don't generate this chunk again
    generated = true;
}

// Poor design, but this is just the duplicated create() that passes the results to vectors
// instead of pushing it to VBOs so the threads can use the function.
void Chunk::create(std::vector<glm::vec4> &interleaved, std::vector<GLuint> &idx) {
    // Interleaved takes the form pos0col0nor0pos1col1nor1...

    // Loop through each block in the chunk array;
    // check each neighbor of the block, and add a face
    // for each neighbor that is EMPTY
    for(int x = 0; x < 16; ++x) {
        for(int y = 0; y < 256; ++y) {
            for(int z = 0; z < 16; ++z) {
                // Skip all empty blocks; they won't have faces to draw
                if (getBlockAt(x, y, z) == EMPTY) {
                    continue;
                }
                // Check which faces need to be drawn
                std::array<bool, 6> faces = checkBlockFaces(x, y, z);
                // Generate a vector of all the interleaved face data
                std::vector<glm::vec4> new_faces = createFaces(faces, x, y, z);
                // Append this vector to the current interleaved
                interleaved.insert(std::end(interleaved), std::begin(new_faces), std::end(new_faces));
            }
        }
    }
    // Create the index buffer from the interleaved VBO
    for(uint i = 0; i < interleaved.size(); i +=4) {
        idx.push_back(i);
        idx.push_back(i+1);
        idx.push_back(i+2);
        idx.push_back(i);
        idx.push_back(i+2);
        idx.push_back(i+3);
    }

    // Don't generate this chunk again
    generated = true;
}

// Builds procedural terrain for the chunk based on its offset
void Chunk::generateChunk(int x_offset, int z_offset) {
    for(int x = 0; x < 16; ++x) {
        for(int z = 0; z < 16; ++z) {
            createBlock(x, z, x_offset, z_offset);
        }
    }
}

//Perlin noise helper functions
glm::vec2 Chunk::random2(glm::vec2 p) {
    return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(123.4, 321.7)), glm::dot(p, glm::vec2(258.1, 195.3)))) * 2343524.545324f);
}

float Chunk::surflet(glm::vec2 p, glm::vec2 gridPt) {
    glm::vec2 t2 = glm::abs(p - gridPt);
    glm::vec2 t = glm::vec2(1.f) - 6.f * glm::pow(t2, glm::vec2(5.f)) + 15.f * glm::pow(t2, glm::vec2(4.f)) - 10.f * glm::pow(t2, glm::vec2(3.f));
    glm::vec2 gradient = random2(gridPt) * 2.f - glm::vec2(1, 1);
    glm::vec2 diff = p - gridPt;
    float height = glm::dot(diff, gradient);
    return height * t.x * t.y;
}

//Perlin noise
float Chunk::perlin(glm::vec2 uv) {
    float surfletSum = 0.f;
    for(int dx = 0; dx <= 1; dx++) {
        for(int dy = 0; dy <= 1; dy++) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }
    return surfletSum;
}

//Worley noise
float Chunk::worley(glm::vec2 uv) {
    uv = uv * 2.f;
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.f;

    for (int y = -1; y <= 1; y++) {
        for (int x  =-1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(uvInt + neighbor);
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

float Chunk::noise1D(int x) {
    return glm::fract(glm::sin(glm::dot(glm::vec2(x, x * 123456432), glm::vec2(124.3, 235.5))) * 213454.54343);
}

float Chunk::interpNoise1D(float x) {
    int intX = int(floor(x));
    float fractX = glm::fract(x);

    float v1 = noise1D(intX);
    float v2 = noise1D(intX + 1);
    return glm::mix(v1, v2, fractX);
}

float Chunk::fbm(float x) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i <= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise1D(x * freq) * amp;
    }
    return total;
}

int Chunk::grassHeight(int x, int z) {
    float noise = worley(glm::vec2(x / 64.f, z / 64.f));
    return 129 + noise * 40 + 5;
}

int Chunk::mountainHeight(int x, int z) {
    float noise = perlin(glm::vec2(x / 32.f, z / 32.f));
    noise = glm::smoothstep(0.25, 0.75, double(noise));
    noise = pow(noise, 2);
    return noise * 127 + 129;
}

void Chunk::createBlock(int x, int z, int x_offset, int z_offset) {
    int x_coord = x + x_offset;
    int z_coord = z + z_offset;
    int grHeight = grassHeight(x_coord, z_coord);
    int mtHeight = mountainHeight(x_coord, z_coord);

    float pn = perlin(glm::vec2(x_coord / 256.f, z_coord / 256.f)) * 0.5 + 0.5;
    pn = glm::smoothstep(0.25, 0.75, double(pn));
    pn = pn * 2;
    int lerp = int((1 - pn) * grHeight + pn * mtHeight);
    //grass
    if (pn > 0.5) {
        for (int i = 0; i < lerp; i++) {
            if (i == lerp - 1) {
                setBlockAt(x, i, z, GRASS);
            } else if (i <= 128 && i >= 0) {
                setBlockAt(x, i, z, STONE);
            } else if (i > 128) {
                setBlockAt(x, i, z, DIRT);
            } else {
                setBlockAt(x, i, z, EMPTY);
            }
        }
    }
    else {
       //mountains
        for (int i = 0; i < lerp; i++) {
            if (i == lerp -1 && lerp > 200) {
                setBlockAt(x, i, z, SNOW);
            } else if (i < 0) {
                setBlockAt(x, i, z, EMPTY);
            } else {
                setBlockAt(x, i, z, STONE);
            }
        }
    }
}
