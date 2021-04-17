#ifndef GL2_H
#define GL2_H

#ifdef HAVE_GLES
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#define glClearDepth		glClearDepthf
#define GL_RGBA8			GL_RGBA
#define GL_DEPTH24_STENCIL8	GL_DEPTH24_STENCIL8_OES
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT			0x80E1
#endif
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT		0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT	0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT	0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT	0x83F3

#if defined(PANDORA) || defined(__amigaos4__)
#define NO_VBO
// disabling use of VBO, as it's faster without on PANDORA...
// for AMIGAOS4, it doesn't seems to be available for now
#endif

#else
#include "GL/glew.h"
#endif
#ifdef WIN32
#include "GL/wglew.h"
#endif

#if BUILD_DEV
#define GLERRORCHECK do { const GLenum Error = glGetError(); if( Error == GL_NO_ERROR ) { break; } else { PRINTF( "GL error: 0x%04X\n", Error ); WARNDESC( "GL check" ); } } while(1)
#else
#define GLERRORCHECK DoNothing
#endif

#if BUILD_DEBUG
#define GLPARANOIDERRORCHECK GLERRORCHECK
#else
#define GLPARANOIDERRORCHECK DoNothing
#endif

// Guard structs that save old values and restore when they fall out of scope.
#define GLGUARD_BINDTEXTURE		SGLGuard_BindTexture	AutoGuard_BindTexture
#define GLGUARD_BINDCUBEMAP		SGLGuard_BindCubemap	AutoGuard_BindCubemap
#define GLGUARD_ACTIVETEXTURE	SGLGuard_ActiveTexture	AutoGuard_ActiveTexture

struct SGLGuard_BindTexture
{
	SGLGuard_BindTexture()
	:	m_FormerValue( 0 )
	{
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &m_FormerValue );
	}

	~SGLGuard_BindTexture()
	{
		glBindTexture( GL_TEXTURE_2D, m_FormerValue );
	}

	GLint m_FormerValue;
};

struct SGLGuard_BindCubemap
{
	SGLGuard_BindCubemap()
	:	m_FormerValue( 0 )
	{
		glGetIntegerv( GL_TEXTURE_BINDING_CUBE_MAP, &m_FormerValue );
	}

	~SGLGuard_BindCubemap()
	{
		glBindTexture( GL_TEXTURE_CUBE_MAP, m_FormerValue );
	}

	GLint m_FormerValue;
};

struct SGLGuard_ActiveTexture
{
	SGLGuard_ActiveTexture()
	:	m_FormerValue( 0 )
	{
		glGetIntegerv( GL_ACTIVE_TEXTURE, &m_FormerValue );
	}

	~SGLGuard_ActiveTexture()
	{
		glActiveTexture( m_FormerValue );
	}

	GLint m_FormerValue;
};

#endif // GL2_H
