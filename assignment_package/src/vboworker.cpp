#include "vboworker.h"

VBOWorker::VBOWorker(Chunk *c)
    : chunk(c), vbo_data(), completed(false) {}

bool VBOWorker::isCompleted() {
    return completed;
}

Chunk* VBOWorker::getChunk() {
    return chunk;
}

VBOData VBOWorker::getData() {
    return vbo_data;
}

void VBOWorker::run() {
    chunk->create(vbo_data.opaque_vertex, vbo_data.opaque_index);
    completed = true;
}
