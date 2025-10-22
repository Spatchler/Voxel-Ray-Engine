#pragma once
#include <print>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RTVE {
  void GLFWframebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight);

  class Window {
  public:
    Window(const Window&) = delete;
    static Window& get() { return sInstance; }

    void init(const char* pTitle);
    ~Window();

    void setClearColor(const glm::vec4& pClearColor);
    void clear();
    void swapBuffers();
    void pollEvents();

    bool shouldWindowClose();

    void setViewportSize(glm::vec2 pSize);
    void updateViewportSize();
    glm::vec2 getSize();

    double getTime();

    void setClearColor(glm::vec4 pClearColor);
  private:
    Window() {};
    static Window sInstance;

    glm::vec2 mSize;
    glm::vec4 mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);

    GLFWwindow* mWindow = NULL;
  };
}

