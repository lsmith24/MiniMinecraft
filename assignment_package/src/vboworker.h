#pragma once
#ifndef VBOWORKER_H
#define VBOWORKER_H

class VBOWorker;

#include <QRunnable>
#include "scene/terrain.h"

struct VBOData {
    std::vector<glm::vec4> opaque_vertex;
    std::vector<GLuint> opaque_index;
    std::vector<glm::vec4> trans_vertex;
    std::vector<GLuint> trans_index;
};

class VBOWorker : public QRunnable {
private:
    Chunk *chunk;
    VBOData vbo_data;
    bool completed;
public:
    VBOWorker(Chunk *c);
    bool isCompleted();
    Chunk* getChunk();
    VBOData getData();
    void run() override;
};

#endif // VBOWORKER_H
