#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
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

std::vector<glm::vec4> Chunk::createFaces(std::array<bool, 6> faces, int x, int y, int z) {
    // Get face color based on block type.
    // All faces will use this color
    BlockType block = getBlockAt(x, y, z);
    glm::vec4 color;
    switch(block) {
        case GRASS:
            color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
            break;
        case DIRT:
            color = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
            break;
        case STONE:
            color = glm::vec4(0.5f);
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
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
        //UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(1,0,0,0)); // Normal
    }
    // Left face
    if (faces[1]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(-1,0,0,0)); // Normal
    }
    // Top face
    if (faces[2]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,1,0,0)); // Normal
    }
    // Bottom face
    if (faces[3]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,-1,0,0)); // Normal
    }
    // Front face
    if (faces[4]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        // LR
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
        // UL
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,1,0)); // Normal
    }
    // Back face
    if (faces[5]) {
        // UR
        face_vbo.push_back(translate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        // LR
        face_vbo.push_back(translate * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        // LL
        face_vbo.push_back(translate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
        // UL
        face_vbo.push_back(translate * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Position
        face_vbo.push_back(color); // Color
        face_vbo.push_back(glm::vec4(0,0,-1,0)); // Normal
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

void Chunk::create() {
    // Interleaved takes the form pos0col0nor0pos1col1nor1...
    std::vector<glm::vec4> interleaved;

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
    std::vector<GLuint> idx;
    for(uint i = 0; i < interleaved.size(); i +=4) {
        idx.push_back(i);
        idx.push_back(i+1);
        idx.push_back(i+2);
        idx.push_back(i);
        idx.push_back(i+2);
        idx.push_back(i+3);
    }

    // Upload this data to the VBO
    bufferData(interleaved, idx);
}
