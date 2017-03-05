#include "core.h"
#include "gl2shaderprogram.h"
#include "ivertexshader.h"
#include "ipixelshader.h"
#include "ivertexdeclaration.h"
#include "simplestring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GL2ShaderProgram::GL2ShaderProgram()
    : m_VertexShader(nullptr),
      m_PixelShader(nullptr),
      m_ShaderProgram(0),
      m_UniformTable(),
      m_MaxCache(0),
      m_UniformCache(nullptr),
      m_UniformSize(nullptr) {}

GL2ShaderProgram::~GL2ShaderProgram() {
  if (m_ShaderProgram != 0) {
    ASSERT(m_VertexShader);
    ASSERT(m_PixelShader);

    GLuint VertexShader = *static_cast<GLuint*>(m_VertexShader->GetHandle());
    ASSERT(VertexShader != 0);
    glDetachShader(m_ShaderProgram, VertexShader);

    GLuint PixelShader = *static_cast<GLuint*>(m_PixelShader->GetHandle());
    ASSERT(PixelShader != 0);
    glDetachShader(m_ShaderProgram, PixelShader);

    glDeleteProgram(m_ShaderProgram);
  }
  if(m_UniformCache) {
    for (int i=0; i<m_MaxCache; i++) {
      free(m_UniformCache[i]);
    }
    free(m_UniformCache);
    free(m_UniformSize);
  }
}

/*virtual*/ void GL2ShaderProgram::Initialize(
    IVertexShader* const pVertexShader, IPixelShader* const pPixelShader,
    IVertexDeclaration* const pVertexDeclaration) {
  XTRACE_FUNCTION;

  m_VertexShader = pVertexShader;
  m_PixelShader = pPixelShader;

  ASSERT(m_VertexShader);
  ASSERT(m_PixelShader);

  m_ShaderProgram = glCreateProgram();
  ASSERT(m_ShaderProgram != 0);

  GLuint VertexShader = *static_cast<GLuint*>(m_VertexShader->GetHandle());
  ASSERT(VertexShader != 0);
  glAttachShader(m_ShaderProgram, VertexShader);

  GLuint PixelShader = *static_cast<GLuint*>(m_PixelShader->GetHandle());
  ASSERT(PixelShader != 0);
  glAttachShader(m_ShaderProgram, PixelShader);

  BindAttributes(pVertexDeclaration);

  glLinkProgram(m_ShaderProgram);
  GLERRORCHECK;

  GLint LinkStatus;
  glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &LinkStatus);

  if (LinkStatus != GL_TRUE) {
    GLint LogLength;
    glGetProgramiv(m_ShaderProgram, GL_INFO_LOG_LENGTH, &LogLength);
    Array<GLchar> Log;
    Log.Resize(LogLength);
    glGetProgramInfoLog(m_ShaderProgram, LogLength, nullptr, Log.GetData());
    if (LogLength > 0) {
      PRINTF("GLSL shader program link failed:\n");
      PRINTF(Log.GetData());
    }
    WARNDESC("GLSL shader program link failed");
  }

  BuildUniformTable();
  SetSamplerUniforms();
}

void GL2ShaderProgram::BindAttributes(
    IVertexDeclaration* const pVertexDeclaration) const {
  const uint VertexSignature = pVertexDeclaration->GetSignature();
  GLuint Index = 0;

#define BINDATTRIBUTE(SIGNATURE, VARIABLENAME)                  \
  if (SIGNATURE == (VertexSignature & SIGNATURE)) {             \
    glBindAttribLocation(m_ShaderProgram, Index, VARIABLENAME); \
    ++Index;                                                    \
  }

  BINDATTRIBUTE(VD_POSITIONS, "InPosition");
  BINDATTRIBUTE(VD_COLORS, "InColor");
#if USE_HDR
  BINDATTRIBUTE(VD_FLOATCOLORS, "InFloatColor1");
  BINDATTRIBUTE(VD_BASISCOLORS, "InFloatColor2");
  BINDATTRIBUTE(VD_BASISCOLORS, "InFloatColor3");
  BINDATTRIBUTE(VD_FLOATCOLORS_SM2, "InFloatColor1");
  BINDATTRIBUTE(VD_BASISCOLORS_SM2, "InFloatColor2");
  BINDATTRIBUTE(VD_BASISCOLORS_SM2, "InFloatColor3");
#endif
  BINDATTRIBUTE(VD_UVS, "InUV");
  BINDATTRIBUTE(VD_NORMALS, "InNormal");
  BINDATTRIBUTE(VD_TANGENTS, "InTangent");
  BINDATTRIBUTE(VD_BONEINDICES, "InBoneIndices");
  BINDATTRIBUTE(VD_BONEWEIGHTS, "InBoneWeights");

#undef SETSTREAM
}

