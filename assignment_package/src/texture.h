#pragma once

#include <openglcontext.h>
#include <memory>

class Texture
{
public:
    Texture(OpenGLContext* context);

    void create(const QString filePath);
    void load(int texSlot);
    void bind(int texSlot);

    OpenGLContext* context;
    GLuint m_textureHandle;
    std::shared_ptr<QImage> m_textureImage;
};
