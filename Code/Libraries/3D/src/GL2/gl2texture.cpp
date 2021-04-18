#include "core.h"
#include "gl2texture.h"
#include "idatastream.h"
#include "configmanager.h"
#include "mathcore.h"
#ifdef HAVE_GLES
#include "decompress.h"
#endif

GL2Texture::GL2Texture()
:	m_Texture( 0 )
{
}

GL2Texture::GL2Texture( GLuint Texture )
:	m_Texture( Texture )
{
}

GL2Texture::~GL2Texture()
{
	if( m_Texture != 0 )
	{
		glDeleteTextures( 1, &m_Texture );
	}
}

/*virtual*/ void* GL2Texture::GetHandle()
{
	return &m_Texture;
}

#ifdef HAVE_GLES
// I don't trust the BGRA extensions on GLES
static inline byte* GLES_ConvertBGRA2RGBA( int Width, int Height, const byte* texture )
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
static inline void *uncompressDXTc( GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data )
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
#endif

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

/*virtual*/ void GL2Texture::CreateTexture( const STextureData& TextureData )
{
	XTRACE_FUNCTION;

	GLGUARD_ACTIVETEXTURE;
	GLGUARD_BINDTEXTURE;

#ifdef HAVE_GLES
	CHECKDESC( TextureData.m_Format == 2, "Invalid GLES format GL_RGBA32F" );
#endif
	const uint		MipLevels	= TextureData.m_MipChain.Size();
	const GLenum	Format		= GLImageFormat[ TextureData.m_Format ];
	const GLint		Border		= 0;
	const bool		Compressed	= TextureData.m_Format == EIF_DXT1 ||
								  TextureData.m_Format == EIF_DXT3 ||
								  TextureData.m_Format == EIF_DXT5;

	glGenTextures( 1, &m_Texture );
	ASSERT( m_Texture != 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_Texture );
#ifndef HAVE_GLES
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels - 1 );
#endif

	// Now fill the texture (and mipmaps)
	for( uint MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
	{
		const TTextureMip&	Mip		= TextureData.m_MipChain[ MipLevel ];
		const uint			MinSize	= 1;
		const uint			Width	= Max( MinSize, TextureData.m_Width >> MipLevel );
		const uint			Height	= Max( MinSize, TextureData.m_Height >> MipLevel );

		if( Compressed )
		{
#ifdef HAVE_GLES
			if( Format == GL_RGBA8 )
			{
				void* Temp = uncompressDXTc( Width, Height, Format, Mip.Size(), Mip.GetData() );
				glTexImage2D( GL_TEXTURE_2D, MipLevel, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Temp );
				free( Temp );
			}
			else
			{
				WARN;
			}
#else
			glCompressedTexImage2D( GL_TEXTURE_2D, MipLevel, Format, Width, Height, Border, Mip.Size(), Mip.GetData() );
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
				glTexImage2D( GL_TEXTURE_2D, MipLevel, Format, Width, Height, Border, GL_BGRA, GL_UNSIGNED_BYTE, Mip.GetData() );
			}
			else if( Format == GL_RGBA32F)
			{
				glTexImage2D( GL_TEXTURE_2D, MipLevel, Format, Width, Height, Border, GL_RGBA, GL_FLOAT, Mip.GetData() );
			}
			else
			{
				WARN;
			}
#endif
		}
	}

	GLERRORCHECK;
}

// Mirrors DDCOLORKEY
struct SDDColorKey
{
	uint	m_ColorSpaceLow;
	uint	m_ColorSpaceHigh;
};

// Mirrors DDSCAPS2
struct SDDSurfaceCaps
{
	uint	m_Caps[4];
};

// Mirrors DDPIXELFORMAT
struct SDDPixelFormat
{
	uint	m_Size;
	uint	m_Flags;
	uint	m_ID;
	uint	m_BitCount;
	uint	m_BitMasks[4];
};

// Mirrors DDSURFACEDESC2
struct SDDSurfaceFormat
{
	uint			m_Size;
	uint			m_Flags;
	uint			m_Height;
	uint			m_Width;
	int				m_Pitch;
	uint			m_NumBackBuffers;
	uint			m_NumMipMaps;
	uint			m_AlphaBitDepth;
	uint			m_Reserved;
	uint			m_Surface;	// This is a 32-bit pointer in the original format
	SDDColorKey		m_DestOverlayColorKey;
	SDDColorKey		m_DestBlitColorKey;
	SDDColorKey		m_SrcOverlayColorKey;
	SDDColorKey		m_SrcBlitColorKey;
	SDDPixelFormat	m_PixelFormat;
	SDDSurfaceCaps	m_Caps;
	uint			m_TextureStage;
};

#define DDS_TAG		0x20534444	// 'DDS '
#define DXT1_TAG	0x31545844	// 'DXT1'
#define DXT3_TAG	0x33545844	// 'DXT3'
#define DXT5_TAG	0x35545844	// 'DXT5'

//  DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS            0x00000001 
#define DDPF_FOURCC                 0x00000004 
#define DDPF_INDEXED                0x00000020 
#define DDPF_RGB                    0x00000040 

