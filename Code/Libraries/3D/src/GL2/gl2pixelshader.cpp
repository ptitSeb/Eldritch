#include "core.h"
#include "gl2pixelshader.h"
#include "idatastream.h"
#include "memorystream.h"
#ifdef HAVE_GLES
#include <stdio.h>
byte* ConvertShader( byte* org ); // defined in glvertexshader.cpp
#endif

GL2PixelShader::GL2PixelShader()
:	m_PixelShader( 0 )
{
}

GL2PixelShader::~GL2PixelShader()
{
	if( m_PixelShader != 0 )
	{
		glDeleteShader( m_PixelShader );
	}
}

void GL2PixelShader::Initialize( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

#ifdef HAVE_GLES
	int Length			= Stream.Size();
	byte* pBuffer		= new byte[ Length+1 ];
	Stream.Read( Length, pBuffer );
	pBuffer[ Length ]	= '\0';
	
	byte* Tmp	= ConvertShader( pBuffer );
	SafeDeleteArray(pBuffer);
	pBuffer		= Tmp;
	Length		= strlen((const char*)pBuffer);
#else
	const int Length	= Stream.Size();
	byte* pBuffer		= new byte[ Length ];
	Stream.Read( Length, pBuffer );
#endif

	m_PixelShader = glCreateShader( GL_FRAGMENT_SHADER );
	ASSERT( m_PixelShader != 0 );

	// Copy the GLSL source
	const GLsizei	NumStrings		= 1;
	const GLchar*	Strings[]		= { reinterpret_cast<GLchar*>( pBuffer ) };
	const GLint		StringLengths[]	= { Length };	// I don't trust this file to be null-terminated, so explicitly declare the length.
	glShaderSource( m_PixelShader, NumStrings, Strings, StringLengths );

	// Compile the shader
	glCompileShader( m_PixelShader );
	GLERRORCHECK;

	GLint CompileStatus;
	glGetShaderiv( m_PixelShader, GL_COMPILE_STATUS, &CompileStatus );

	if( CompileStatus != GL_TRUE )
	{
		GLint LogLength;
		glGetShaderiv( m_PixelShader, GL_INFO_LOG_LENGTH, &LogLength );
		Array<GLchar>	Log;
		Log.Resize( LogLength );
		glGetShaderInfoLog( m_PixelShader, LogLength, NULL, Log.GetData() );
		if( LogLength > 0 )
		{
			PRINTF( "GLSL fragment shader compile failed:\n" );
			PRINTF( Log.GetData() );
		}
		WARNDESC( "GLSL fragment shader compile failed" );
	}

	SafeDeleteArray( pBuffer );

	GLERRORCHECK;
}

/*virtual*/ bool GL2PixelShader::GetRegister( const HashedString& Parameter, uint& Register ) const
{
	Unused( Parameter );
	Unused( Register );

	// Shouldn't be called. Handled by GL2ShaderProgram.
	WARN;
	return false;
}
