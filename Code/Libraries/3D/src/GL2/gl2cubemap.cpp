#include "core.h"
#include "gl2cubemap.h"
#include "idatastream.h"
#include "configmanager.h"
#include "mathcore.h"
#ifdef HAVE_GLES
#include "decompress.h"
#endif

GL2Cubemap::GL2Cubemap()
:	m_CubeTexture( 0 )
{
}

GL2Cubemap::GL2Cubemap( GLuint CubeTexture )
:	m_CubeTexture( CubeTexture )
{
}

GL2Cubemap::~GL2Cubemap()
{
	if( m_CubeTexture != 0 )
	{
		glDeleteTextures( 1, &m_CubeTexture );
	}
}

/*virtual*/ void* GL2Cubemap::GetHandle()
{
	return &m_CubeTexture;
}

#ifdef HAVE_GLES
// I don't trust the BGRA extensions on GLES
byte* GLES_ConvertBGRA2RGBA( int Width, int Height, const byte* texture )
{
	byte*	ret = (byte*)malloc( Width * Height * 4 );
	GLuint	tmp;
	GLuint*	dest = (GLuint*)ret;
	for( int i = 0; i < Height; i++ )
	{
		for( int j = 0; j < Width; j++ )
		{
			tmp = *(const GLuint*)texture;
#ifdef __amigaos4__
			*dest = ( tmp & 0x00ff00ff ) | ( ( tmp & 0xff000000 ) >> 16 ) | ( ( tmp & 0x0000ff00 ) << 16 );
#else
			*dest = ( tmp & 0xff00ff00 ) | ( ( tmp & 0x00ff0000 ) >> 16 ) | ( ( tmp & 0x000000ff ) << 16 );
#endif
			texture += 4;
			dest++;
		}
	}
	return ret;
}
void *uncompressDXTc( GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data )
{
	// uncompress a DXTc image
	// get pixel size of uncompressed image => fixed RGBA
	int pixelsize = 4;
	// alloc memory
	void *pixels = malloc( ( ( width + 3 ) & ~3 ) * ( ( height + 3 ) & ~3 ) * pixelsize );
	// uncompress loop
	int blocksize;
	switch( format )
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			blocksize = 8;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			blocksize = 16;
			break;
	}
	uintptr_t src = (uintptr_t)data;
	for( int y = 0; y < height; y += 4 )
	{
		for ( int x = 0; x < width; x += 4 )
		{
			uint8_t*	pSrc = (uint8_t*)src;
			uint32_t*	pPix = (uint32_t*)pixels;
			switch( format )
			{
				case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
				case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				DecompressBlockDXT1( x, y, width, ( format==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )?1:0, pSrc, pPix );
				break;
				case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
				DecompressBlockDXT3( x, y, width, pSrc, pPix );
				break;
				case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				DecompressBlockDXT5( x, y, width, pSrc, pPix );
				break;
			}
			src += blocksize;
		}
	}
	return pixels;
}
#endif //HAVE_GLES

// Maps to EImageFormat
static GLenum GLImageFormat[] =
{
	0,
	GL_RGBA8,
#ifdef HAVE_GLES
	0,
#else
	GL_RGBA32F,
#endif
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};

// Ordered X+, X-, Y+, Y-, Z+, Z-
// (or right, left, front, back, up, down)
static GLenum GLCubemapTarget[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,	// Swizzled
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,	// Swizzled
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,	// Swizzled
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,	// Swizzled
};

/*virtual*/ void GL2Cubemap::CreateCubemap( const SCubemapData& CubemapData )
{
	XTRACE_FUNCTION;

	GLGUARD_ACTIVETEXTURE;
	GLGUARD_BINDCUBEMAP;

	const STextureData&	FirstTextureData	= CubemapData.m_Textures[ 0 ];
#ifdef HAVE_GLES
	CHECKDESC( FirstTextureData.m_Format == 2, "Invalid GLES format GL_RGBA32F" );
#endif
	const uint		MipLevels			= FirstTextureData.m_MipChain.Size();
	const GLenum	Format				= GLImageFormat[ FirstTextureData.m_Format ];
	const GLint		Border				= 0;
	const bool		Compressed			= FirstTextureData.m_Format == EIF_DXT1 ||
										  FirstTextureData.m_Format == EIF_DXT3 ||
										  FirstTextureData.m_Format == EIF_DXT5;

	glGenTextures( 1, &m_CubeTexture );
	ASSERT( m_CubeTexture != 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_CubeTexture );
#ifndef HAVE_GLES
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, MipLevels - 1 );
#endif

	for( uint Side = 0; Side < 6; ++Side )
	{
		const STextureData&	TextureData	= CubemapData.m_Textures[ Side ];
		const GLenum		Target		= GLCubemapTarget[ Side ];

		// Now fill the texture (and mipmaps)
		for( uint MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
		{
			const TTextureMip&	Mip		= TextureData.m_MipChain[ MipLevel ];
			const uint			Width	= Max( static_cast<uint>( 1 ), TextureData.m_Width >> MipLevel );
			const uint			Height	= Max( static_cast<uint>( 1 ), TextureData.m_Height >> MipLevel );

			if( Compressed )
			{
#ifdef HAVE_GLES
				if( Format == GL_RGBA8 )
				{
					void* Temp = uncompressDXTc( Width, Height, Format, Mip.Size(), Mip.GetData() );
					glTexImage2D( GL_TEXTURE_2D, MipLevel, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Temp );
					free( Temp );
				}
				else
				{
					WARN;
				}
#else
				glCompressedTexImage2D( Target, MipLevel, Format, Width, Height, Border, Mip.Size(), Mip.GetData() );
#endif
			}
			else
			{
#ifdef HAVE_GLES
				if( Format == GL_RGBA8 )
				{
					byte* Temp = GLES_ConvertBGRA2RGBA( Width, Height, Mip.GetData() );
					glTexImage2D( GL_TEXTURE_2D, MipLevel, Format, Width, Height, Border, GL_RGBA, GL_UNSIGNED_BYTE, Temp );
					free( Temp );
				}
				else
				{
					WARN;
				}
#else
				if( Format == GL_RGBA8 )
				{
					glTexImage2D( Target, MipLevel, Format, Width, Height, Border, GL_BGRA, GL_UNSIGNED_BYTE, Mip.GetData() );
				}
				else if( Format == GL_RGBA32F )
				{
					glTexImage2D( Target, MipLevel, Format, Width, Height, Border, GL_RGBA, GL_FLOAT, Mip.GetData() );
				}
				else
				{
					WARN;
				}
#endif
			}
		}
	}
}
