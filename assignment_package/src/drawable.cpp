#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_count_opq(-1), m_count_tran(-1),
      m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(),
      m_bufOpAll(), m_bufTransAll(), m_bufIdxOpq(), m_bufIdxTran(),
      m_opAllGenerated(false), m_transAllGenerated(false),
      m_idxOpqGenerated(false), m_idxTranGenerated(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufOpAll);
    mp_context->glDeleteBuffers(1, &m_bufTransAll);
    mp_context->glDeleteBuffers(1, &m_bufIdxOpq);
    mp_context->glDeleteBuffers(1, &m_bufIdxTran);
    m_opAllGenerated = m_transAllGenerated = m_idxOpqGenerated = m_idxTranGenerated = false;
    m_count_tran = -1;
    m_count_opq = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

int Drawable::elemCountOpq()
{
    return m_count_opq;
}

int Drawable::elemCountTran()
{
    return m_count_tran;
}


void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generateIdxOpq()
{
    m_idxOpqGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxOpq);
}

void Drawable::generateIdxTran()
{
    m_idxTranGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxTran);
}

void Drawable::generateOpAll() {
    mp_context->glGenBuffers(1, &m_bufOpAll);
    m_opAllGenerated = true;
}

void Drawable::generatedTransAll() {
    mp_context->glGenBuffers(1, &m_bufTransAll);
    m_transAllGenerated = true;
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}


bool Drawable::bindIdxOpq()
{
    if(m_idxOpqGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
    }
    return m_idxOpqGenerated;
}

bool Drawable::bindIdxTran()
{
    if(m_idxTranGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTran);
    }
    return m_idxTranGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindOpAll() {
    if (m_opAllGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpAll);
    }
    return m_opAllGenerated;
}

bool Drawable::bindTransAll() {
    if (m_transAllGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransAll);
    }
    return m_transAllGenerated;
}
