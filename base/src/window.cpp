#include "window.hpp"

RTVE::Window RTVE::Window::sInstance;

void RTVE::GLFWframebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight) {
  RTVE::Window::get().setViewportSize(glm::vec2(pWidth, pHeight));
}

void RTVE::GLFWmouseCallback(GLFWwindow* pWindow, double pX, double pY) {
  RTVE::Window::get().mouseCallback(glm::vec2(pX, pY));
}

void RTVE::Window::init(const char* pTitle) {
  // GLFW Init and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _MACOSX
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // GLFW Window creation
  mSize.x = 720;
  mSize.y = 720;
  mWindow = glfwCreateWindow(mSize.x, mSize.y, pTitle, NULL, NULL);
  if (mWindow == NULL) {
    std::println("Failed to create GLFW window");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(mWindow);

  // Enable VSYNC by default
  glfwSwapInterval(-1);

  // GLFW callbacks
  glfwSetFramebufferSizeCallback(mWindow, RTVE::GLFWframebufferSizeCallback);
  glfwSetCursorPosCallback(mWindow, RTVE::GLFWmouseCallback);
  // glfwSetScrollCallback(mWindow, GLFWscrollCallback);

  // GLAD Load opengl function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    std::println("Failed to initialize GLAD");

  glEnable(GL_DEPTH_TEST);

  glLineWidth(2);
}

RTVE::Window::~Window() {
  glfwDestroyWindow(mWindow);
}

void RTVE::Window::setClearColor(const glm::vec4& pClearColor) {
  mClearColor = pClearColor;
}

void RTVE::Window::clear() {
  glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, mClearColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RTVE::Window::swapBuffers() {
  glfwSwapBuffers(mWindow);
}

void RTVE::Window::pollEvents() {
  glfwPollEvents();
}

void RTVE::Window::captureCursor() {
  glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

int RTVE::Window::getKeyGLFW(int pKey) {
  return glfwGetKey(mWindow, pKey);
}

bool RTVE::Window::shouldWindowClose() {
  return glfwWindowShouldClose(mWindow);
}

void RTVE::Window::setViewportSize(glm::vec2 pSize) {
  glViewport(0, 0, pSize.x, pSize.y);
  mSize = pSize;
}

void RTVE::Window::updateViewportSize() {
  int scrWidth, scrHeight;
  glfwGetFramebufferSize(mWindow, &scrWidth, &scrHeight);
  glViewport(0, 0, scrWidth, scrHeight);
}

glm::vec2 RTVE::Window::getSize() {
  return mSize;
}

double RTVE::Window::getTime() {
  return glfwGetTime();
}

