#include "shader.hpp"

RTVE::Shader::Shader(const std::string& pVertexPath, const std::string& pFragPath) {
  load(pVertexPath, pFragPath);
}

void RTVE::Shader::load(const std::string& pVertexPath, const std::string& pFragPath) {
  std::string vCode;
  std::string fCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // Read files
    vShaderFile.open(pVertexPath);
    fShaderFile.open(pFragPath);
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vShaderFile.close();
    fShaderFile.close();
    vCode = vShaderStream.str();
    fCode = fShaderStream.str();
  }
  catch (std::ifstream::failure& e) {
    std::println("SHADER::FILE_NOT_SUCCESSFULLY_READ: {0}", e.what());
  }
  const char* vShaderCode = vCode.c_str();
  const char* fShaderCode = fCode.c_str();

  // Compile
  uint vert, frag;
  // Vertex
  vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vShaderCode, NULL);
  glCompileShader(vert);
  checkCompileErrors(vert, "VERTEX");
  // Fragment
  frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fShaderCode, NULL);
  glCompileShader(frag);
  checkCompileErrors(frag, "FRAGMENT");

  // Shader program
  mID = glCreateProgram();
  glAttachShader(mID, vert);
  glAttachShader(mID, frag);
  glLinkProgram(mID);
  checkCompileErrors(mID, "PROGRAM");
  // Shader's are linked and no longer needed
  glDeleteShader(vert);
  glDeleteShader(frag);
}

void RTVE::Shader::use() const {
  glUseProgram(mID);
}

uint RTVE::Shader::getID() {
  return mID;
}

// ------------------------------------------------------------------------

void RTVE::Shader::setBool(const std::string& name, bool value) const {
  use();
  glUniform1i(glGetUniformLocation(mID, name.c_str()), (int)value);
}
void RTVE::Shader::setInt(const std::string& name, int value) const {
  use();
  glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
}
void RTVE::Shader::setFloat(const std::string& name, float value) const {
  use();
  glUniform1f(glGetUniformLocation(mID, name.c_str()), value);
}

// ------------------------------------------------------------------------

void RTVE::Shader::setVec2(const std::string& name, const glm::vec2& value) const {
  use();
  glUniform2fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]); 
}
void RTVE::Shader::setVec2(const std::string& name, float x, float y) const {
  use();
  glUniform2f(glGetUniformLocation(mID, name.c_str()), x, y); 
}

// ------------------------------------------------------------------------

void RTVE::Shader::setVec3(const std::string& name, const glm::vec3& value) const {
  use();
  glUniform3fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]); 
}
void RTVE::Shader::setVec3(const std::string& name, float x, float y, float z) const {
  use();
  glUniform3f(glGetUniformLocation(mID, name.c_str()), x, y, z); 
}

// ------------------------------------------------------------------------

void RTVE::Shader::setVec4(const std::string& name, const glm::vec4& value) const {
  use();
  glUniform4fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]); 
}
void RTVE::Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
  use();
  glUniform4f(glGetUniformLocation(mID, name.c_str()), x, y, z, w); 
}

// ------------------------------------------------------------------------

void RTVE::Shader::setMat2(const std::string& name, const glm::mat2& mat) const { 
  use();
  glUniformMatrix2fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void RTVE::Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
  use();
  glUniformMatrix3fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void RTVE::Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
  use();
  glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// ------------------------------------------------------------------------

void RTVE::Shader::checkCompileErrors(GLuint pShader, const std::string& pType) {
  GLint success;
  GLchar infoLog[1024];
  if (pType != "PROGRAM") {
    glGetShaderiv(pShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pShader, 1024, NULL, infoLog);
      std::println("ERROR::SHADER_COMPILATION_ERROR of type: {0}\n{1}\n -- --------------------------------------------------- -- ", pType, infoLog);
    }
  }
  else {
    glGetProgramiv(pShader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(pShader, 1024, NULL, infoLog);
      std::println("ERROR::PROGRAM_LINKING_ERROR of type: {0}\n{1}\n -- --------------------------------------------------- -- ", pType, infoLog);
    }
  }
}

