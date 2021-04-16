#include "blocktypeworker.h"
#include "scene/river.h"
#include <iostream>

BlockTypeWorker::BlockTypeWorker(OpenGLContext *context, Terrain *terrain, QMutex *m, int x, int z)
    : ctx(context), m_terrain(terrain), mutex(m), x_offset(x), z_offset(z) {}

void BlockTypeWorker::run() {
    std::vector< uPtr<Chunk> > chunks;
    for(int i = 0; i < 64; i += 16) {
        for(int j = 0; j < 64; j += 16) {
            int new_x = x_offset + i;
            int new_z = z_offset + j;
            chunks.push_back(mkU<Chunk>(Chunk(ctx)));
            Chunk *chunk = chunks.back().get();
            chunk->x_offset = new_x;
            chunk->z_offset = new_z;
            chunk->generateChunk(new_x, new_z);

        }
    }
    River river = River(m_terrain, x_offset, z_offset);
//    double putRiver = double(rand()) / RAND_MAX; //probability of river
//    if (putRiver < 0.2) {
//        river.makeRiver();
//        std::cout << "River Created" << std::endl;
//    }
    // Move the chunks off of the thread
    mutex->lock();
    for(unsigned int i = 0; i < chunks.size(); ++i) {
        m_terrain->gen_chunks.push_back(std::move(chunks[i]));
    }
    mutex->unlock();
}