// NOTE: This is functionally identical to D3D9Texture::StaticLoadDDS, just without using DX headers and with GL-specific asserts.
/*static*/ void GL2Texture::StaticLoadDDS( const IDataStream& Stream, STextureData& OutTextureData )
{
	XTRACE_FUNCTION;

#ifndef HAVE_GLES
	ASSERT( GLEW_EXT_texture_compression_s3tc );
#endif

	const uint DDSTag = Stream.ReadUInt32();
	DEVASSERT( DDSTag == DDS_TAG );
	Unused( DDSTag );

	SDDSurfaceFormat DDSFormat;
	Stream.Read( sizeof( SDDSurfaceFormat ), &DDSFormat );
#ifdef __amigaos4__
	littleBigEndian( &DDSFormat.m_Size );
	littleBigEndian( &DDSFormat.m_Flags );
	littleBigEndian( &DDSFormat.m_Height );
	littleBigEndian( &DDSFormat.m_Width );
	littleBigEndian( &DDSFormat.m_Pitch );
	littleBigEndian( &DDSFormat.m_NumBackBuffers );
	littleBigEndian( &DDSFormat.m_NumMipMaps );
	littleBigEndian( &DDSFormat.m_AlphaBitDepth );
	littleBigEndian( &DDSFormat.m_Reserved );
	littleBigEndian( &DDSFormat.m_Surface );
	littleBigEndian( &DDSFormat.m_DestOverlayColorKey.m_ColorSpaceLow );
	littleBigEndian( &DDSFormat.m_DestOverlayColorKey.m_ColorSpaceHigh );
	littleBigEndian( &DDSFormat.m_DestBlitColorKey.m_ColorSpaceLow );
	littleBigEndian( &DDSFormat.m_DestBlitColorKey.m_ColorSpaceHigh );
	littleBigEndian( &DDSFormat.m_SrcOverlayColorKey.m_ColorSpaceLow );
	littleBigEndian( &DDSFormat.m_SrcOverlayColorKey.m_ColorSpaceHigh );
	littleBigEndian( &DDSFormat.m_SrcBlitColorKey.m_ColorSpaceLow );
	littleBigEndian( &DDSFormat.m_SrcBlitColorKey.m_ColorSpaceHigh );
	littleBigEndian( &DDSFormat.m_PixelFormat.m_Size );
	littleBigEndian( &DDSFormat.m_PixelFormat.m_Flags );
	littleBigEndian( &DDSFormat.m_PixelFormat.m_ID );
	littleBigEndian( &DDSFormat.m_PixelFormat.m_BitCount );
	for ( int ii = 0; ii < 4; ++ii )
	{
		littleBigEndian( &DDSFormat.m_PixelFormat.m_BitMasks[ ii ] );
	}
	for ( int ii = 0; ii < 4; ++ii )
	{
		littleBigEndian( &DDSFormat.m_Caps.m_Caps[ ii ] );
	}
	littleBigEndian( &DDSFormat.m_TextureStage );
	DEVASSERT( DDSFormat.m_Flags & DDPF_ALPHAPIXELS != 0 ); // TODO?
#endif
	DEVASSERT( DDSFormat.m_Size == sizeof( SDDSurfaceFormat ) );

	OutTextureData.m_Width	= DDSFormat.m_Width;
	OutTextureData.m_Height	= DDSFormat.m_Height;

	// Resize doesn't construct! So PushBack instead
	const uint MipLevels = Max( static_cast<uint>( 1 ), DDSFormat.m_NumMipMaps );
	for( uint MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
	{
		OutTextureData.m_MipChain.PushBack();
	}

	// GL doesn't support DXT2 or DXT4 (premultiplied alpha) formats.
	if( DDSFormat.m_PixelFormat.m_ID == DXT1_TAG )
	{
		OutTextureData.m_Format = EIF_DXT1;
	}
	else if( DDSFormat.m_PixelFormat.m_ID == DXT3_TAG )
	{
		OutTextureData.m_Format = EIF_DXT3;
	}
	else if( DDSFormat.m_PixelFormat.m_ID == DXT5_TAG )
	{
		OutTextureData.m_Format = EIF_DXT5;
	}
	DEVASSERT( OutTextureData.m_Format != EIF_Unknown );

	const uint	FormatBytes	= ( OutTextureData.m_Format == EIF_DXT1 ? 8 : 16 );
	const uint	MinSize		= 1;
	const uint	BlocksWide	= Max( MinSize, DDSFormat.m_Width >> 2 );
	const uint	BlocksHigh	= Max( MinSize, DDSFormat.m_Height >> 2 );

	for( uint MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
	{
		const uint	MipBlocksWide	= Max( MinSize, BlocksWide >> MipLevel );
		const uint	MipBlocksHigh	= Max( MinSize, BlocksHigh >> MipLevel );
		const uint	ReadBytes		= MipBlocksWide * MipBlocksHigh * FormatBytes;

		TTextureMip& Mip = OutTextureData.m_MipChain[ MipLevel ];
		Mip.Resize( ReadBytes );
		Stream.Read( ReadBytes, Mip.GetData() );
	}
}
