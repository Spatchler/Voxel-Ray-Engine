#include "computeShader.hpp"

using namespace RTVE;

ComputeShader::ComputeShader(const std::string& pPath) {
  std::string computeCode;
  std::ifstream cShaderFile;

  cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    // Read files
    cShaderFile.open(pPath);

    std::stringstream cShaderStream;
    // read file's buffer contents into streams
    cShaderStream << cShaderFile.rdbuf();
    // close file handlers
    cShaderFile.close();
    // convert stream into string
    computeCode = cShaderStream.str();
  }
  catch (std::ifstream::failure& e) {
    std::println("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: {0}", e.what());
  }
  const char* cShaderCode = computeCode.c_str();
  // Compile
  uint compute;
  // compute shader
  compute = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(compute, 1, &cShaderCode, NULL);
  glCompileShader(compute);
  checkCompileErrors(compute, "COMPUTE");
  
  // shader Program
  mID = glCreateProgram();
  glAttachShader(mID, compute);
  glLinkProgram(mID);
  checkCompileErrors(mID, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(compute);
}

