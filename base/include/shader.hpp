#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <print>
#include <vector>

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace RTVE {
  class Shader {
  public:
    Shader() = default;
    Shader(const std::string& pVertexPath, const std::string& pFragPath);

    void load(const std::string& pVertexPath, const std::string& pFragPath);

    void use() const;
    GLuint getID();
    void printBufferOffsets();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setUInt(const std::string& name, uint value) const;
    void setFloat(const std::string& name, float value) const;

    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;

    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;

    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
  protected:
    void checkCompileErrors(GLuint pShader, const std::string& pType);

    GLuint mID;
  };
}

