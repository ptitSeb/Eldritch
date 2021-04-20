#include "core.h"
#include "gl2vertexbuffer.h"
#include "vector.h"
#include "vector2.h"
#include "vector4.h"
#ifdef __amigaos4__
#include "idatastream.h"
#endif
#ifdef NO_VBO
#include <cstdlib>
#include <cstring>
#endif

#ifdef HAVE_GLES
extern "C" {
	//void*	eglGetProcAddress(const char*); // cannot include EGL/egl.h, as it conflict with other headers...
	void*	SDL_GL_GetProcAddress(const char* proc);
}
#define eglGetProcAddress(aa) SDL_GL_GetProcAddress(aa)
static int						MapBufferInited = 0;
static PFNGLMAPBUFFEROESPROC	glMapBuffer = NULL;
static PFNGLUNMAPBUFFEROESPROC	glUnmapBuffer = NULL;
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
#endif

GL2VertexBuffer::GL2VertexBuffer()
:	m_RefCount( 0 )
,	m_NumVertices( 0 )
,	m_Dynamic( false )
,	m_PositionsVBO( 0 )
,	m_ColorsVBO( 0 )
,	m_FloatColorsVBO( 0 )
,	m_UVsVBO( 0 )
,	m_NormalsVBO( 0 )
//,	m_NormalsBVBO( 0 )
,	m_TangentsVBO( 0 )
,	m_BoneIndicesVBO( 0 )
,	m_BoneWeightsVBO( 0 )
{
#if !defined(NO_VBO) && defined(HAVE_GLES)
	if( !MapBufferInited )
	{
		glMapBuffer		= reinterpret_cast<PFNGLMAPBUFFEROESPROC>(eglGetProcAddress("glMapBufferOES"));		// Hmm...
		glUnmapBuffer	= reinterpret_cast<PFNGLUNMAPBUFFEROESPROC>(eglGetProcAddress("glUnmapBufferOES"));	// Hmm...
		MapBufferInited	= 1;
	}
#endif
}

GL2VertexBuffer::~GL2VertexBuffer()
{
	DeviceRelease();
}

