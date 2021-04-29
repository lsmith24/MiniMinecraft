#pragma once

#include "drawable.h"
#include <glm_includes.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Cube : public Drawable
{
private:
    glm::vec3 color;
public:
    Cube(OpenGLContext* context) : Drawable(context), color(){}
    Cube(OpenGLContext* context, glm::vec3 color);
    virtual ~Cube(){}
    void create() override;
};
