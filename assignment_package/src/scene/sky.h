#pragma once
#include "drawable.h"
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>

class Sky : public Drawable
{
public:
    Sky(OpenGLContext* context);
    virtual void create() override;
};