int GL2VertexBuffer::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int GL2VertexBuffer::Release()
{
	DEVASSERT( m_RefCount > 0 );
	--m_RefCount;
	if( m_RefCount <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCount;
}

void GL2VertexBuffer::DeviceRelease()
{
#ifdef NO_VBO
#define SAFEDELETEBUFFER( USE ) if( m_##USE##VBO != 0 ) { free( m_##USE##VBO ); }
#else
#define SAFEDELETEBUFFER( USE ) if( m_##USE##VBO != 0 ) { glDeleteBuffers( 1, &m_##USE##VBO ); }
#endif
	SAFEDELETEBUFFER( Positions );
	SAFEDELETEBUFFER( Colors );
	SAFEDELETEBUFFER( FloatColors );
	SAFEDELETEBUFFER( UVs );
	SAFEDELETEBUFFER( Normals );
	//SAFEDELETEBUFFER( NormalsB );
	SAFEDELETEBUFFER( Tangents );
	SAFEDELETEBUFFER( BoneIndices );
	SAFEDELETEBUFFER( BoneWeights );
#undef SAFEDELETEBUFFER
}

void GL2VertexBuffer::DeviceReset()
{
}

void GL2VertexBuffer::RegisterDeviceResetCallback( const SDeviceResetCallback& Callback )
{
	Unused( Callback );
}

void GL2VertexBuffer::Init( const SInit& InitStruct )
{
	XTRACE_FUNCTION;

	m_NumVertices	= InitStruct.NumVertices;
	m_Dynamic		= InitStruct.Dynamic;

	const GLenum Usage = InitStruct.Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

#ifdef NO_VBO
#define CREATEBUFFER( USE, TYPE )												\
	if( InitStruct.USE )														\
	{																			\
		m_##USE##VBO = malloc( m_NumVertices * sizeof( TYPE ) );				\
		memcpy( m_##USE##VBO, InitStruct.USE, m_NumVertices * sizeof( TYPE ) );	\
		ASSERT( m_##USE##VBO != 0 );											\
	}
#else
#define CREATEBUFFER( USE, TYPE )																			\
	if( InitStruct.USE )																					\
	{																										\
		glGenBuffers( 1, &m_##USE##VBO );																	\
		ASSERT( m_##USE##VBO != 0 );																		\
		glBindBuffer( GL_ARRAY_BUFFER, m_##USE##VBO );														\
		glBufferData( GL_ARRAY_BUFFER, InitStruct.NumVertices * sizeof( TYPE ), InitStruct.USE, Usage );	\
	}
#endif

	CREATEBUFFER( Positions, Vector );
	CREATEBUFFER( Colors, uint );
	CREATEBUFFER( FloatColors, Vector4 );
	CREATEBUFFER( UVs, Vector2 );
	CREATEBUFFER( Normals, Vector );
	//CREATEBUFFER( NormalsB, Vector );
	CREATEBUFFER( Tangents, Vector4 );
	CREATEBUFFER( BoneIndices, SBoneData );
	CREATEBUFFER( BoneWeights, SBoneData );

#undef CREATEBUFFER

	GLERRORCHECK;
}

#ifdef NO_VBO
#define GETBUFFER( USE ) void* GL2VertexBuffer::Get##USE() { return m_##USE##VBO; }
#else
#define GETBUFFER( USE ) void* GL2VertexBuffer::Get##USE() { return &m_##USE##VBO; }
#endif

GETBUFFER( Positions )
GETBUFFER( Colors )
GETBUFFER( FloatColors )
GETBUFFER( UVs )
GETBUFFER( Normals )
//GETBUFFER( NormalsB )
GETBUFFER( Tangents )
GETBUFFER( BoneIndices )
GETBUFFER( BoneWeights )

#undef GETBUFFER

uint GL2VertexBuffer::GetNumVertices()
{
	return m_NumVertices;
}

void GL2VertexBuffer::SetNumVertices( uint NumVertices )
{
	m_NumVertices = NumVertices;
}

void* GL2VertexBuffer::Lock( IVertexBuffer::EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

#ifdef NO_VBO
	switch (VertexType) {
		case EVE_Positions:
			return m_PositionsVBO;
		case EVE_Colors:
			return m_ColorsVBO;
		case EVE_FloatColors:
			return m_FloatColorsVBO;
		case EVE_UVs:
			return m_UVsVBO;
		case EVE_Normals:
			return m_NormalsVBO;
		case EVE_Tangents:
			return m_TangentsVBO;
		case EVE_BoneIndices:
			return m_BoneIndicesVBO;
		case EVE_BoneWeights:
			return m_BoneWeightsVBO;
		default:
			WARN;
			return 0;
	}
#else
	const GLuint VBO = InternalGetVBO( VertexType );
	ASSERT( VBO != 0 );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );

	void* const pData = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
	ASSERT( pData );

	return pData;
#endif
}

void GL2VertexBuffer::Unlock( EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

#ifdef __amigaos4__
	if( VertexType==EVE_Colors )
	{
		GLuint* p = (GLuint*)m_ColorsVBO;
		for( int i=0; i<m_NumVertices; ++i, ++p )
		{
			littleBigEndian(p);
		}
	}
#endif
#ifndef NO_VBO
	const GLuint VBO = InternalGetVBO( VertexType );
	ASSERT( VBO != 0 );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );

	const GLboolean Success = glUnmapBuffer( GL_ARRAY_BUFFER );
	ASSERT( Success == GL_TRUE );
	Unused( Success );
#endif
}

#ifndef NO_VBO
GLuint GL2VertexBuffer::InternalGetVBO( EVertexElements VertexType )
{
	switch( VertexType )
	{
	case EVE_Positions:
		return m_PositionsVBO;
	case EVE_Colors:
		return m_ColorsVBO;
	case EVE_FloatColors:
		return m_FloatColorsVBO;
	case EVE_UVs:
		return m_UVsVBO;
	case EVE_Normals:
		return m_NormalsVBO;
	//case EVE_NormalsB:
	//	return m_NormalsBVBO;
	case EVE_Tangents:
		return m_TangentsVBO;
	case EVE_BoneIndices:
		return m_BoneIndicesVBO;
	case EVE_BoneWeights:
		return m_BoneWeightsVBO;
	default:
		WARN;
		return 0;
	}
}
#endif
