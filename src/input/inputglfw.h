#ifndef INPUTGLFW_H
#define INPUTGLFW_H

#include "input.h"

class GLFWwindow;

class InputGLFW : public Input
{
    GLFWwindow * m_owner_window{nullptr};

public:
    InputGLFW(GLFWwindow * owner);
    ~InputGLFW() override = default;
};

#endif   // INPUTGLFW_H
