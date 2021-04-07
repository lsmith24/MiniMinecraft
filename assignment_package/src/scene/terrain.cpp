#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include "math.h"

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(),  mp_context(context)
{}

Terrain::~Terrain() {
    // Destroy all chunks
    for (const auto &c : m_chunks) {
        c.second->destroy();
    }
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

std::unordered_set<int64_t> Terrain::getTerrainZones() {
    return m_generatedTerrain;
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(Chunk(mp_context));
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    for(int i = 0; i < 16; ++i) {
        for(int j = 0; j < 16; ++j) {
            createBlock(x + i, z + j);
        }
    }
    return cPtr;
}

void Terrain::createGenericChunk(int chunk_x, int chunk_z) {
    // Create the basic terrain floor
    const uPtr<Chunk> &chunk = getChunkAt(chunk_x, chunk_z);
//    for(int x = chunk_x; x < chunk_x + 16; ++x) {
//        for(int z = chunk_z; z < chunk_z + 16; ++z) {
//            if((x + z) % 2 == 0) {
//                chunk->setBlockAt(x, 128, z, STONE);
//            }
//            else {
//                chunk->setBlockAt(x, 128, z, DIRT);
//            }
            //createBlock(x, z);
//        }
//    }
    chunk->create();
}

// NOTE: remove the generic terrain generation when other terrain generation is implemented
void Terrain::expandChunks(const Player &player) {
    // Get the zone that the player is currently in
    int player_x = static_cast<int>(glm::floor(player.mcr_position[0] / 64.f) * 64);
    int player_z = static_cast<int>(glm::floor(player.mcr_position[2] / 64.f) * 64);

    // Add new terrain zones if needed
    for(int x = -64; x <= 64; x += 64) {
        for(int z = -64; z <= 64; z += 64) {
            int new_x = player_x + x;
            int new_z = player_z + z;
            // If the key is not in the set, add it and generate the new terrain zone
            if (m_generatedTerrain.count(toKey(new_x, new_z)) <= 0) {
               m_generatedTerrain.insert(toKey(new_x, new_z));
               // Check / create the 16 chunks of the zone
               // If chunks are created indepently of a terrain zone,
               // a check should be added to ensure a chunk is not instaniated
               // multiple times
               if(!hasChunkAt(new_x, new_z)) {
                   for(int x2 = 0; x2 < 64; x2 += 16) {
                       for(int z2 = 0; z2 < 64; z2 += 16) {
                           instantiateChunkAt(new_x + x2, new_z + z2);
                           createGenericChunk(new_x + x2, new_z + z2);
                       }
                   }
               }
            }
        }
    }
}

void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if (hasChunkAt(x, z)) {
               const uPtr<Chunk> &chunk = getChunkAt(x, z);
               shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
               shaderProgram->drawInterleaved(*chunk);
            }
        }
    }
}

void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = -64; x < 128; x += 16) {
        for(int z = -64; z < 128; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    for(int i = -64; i <= 64; i += 64) {
        for(int j = -64; j <=64; j += 64) {
            m_generatedTerrain.insert(toKey(i, j));
        }
    }

    // Build all of the chunks
    for (const auto &c : m_chunks) {
        c.second->create();
    }
}

//Perlin noise helper functions
glm::vec2 Terrain::random2(glm::vec2 p) {
    return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(123.4, 321.7)), glm::dot(p, glm::vec2(258.1, 195.3)))) * 2343524.545324f);
}

float Terrain::surflet(glm::vec2 p, glm::vec2 gridPt) {
    glm::vec2 t2 = glm::abs(p - gridPt);
    glm::vec2 t = glm::vec2(1.f) - 6.f * glm::pow(t2, glm::vec2(5.f)) + 15.f * glm::pow(t2, glm::vec2(4.f)) - 10.f * glm::pow(t2, glm::vec2(3.f));
    glm::vec2 gradient = random2(gridPt) * 2.f - glm::vec2(1, 1);
    glm::vec2 diff = p - gridPt;
    float height = glm::dot(diff, gradient);
    return height * t.x * t.y;
}

//Perlin noise
float Terrain::perlin(glm::vec2 uv) {
    float surfletSum = 0.f;
    for(int dx = 0; dx <= 1; dx++) {
        for(int dy = 0; dy <= 1; dy++) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }
    return surfletSum;
}

//Worley noise
float Terrain::worley(glm::vec2 uv) {
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

float Terrain::noise1D(int x) {
    return glm::fract(glm::sin(glm::dot(glm::vec2(x, x * 123456432), glm::vec2(124.3, 235.5))) * 213454.54343);
}

float Terrain::interpNoise1D(float x) {
    int intX = int(floor(x));
    float fractX = glm::fract(x);

    float v1 = noise1D(intX);
    float v2 = noise1D(intX + 1);
    return glm::mix(v1, v2, fractX);
}

float Terrain::fbm(float x) {
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

int Terrain::grassHeight(int x, int z) {
    float noise = worley(glm::vec2(x / 64.f, z / 64.f));
    return 129 + noise * 40 + 5;
}

int Terrain::mountainHeight(int x, int z) {
    float noise = perlin(glm::vec2(x / 32.f, z / 32.f));
    noise = glm::smoothstep(0.25, 0.75, double(noise));
    noise = pow(noise, 2);
    return noise * 127 + 129;
}

void Terrain::createBlock(int x, int z) {
    int grHeight = grassHeight(x, z);
    int mtHeight = mountainHeight(x, z);

    float pn = perlin(glm::vec2(x / 256.f, z / 256.f)) * 0.5 + 0.5;
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

//void Terrain::recreateChunk(int x, int z) {
//    vboDatamut.lock();
//    Chunk* c = getChunkAt(x, z).get();
//    c->create();
//    vboDatamut.unlock();

//}

//void Terrain::recreateChunk(int x, int z) {
//    vboDatamut.lock();
//    Chunk* c = getChunkAt(x, z).get();
//    c->create();
//    vboDatamut.unlock();

//}
