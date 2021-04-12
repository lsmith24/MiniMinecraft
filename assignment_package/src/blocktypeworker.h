#pragma once
#ifndef BLOCKTYPEWORKER_H
#define BLOCKTYPEWORKER_H

class BlockTypeWorker;

#include <QRunnable>
#include <QMutex>
#include "scene/terrain.h"

class BlockTypeWorker : public QRunnable {
private:
    OpenGLContext *ctx;
    Terrain *m_terrain;
    QMutex *mutex;
    int x_offset, z_offset;
public:
    BlockTypeWorker(OpenGLContext *context, Terrain *terrain, QMutex *m, int x, int z);
    void run() override;
};

#endif // BLOCKTYPEWORKER_H
