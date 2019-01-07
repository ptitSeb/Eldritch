#include "core.h"
#include "gl2vertexshader.h"
#include "idatastream.h"
#include "memorystream.h"
#ifdef HAVE_GLES
#include <stdio.h>

#ifdef PANDORA

const char* SimplePixelShader = "#version 100\n\
precision mediump float;\n\
uniform vec4 Gamma; // Used as float\n\
vec2 FixUV( vec2 UV )\n\
{\n\
  return vec2( UV.x, 1.0 - UV.y );\n\
}\n\
\n\
uniform sampler2D Texture0; // Diffuse\n\
uniform sampler2D Texture1; // Color grading LUT\n\
\n\
varying vec2  PassUV;\n\
\n\
void main()\n\
{\n\
  gl_FragColor = texture2D( Texture0, FixUV( PassUV ) );\n\
}";
#endif

byte* ConvertShader(byte* pBuffer)
{
//printf("Shader source:\n%s\n", pBuffer);
  // first change the version header, and add default precision
  byte* newptr;
  newptr=(byte*)strstr((char*)pBuffer, "#version");
  if (!newptr) 
    newptr = pBuffer;
  else {
    while(*newptr!=0x0a) newptr++;
  }
  const char* GLESHeader = "#version 100\nprecision mediump float;\n";
  auto Tmp = new byte[strlen((char*)newptr)+strlen(GLESHeader)+100];
  strcat(strcpy((char*)Tmp, GLESHeader), (char*)newptr);
  // now check to remove trailling "f" after float, as it's not supported too
  newptr = Tmp;
  int state = 0;
  while (*newptr!=0x00) {
    switch(state) {
      case 0:
        if ((*newptr >= '0') && (*newptr <= '9'))
          state = 1;  // integer part
        else if (*newptr == '.')
          state = 2;  // fractional part
        else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
          state = 0; // separator
        else 
          state = 3; // something else
        break;
      case 1: // integer part
        if ((*newptr >= '0') && (*newptr <= '9'))
          state = 1;  // integer part
        else if (*newptr == '.')
          state = 2;  // fractional part
        else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
          state = 0; // separator
        else  if ((*newptr == 'f' )) {
          // remove that f
          memmove(newptr, newptr+1, strlen((char*)newptr+1)+1);
          newptr--;
        } else
          state = 3;
          break;
      case 2: // fractionnal part
        if ((*newptr >= '0') && (*newptr <= '9'))
          state = 2;
        else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
          state = 0; // separator
        else  if ((*newptr == 'f' )) {
          // remove that f
          memmove(newptr, newptr+1, strlen((char*)newptr+1)+1);
          newptr--;
        } else
          state = 3;
          break;
      case 3:
        if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
          state = 0; // separator
        else      
          state = 3;
          break;
    }
    newptr++;
  }
//printf("New Shader source:\n%s\n", Tmp);
#ifdef PANDORA
  static int tested = 0;
  static int revision = 5;
  if (!tested) {
    tested = 1;
    FILE *f = fopen("/etc/powervr-esrev", "r");
    if (f) {
      fscanf(f, "%d", &revision);
      fclose(f);
      printf("Pandora Model detected = %d\n", revision);
    }
  }
  if ((revision==2) && (strstr(Tmp, "gl_FragColor = Calibrate( ColorGrade( texture2D( Texture0"))) {
    // only for CC model
//printf("Switched !!!\n");
    strcpy(Tmp, SimplePixelShader);
  }
#endif
  return Tmp;
}
#endif

GL2VertexShader::GL2VertexShader() : m_VertexShader(0) {}

GL2VertexShader::~GL2VertexShader() {
  if (m_VertexShader != 0) {
    glDeleteShader(m_VertexShader);
  }
}

void GL2VertexShader::Initialize(const IDataStream& Stream) {
  XTRACE_FUNCTION;

  /*const*/ int Length = Stream.Size();
  auto  pBuffer = new byte[Length+1];
  Stream.Read(Length, pBuffer);
  pBuffer[Length] = '\0';
  #ifdef HAVE_GLES
  auto Tmp = ConvertShader(pBuffer);
  SafeDeleteArray(pBuffer);
  pBuffer = Tmp;
  Length = strlen((char*)pBuffer);
  #endif

  m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
  ASSERT(m_VertexShader != 0);

  // Copy the GLSL source
  const GLsizei NumStrings = 1;
  const GLchar* Strings[] = {reinterpret_cast<GLchar*>(pBuffer)};
  const GLint StringLengths[] = {Length};  // I don't trust this file to be
                                           // null-terminated, so explicitly
                                           // declare the length.
  glShaderSource(m_VertexShader, NumStrings, Strings, StringLengths);

  // Compile the shader
  glCompileShader(m_VertexShader);
  GLERRORCHECK;

  GLint CompileStatus;
  glGetShaderiv(m_VertexShader, GL_COMPILE_STATUS, &CompileStatus);

  if (CompileStatus != GL_TRUE) {
    GLint LogLength;
    glGetShaderiv(m_VertexShader, GL_INFO_LOG_LENGTH, &LogLength);
    Array<GLchar> Log;
    Log.Resize(LogLength);
    glGetShaderInfoLog(m_VertexShader, LogLength, nullptr, Log.GetData());
    if (LogLength > 0) {
      PRINTF("GLSL vertex shader compile failed:\n");
      PRINTF(Log.GetData());
    }
    WARNDESC("GLSL vertex shader compile failed");
  }

  SafeDeleteArray(pBuffer);
}

/*virtual*/ bool GL2VertexShader::GetRegister(const HashedString& Parameter,
                                              uint& Register) const {
  Unused(Parameter);
  Unused(Register);

  // Shouldn't be called. Handled by GL2ShaderProgram.
  WARN;
  return false;
}