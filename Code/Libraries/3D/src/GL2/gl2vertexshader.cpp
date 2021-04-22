#include "core.h"
#include "gl2vertexshader.h"
#include "idatastream.h"
#include "memorystream.h"
#ifdef HAVE_GLES
#include <cstdio>

#ifdef PANDORA
const char* SimplePixelShader =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform vec4 Gamma; // Used as float\n"
	"vec2 FixUV( vec2 UV )\n"
	"{\n"
	"\treturn vec2( UV.x, 1.0 - UV.y );\n"
	"}\n"
	"\n"
	"uniform sampler2D Texture0; // Diffuse\n"
	"uniform sampler2D Texture1; // Color grading LUT\n"
	"\n"
	"varying vec2      PassUV;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = texture2D( Texture0, FixUV( PassUV ) );\n"
	"}";
#endif

byte* ConvertShader( byte* pBuffer )
{
	// printf( "\e[1mShader source:\e[m\n%s\n", (char*)pBuffer );
	// first change the version header, and add default precision
	char* newptr;
	newptr = strstr( (char*)pBuffer, "#version" );
	if( !newptr )
	{
		newptr = (char*)pBuffer;
	}
	else
	{
		while( *newptr != 0x0a ) ++newptr;
	}
	const char*	GLESHeader = "#version 100\nprecision mediump float;\n";
	byte*		Tmp = new byte[ strlen( (char*)newptr ) + strlen( GLESHeader ) + 200 ];
	strcat( strcpy( (char*)Tmp, GLESHeader ), newptr );
	// now check to remove trailing "f" after float, as it's not supported too
	newptr = (char*)Tmp;
	int state = 0;
	while (*newptr!=0x00) {
		switch(state) {
		case 0:
			if ((*newptr >= '0') && (*newptr <= '9'))
				state = 1; // integer part
			else if (*newptr == '.')
				state = 2; // <NaN> '.'
			else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
				state = 0; // separator
			else
				state = 4; // something else
			break;
		case 1: // integer part
			if ((*newptr >= '0') && (*newptr <= '9'))
				state = 1; // integer part
			else if (*newptr == '.')
				state = 3; // fractional part
			else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
				state = 0; // separator
			else if ((*newptr == 'f' ))
				*newptr = '.';
			else
			state = 4;
			break;
		case 2: // <NaN> '.'
			if ((*newptr >= '0') && (*newptr <= '9'))
				state = 3;
			else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
				state = 0; // separator
			else
				state = 4;
			break;
		case 3: // fractionnal part
			if ((*newptr >= '0') && (*newptr <= '9'))
				state = 3;
			else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
				state = 0; // separator
			else if ((*newptr == 'f' )) {
				// remove that f
				memmove(newptr, newptr+1, strlen((char*)newptr+1)+1);
				newptr--;
			} else
				state = 4;
			break;
		case 4:
			if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
				state = 0; // separator
			else
				state = 4;
			break;
		}
		newptr++;
	}

	int openbr = 0;
	newptr = (char*)Tmp;
	//if (strlen(Tmp) < 10) return Tmp;
	while (newptr[10]) {
		if (*newptr == '{') {
			++openbr;
		} else if (*newptr == '}') {
			--openbr;
		} else if (!openbr && (!strncmp(newptr, "float ", 6) || !strncmp(newptr, "float\t", 6))) {
			char *eqsign = strchr(newptr, '=');
			char *semcol = strchr(newptr, ';');
			char *newlin = strchr(newptr, '\n');
			if (!eqsign || !semcol) break;
			if (semcol < eqsign) { ++newptr; continue; }
			if (newlin < semcol) { ++newptr; continue; }
			*eqsign = '(';
			*semcol = ')';
			memmove(newptr + 8, newptr + 6, strlen(newptr + 6) + 1);
			newptr[0] = '#';
			newptr[1] = 'd';
			newptr[2] = 'e';
			newptr[3] = 'f';
			newptr[4] = 'i';
			newptr[5] = 'n';
			newptr[6] = 'e';
			newptr[7] = ' ';
			newptr = semcol;
		}
		++newptr;
	}

	// printf( "\e[31;1mNew Shader source:\e[m\n%s\n", (char*)Tmp );
#ifdef PANDORA
	static int tested = 0;
	static int revision = 5;
	if( !tested )
	{
		tested = 1;
		FILE *f = fopen( "/etc/powervr-esrev", "r" );
		if( f )
		{
			fscanf( f, "%d", &revision );
			fclose( f );
			printf( "Pandora Model detected = %d\n", revision );
		}
	}
	if ( ( revision == 2 ) && strstr( (const char*)Tmp, "gl_FragColor = Calibrate( ColorGrade( texture2D( Texture0" ) )
	{
		// only for CC model
		//printf("Switched !!!\n");
		strcpy( (char*)Tmp, SimplePixelShader );
	}
	#endif
	return Tmp;
}
#endif

GL2VertexShader::GL2VertexShader()
:	m_VertexShader( 0 )
{
}

GL2VertexShader::~GL2VertexShader()
{
	if( m_VertexShader != 0 )
	{
		glDeleteShader( m_VertexShader );
	}
}

void GL2VertexShader::Initialize( const IDataStream& Stream )
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

	m_VertexShader = glCreateShader( GL_VERTEX_SHADER );
	ASSERT( m_VertexShader != 0 );

	// Copy the GLSL source
	const GLsizei	NumStrings		= 1;
	const GLchar*	Strings[]		= { reinterpret_cast<GLchar*>( pBuffer ) };
	const GLint		StringLengths[]	= { Length };	// I don't trust this file to be null-terminated, so explicitly declare the length.
	glShaderSource( m_VertexShader, NumStrings, Strings, StringLengths );

	// Compile the shader
	glCompileShader( m_VertexShader );
	GLERRORCHECK;

	GLint CompileStatus;
	glGetShaderiv( m_VertexShader, GL_COMPILE_STATUS, &CompileStatus );

	if( CompileStatus != GL_TRUE )
	{
		GLint LogLength;
		glGetShaderiv( m_VertexShader, GL_INFO_LOG_LENGTH, &LogLength );
		Array<GLchar>	Log;
		Log.Resize( LogLength );
		glGetShaderInfoLog( m_VertexShader, LogLength, NULL, Log.GetData() );
		if( LogLength > 0 )
		{
			PRINTF( "GLSL vertex shader compile failed:\n" );
			PRINTF( Log.GetData() );
		}
		WARNDESC( "GLSL vertex shader compile failed" );
	}

	SafeDeleteArray( pBuffer );

	GLERRORCHECK;
}

/*virtual*/ bool GL2VertexShader::GetRegister( const HashedString& Parameter, uint& Register ) const
{
	Unused( Parameter );
	Unused( Register );

	// Shouldn't be called. Handled by GL2ShaderProgram.
	WARN;
	return false;
}
