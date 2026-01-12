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

GLuint RTVE::Shader::getID() {
  return mID;
}

void RTVE::Shader::printBufferOffsets() {
  // https://stackoverflow.com/questions/56512216/how-do-i-query-the-alignment-stride-for-an-ssbo-struct
  // Source - https://stackoverflow.com/a
  // Posted by Rabbid76, modified by community. See post 'Timeline' for change history
  // Retrieved 2026-01-11, License - CC BY-SA 4.0

  GLint no_of, ssbo_max_len, var_max_len;
  glGetProgramInterfaceiv(mID, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &no_of);
  glGetProgramInterfaceiv(mID, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);
  glGetProgramInterfaceiv(mID, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);
  std::vector<GLchar> name(100);
  for (int i_resource = 0; i_resource < no_of; ++i_resource) {
    // get name of the shader storage block
    GLsizei strLength;
    glGetProgramResourceName(mID, GL_SHADER_STORAGE_BLOCK, i_resource, ssbo_max_len, &strLength, name.data());

    // get resource index of the shader storage block
    GLint resInx = glGetProgramResourceIndex(mID, GL_SHADER_STORAGE_BLOCK, name.data());

    // get number of the buffer variables in the shader storage block
    GLenum prop = GL_NUM_ACTIVE_VARIABLES;
    GLint num_var;
    glGetProgramResourceiv(mID, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop, 1, nullptr, &num_var);

    // get resource indices of the buffer variables
    std::vector<GLint> vars(num_var);
    prop = GL_ACTIVE_VARIABLES;
    glGetProgramResourceiv(mID, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop, (GLsizei)vars.size(), nullptr, vars.data());
    std::println("vars.size(): {}, {}", vars.size(), vars);

    std::vector<GLint> offsets(num_var);
    std::vector<std::string> var_names(num_var);
    for (GLint i = 0; i < num_var; i++) {
      // get offset of buffer variable relative to SSBO
      GLenum prop = GL_OFFSET;
      glGetProgramResourceiv(mID, GL_BUFFER_VARIABLE, vars[i], 1, &prop, (GLsizei)offsets.size(), nullptr, &offsets[i]);
  
      // get name of buffer variable
      std::vector<GLchar>var_name(var_max_len);
      GLsizei strLength;
      glGetProgramResourceName(mID, GL_BUFFER_VARIABLE, vars[i], var_max_len, &strLength, var_name.data());
      var_names[i] = var_name.data();
      std::println("Name: {}, Offset: {}", var_names[i], offsets[i]);
    }
  }
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
void RTVE::Shader::setUInt(const std::string& name, uint value) const {
  use();
  glUniform1ui(glGetUniformLocation(mID, name.c_str()), value);
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

