#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <QDateTime>
#include "math.h"

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context),
      thread_pool(QThreadPool::globalInstance()), block_workers(),
      vbo_workers(), gen_chunks(), chunk_mtx()
{
    // NOTE: remove unless needed
    //thread_pool->setMaxThreadCount(25); // 25 threads available, one for each possible terrain generation zone
}

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
        c->create();
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

    return cPtr;
}

void Terrain::generateChunk(Chunk* c, int x_offset, int z_offset) {
    // Create the basic terrain floor
    for(int x = 0; x < 16; ++x) {
        for(int z = 0; z < 16; ++z) {
            c->createBlock(x, z, x_offset, z_offset);
        }
    }
    c->create();
}
// NOTE: remove the generic terrain generation when other terrain generation is implemented
void Terrain::expandChunks(const Player &player) {
    // Get the zone that the player is currently in
    int player_x = static_cast<int>(glm::floor(player.mcr_position[0] / 64.f) * 64);
    int player_z = static_cast<int>(glm::floor(player.mcr_position[2] / 64.f) * 64);

    // Add new terrain zones if needed
    for(int x = -128; x <= 128; x += 64) {
        for(int z = -128; z <= 128; z += 64) {
            int new_x = player_x + x;
            int new_z = player_z + z;

            // If the key is not in the set, add it and generate the new terrain zone
            if (m_generatedTerrain.count(toKey(new_x, new_z)) <= 0) {
               m_generatedTerrain.insert(toKey(new_x, new_z));
               // Spawn a worker thread to create the chunk and its blocks
               block_workers.push_back(mkU<BlockTypeWorker>(BlockTypeWorker(mp_context, this, &chunk_mtx, new_x, new_z)));
               thread_pool->start(block_workers.back().get());
               // NOTE: the worker queue should be reset at some point, but since only the back is used
               // it shouldn't create any immediate problems.
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
               shaderProgram->drawInterleavedTrans(*chunk, time);
               shaderProgram->drawInterleavedOpaque(*chunk, time);
            }
        }
    }
}

// Build the base 3x3 zones when the program is started
void Terrain::CreateTestScene() {
    qint64 start_time = QDateTime::currentMSecsSinceEpoch();
    std::cout << "Creating base scene..." << std::endl;
    // Create the chunks of the starting area (3x3 terrain generation zones)
    for(int x = -64; x <= 64; x += 64) {
        for(int z = -64; z <= 64; z += 64) {
            m_generatedTerrain.insert(toKey(x, z));
            for(int x2 = 0; x2 < 64; x2 += 16) {
                for(int z2 = 0; z2 < 64; z2 += 16) {
                    Chunk* c = instantiateChunkAt(x + x2, z + z2);
                    generateChunk(c, x + x2, z + z2);
                }
            }
        }
    }


    std::cout << "Finished creating base scene in " << (QDateTime::currentMSecsSinceEpoch() - start_time) / 1000.0f << " seconds" << std::endl;
}

// Move chunks created from threads to the terrain chunk structure
void Terrain::updateChunks() {
    chunk_mtx.lock();
    if (!gen_chunks.empty()) {
        for(unsigned int i = 0; i < gen_chunks.size(); ++i) {
            int x_offset = gen_chunks[i]->x_offset;
            int z_offset = gen_chunks[i]->z_offset;
            m_chunks[toKey(x_offset, z_offset)] = std::move(gen_chunks[i]);
        }
        gen_chunks.clear();
    }
    chunk_mtx.unlock();
}

void Terrain::updateVBOs() {
    // First, check each terrain generation zone to see if its VBO data needs to be generated
    for(int64_t key : m_generatedTerrain) {
        glm::vec2 coord = toCoords(key);
        // Check the chunks starting from this coordintes
        for(int x = coord.x; x < coord.x + 64; x += 16) {
            for(int z = coord.y; z < coord.y + 64; z += 16) {
                // Check that the worker thread for this zone
                // has finished; if a chunk is missing, assume
                // the zone is not finished and stop initializing.
                if (hasChunkAt(x,z)) {
                    Chunk *c = getChunkAt(x,z).get();
                    if (!c->generating && !c->generated) {
                        vbo_workers.push_back(mkU<VBOWorker>(VBOWorker(c)));
                        vbo_workers.back()->setAutoDelete(false);
                        thread_pool->start(vbo_workers.back().get());
                        c->generating = true;
                    }
                }
                else {
                    continue;
                }
            }
        }
    }
    // Check VBOWorkers to see if they have computed their VBO data; if so, send to GPU and delete worker
    // NOTE: should probably use a mutex, but it hasn't created problems yet.w
    for(unsigned int i = 0; i < vbo_workers.size(); ++i) {
        if(vbo_workers[i]->isCompleted()) {
            vbo_workers[i]->getChunk()->bufferData(vbo_workers[i]->getData().opaque_vertex, vbo_workers[i]->getData().opaque_index);
            vbo_workers.erase(vbo_workers.begin() + i);
            --i;
        }
    }
}

void Terrain::setTime(int t) {
    time = t;
}