void GL2ShaderProgram::BuildUniformTable() {
  GLint NumUniforms;
  glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &NumUniforms);

  m_MaxCache = 0;
  for (int UniformIndex = 0; UniformIndex < NumUniforms; ++UniformIndex) {
    const GLsizei BufferSize = 256;
    GLsizei Length;
    GLint Size;
    GLenum Type;
    GLchar UniformName[BufferSize];
    glGetActiveUniform(m_ShaderProgram, UniformIndex, BufferSize, &Length,
                       &Size, &Type, UniformName);
    const GLint Location = glGetUniformLocation(m_ShaderProgram, UniformName);
    if(m_MaxCache<Location) m_MaxCache = Location;
    // HACKHACK: Remove "[0]" from array names.
    for (GLchar* c = UniformName; *c != '\0'; ++c) {
      if (*c == '[') {
        *c = '\0';
        break;
      }
    }

    m_UniformTable.Insert(UniformName, Location);
  }
  m_MaxCache++;
  m_UniformCache = (void**)malloc(m_MaxCache*sizeof(void*));
  memset(m_UniformCache, 0, m_MaxCache*sizeof(void*));
  m_UniformSize = (int*)malloc(m_MaxCache*sizeof(int));
  memset(m_UniformSize, 0, m_MaxCache*sizeof(int));
}

// Because GLSL doesn't let me specify registers for samplers
// the way HLSL does, I bind them all to preset names here.
void GL2ShaderProgram::SetSamplerUniforms() {
  glUseProgram(m_ShaderProgram);

  for (uint SamplerStage = 0; SamplerStage < MAX_TEXTURE_STAGES;
       ++SamplerStage) {
    const HashedString TextureName =
        SimpleString::PrintF("Texture%d", SamplerStage);
    uint Register;
    if (GetRegister(TextureName, Register)) {
      glUniform1i(Register, SamplerStage);
    }
  }
}

/*virtual*/ bool GL2ShaderProgram::GetVertexShaderRegister(
    const HashedString& Parameter, uint& Register) const {
  return GetRegister(Parameter, Register);
}

/*virtual*/ bool GL2ShaderProgram::GetPixelShaderRegister(
    const HashedString& Parameter, uint& Register) const {
  return GetRegister(Parameter, Register);
}

bool GL2ShaderProgram::GetRegister(const HashedString& Parameter,
                                   uint& Register) const {
  Map<HashedString, uint>::Iterator UniformIter =
      m_UniformTable.Search(Parameter);
  if (UniformIter.IsValid()) {
    Register = UniformIter.GetValue();
    return true;
  } else {
    return false;
  }
}

bool GL2ShaderProgram::IsCached(uint Register, int Size, const void* Value) {
  if(m_UniformSize[Register]<Size) {
    // bigger size, purge the cache...
    free(m_UniformCache[Register]);
    m_UniformCache[Register] = nullptr;
  }
  if(m_UniformCache[Register] && memcmp(m_UniformCache[Register], Value, Size)==0)
    return true; // same value in cache
  
  if(m_UniformCache[Register]==nullptr) {
    m_UniformCache[Register] = malloc(Size);
    m_UniformSize[Register] = Size;
  } 
  
  memcpy(m_UniformCache[Register], Value, Size);
  
  return false;
}