#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include <array>
#include <unordered_map>
#include <cstddef>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, SNOW, ICE, LAVA, WATER
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

public:
    Chunk(OpenGLContext* context);

    // Needed for multithreading
    int x_offset, z_offset;
    bool generating;
    bool generated;

    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    std::array<bool, 6> checkBlockFaces(int x, int y, int z);
    std::vector<glm::vec4> createFaces(std::array<bool, 6> faces, int x, int y, int z);
    std::vector<glm::vec4> createFacesWithUV(std::array<bool, 6> faces, int x, int y, int z);
    void bufferData(const std::vector<glm::vec4> &interleaved, const std::vector<GLuint> &idx);
    void bufferDataTrans(const std::vector<glm::vec4> &interleaved, const std::vector<GLuint> &idx);
    void create() override;
    void create(std::vector<glm::vec4> &interleaved, std::vector<GLuint> &idx, std::vector<glm::vec4> &interleavedTrans, std::vector<GLuint> &idxTrans);

    // Fills chunk with procedural height field data
    void generateChunk(int x_offset, int z_offset);

    glm::vec2 random2(glm::vec2 p);
    float surflet(glm::vec2 p, glm::vec2 gridPt);
    float perlin(glm::vec2 uv);
    float worley(glm::vec2 uv);
    float noise1D(int x);
    float interpNoise1D(float x);
    float fbm(float x);
    int grassHeight(int x, int z);
    int mountainHeight(int x, int z);
    void createBlock(int x, int z, int x_offset, int z_offset);
};
