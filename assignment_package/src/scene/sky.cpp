#include "sky.h"

Sky::Sky(OpenGLContext* context) : Drawable(context)
{}

void Sky::create() {
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_pos[4] {glm::vec4(-1.f, -1.f, 0.9999f, 1.f),
                           glm::vec4(1.f, -1.f, 0.9999f, 1.f),
                           glm::vec4(1.f, 1.f, 0.9999f, 1.f),
                           glm::vec4(-1.f, 1.f, 0.9999f, 1.f)};
    m_count = 6;

    //create VBO
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_pos, GL_STATIC_DRAW);
}

//void Sky::create()
//{
//    GLuint idx[6]{0, 1, 2, 0, 2, 3};
//    glm::vec4 dummyCol(1, 0, 0, 0); //red
//    glm::vec4 dummyNor(0, 0, 1, 0);
//    glm::vec4 dummy(0, 0, 0, 0);
//    std::vector<glm::vec4> vbo( {glm::vec4(-1.f, -1.f, 0.999999f, 1.f), dummyNor, dummyCol, dummy,
//                           glm::vec4(1.f, -1.f, 0.999999f, 1.f), dummyNor, dummyCol, dummy,
//                           glm::vec4(1.f, 1.f, 0.999999f, 1.f), dummyNor, dummyCol, dummy,
//                           glm::vec4(-1.f, 1.f, 0.999999f, 1.f), dummyNor, dummyCol, dummy});

//    m_count = 6;

//    // Create a VBO on our GPU and store its handle in bufIdx
//    generateIdx();
//    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
//    // and that it will be treated as an element array buffer (since it will contain triangle indices)
//    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
//    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
//    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
//    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

//    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
//    // array buffers rather than element array buffers, as they store vertex attributes like position.
//    generateCombo();
//    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCombo);
//    mp_context->glBufferData(GL_ARRAY_BUFFER, vbo.size() * sizeof(glm::vec4), vbo.data(), GL_STATIC_DRAW);

//}

