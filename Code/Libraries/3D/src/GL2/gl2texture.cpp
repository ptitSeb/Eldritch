#include "core.h"
#include "gl2texture.h"
#include "idatastream.h"
#include "configmanager.h"
#include "mathcore.h"
#ifdef HAVE_GLES
#include "decompress.h"
#endif

GL2Texture::GL2Texture() : m_Texture(0) {}

GL2Texture::GL2Texture(GLuint Texture) : m_Texture(Texture) {}

GL2Texture::~GL2Texture() {
  if (m_Texture != 0) {
    glDeleteTextures(1, &m_Texture);
  }
}

/*virtual*/ void* GL2Texture::GetHandle() { return &m_Texture; }

#ifdef __amigaos4__
void BigEndian_ConvertRGBA(int Width, int Height, byte* texture)
{
  GLuint* dest = (GLuint*)texture;
  for (int i = 0; i < Height; i++) {
      for (int j = 0; j < Width; j++) {
        littleBigEndian(dest);
        ++dest;
      }
  }
}
#endif
#ifdef HAVE_GLES
// I don't trust the BGRA extensions on GLES
byte* GLES_ConvertBGRA2RGBA(int Width, int Height, byte* texture)
{
  byte* ret = (byte*) malloc(Width * Height * 4);
  GLuint tmp;
  GLuint* dest = (GLuint*)ret;
  for (int i = 0; i < Height; i++) {
      for (int j = 0; j < Width; j++) {
        tmp = *(const GLuint*)texture;
        #ifdef __amigaos4__
        *dest = ((tmp&0x00ffffff)<<8) | ((tmp&0xff000000)>>24);
        #else
        *dest = (tmp&0xff00ff00) | ((tmp&0x00ff0000)>>16) | ((tmp&0x000000ff)<<16);
        #endif
        texture += 4;
        dest++;
      }
  }
  return ret;
}
#endif

/*virtual*/ void GL2Texture::CreateTexture(byte* const ARGBImage) {
  XTRACE_FUNCTION;

  glGenTextures(1, &m_Texture);
  ASSERT(m_Texture != 0);

  glBindTexture(GL_TEXTURE_2D, m_Texture);

  const int MipLevels = CountMipLevels();

  STATICHASH(DebugMips);
  const bool DebugMips = ConfigManager::GetBool(sDebugMips);

  // Now fill the texture (and mipmaps)
  int Width = m_Width;
  int Height = m_Height;
  int MipWidth = 0;
  int MipHeight = 0;
  byte* ThisLevel = ARGBImage;
  byte* NextLevel = nullptr;
  for (int MipLevel = 0; MipLevel < MipLevels; ++MipLevel) {
    #ifdef HAVE_GLES
    byte* tmp = GLES_ConvertBGRA2RGBA(Width, Height, ThisLevel);
    glTexImage2D(GL_TEXTURE_2D, MipLevel, GL_RGBA, Width, Height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, tmp);
    free(tmp);
    #else
    glTexImage2D(GL_TEXTURE_2D, MipLevel, GL_RGBA8, Width, Height, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, ThisLevel);
    #endif

    if (DebugMips) {
      NextLevel =
          MakeDebugMip(MipLevel + 1, Width, Height, MipWidth, MipHeight);
    } else {
      NextLevel = MakeMip(Width, Height, MipWidth, MipHeight, ThisLevel);
    }

    SafeDeleteArray(ThisLevel);
    ThisLevel = NextLevel;
    Width = MipWidth;
    Height = MipHeight;
  }
  SafeDeleteArray(NextLevel);
}

// Mirrors DDCOLORKEY
struct SDDColorKey {
  uint m_ColorSpaceLow;
  uint m_ColorSpaceHigh;
};

// Mirrors DDSCAPS2
struct SDDSurfaceCaps {
  uint m_Caps[4];
};

// Mirrors DDPIXELFORMAT
struct SDDPixelFormat {
  uint m_Size;
  uint m_Flags;
  uint m_ID;
  uint m_BitCount;
  uint m_BitMasks[4];
};

// Mirrors DDSURFACEDESC2
struct SDDSurfaceFormat {
  uint m_Size;
  uint m_Flags;
  uint m_Height;
  uint m_Width;
  int m_Pitch;
  uint m_NumBackBuffers;
  uint m_NumMipMaps;
  uint m_AlphaBitDepth;
  uint m_Reserved;
  uint m_Surface;
  SDDColorKey m_DestOverlayColorKey;
  SDDColorKey m_DestBlitColorKey;
  SDDColorKey m_SrcOverlayColorKey;
  SDDColorKey m_SrcBlitColorKey;
  SDDPixelFormat m_PixelFormat;
  SDDSurfaceCaps m_Caps;
  uint m_TextureStage;
};

#define DDS_TAG 0x20534444   // 'DDS '
#define DXT1_TAG 0x31545844  // 'DXT1'
#define DXT3_TAG 0x33545844  // 'DXT3'
#define DXT5_TAG 0x35545844  // 'DXT5'

#ifdef HAVE_GLES
void *uncompressDXTc(GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
  // uncompress a DXTc image
  // get pixel size of uncompressed image => fixed RGBA
  int pixelsize = 4;
  // alloc memory
  void *pixels = malloc(((width+3)&~3)*((height+3)&~3)*pixelsize);
  // uncompress loop
  int blocksize;
  switch (format) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      blocksize = 8;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      blocksize = 16;
      break;
  }
  uintptr_t src = (uintptr_t) data;
  for (int y=0; y<height; y+=4) {
    for (int x=0; x<width; x+=4) {
      switch(format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
          DecompressBlockDXT1(x, y, width, (uint8_t*)src, (uint32_t*)pixels);
          break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
          DecompressBlockDXT3(x, y, width, (uint8_t*)src, (uint32_t*)pixels);
          break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
          DecompressBlockDXT5(x, y, width, (uint8_t*)src, (uint32_t*)pixels);
          break;
      }
      src+=blocksize;
    }
  }
  return pixels;
}
#endif //HAVE_GLES

/*virtual*/ void GL2Texture::CreateTextureFromDDS(const IDataStream& Stream) {
  XTRACE_FUNCTION;

#ifndef HAVE_GLES
  ASSERT(GLEW_EXT_texture_compression_s3tc);
#endif

  const uint DDSTag = Stream.ReadUInt32();
  DEVASSERT(DDSTag == DDS_TAG);
  Unused(DDSTag);

  SDDSurfaceFormat DDSFormat;
  Stream.Read(sizeof(SDDSurfaceFormat), &DDSFormat);
  #ifdef __amigaos4__
  littleBigEndian(&DDSFormat.m_Size);
  littleBigEndian(&DDSFormat.m_Flags);
  littleBigEndian(&DDSFormat.m_Height);
  littleBigEndian(&DDSFormat.m_Width);
  littleBigEndian(&DDSFormat.m_Pitch);
  littleBigEndian(&DDSFormat.m_NumBackBuffers);
  littleBigEndian(&DDSFormat.m_NumMipMaps);
  littleBigEndian(&DDSFormat.m_AlphaBitDepth);
  littleBigEndian(&DDSFormat.m_Reserved);
  littleBigEndian(&DDSFormat.m_Surface);
  littleBigEndian(&DDSFormat.m_DestOverlayColorKey.m_ColorSpaceLow);
  littleBigEndian(&DDSFormat.m_DestOverlayColorKey.m_ColorSpaceHigh);
  littleBigEndian(&DDSFormat.m_DestBlitColorKey.m_ColorSpaceLow);
  littleBigEndian(&DDSFormat.m_DestBlitColorKey.m_ColorSpaceHigh);
  littleBigEndian(&DDSFormat.m_SrcOverlayColorKey.m_ColorSpaceLow);
  littleBigEndian(&DDSFormat.m_SrcOverlayColorKey.m_ColorSpaceHigh);
  littleBigEndian(&DDSFormat.m_SrcBlitColorKey.m_ColorSpaceLow);
  littleBigEndian(&DDSFormat.m_SrcBlitColorKey.m_ColorSpaceHigh);
  littleBigEndian(&DDSFormat.m_PixelFormat.m_Size);
  littleBigEndian(&DDSFormat.m_PixelFormat.m_Flags);
  littleBigEndian(&DDSFormat.m_PixelFormat.m_ID);
  littleBigEndian(&DDSFormat.m_PixelFormat.m_BitCount);
  for (int ii=0; ii<4; ++ii)
    littleBigEndian(&DDSFormat.m_PixelFormat.m_BitMasks[ii]);
  for (int ii=0; ii<4; ++ii)
    littleBigEndian(&DDSFormat.m_Caps.m_Caps[ii]);
  littleBigEndian(&DDSFormat.m_TextureStage);
  #endif
  DEVASSERT(DDSFormat.m_Size == sizeof(SDDSurfaceFormat));

  // GL doesn't support DXT2 or DXT4 (premultiplied alpha) formats.
  DEVASSERT(DDSFormat.m_PixelFormat.m_ID == DXT1_TAG ||
            DDSFormat.m_PixelFormat.m_ID == DXT3_TAG ||
            DDSFormat.m_PixelFormat.m_ID == DXT5_TAG);

  m_Width = DDSFormat.m_Width;
  m_Height = DDSFormat.m_Height;

  GLenum GLFormat = 0;
  if (DDSFormat.m_PixelFormat.m_ID == DXT1_TAG) {
    GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
  } else if (DDSFormat.m_PixelFormat.m_ID == DXT3_TAG) {
    GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  } else if (DDSFormat.m_PixelFormat.m_ID == DXT5_TAG) {
    GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  }

  glGenTextures(1, &m_Texture);
  ASSERT(m_Texture != 0);

  glBindTexture(GL_TEXTURE_2D, m_Texture);

  GLsizei Width = m_Width;
  GLsizei Height = m_Height;
  const uint FormatBytes = (DDSFormat.m_PixelFormat.m_ID == DXT1_TAG ? 8 : 16);
  const uint MinSize = 1;
  const uint BlocksWide = Max(MinSize, m_Width >> 2);
  const uint BlocksHigh = Max(MinSize, m_Height >> 2);
  uint ReadBytes = BlocksWide * BlocksHigh * FormatBytes;
  Array<byte> ReadArray;
  ReadArray.SetDeflate(false);
  ReadArray.Reserve(ReadBytes);
  for (uint MipLevel = 0; MipLevel < DDSFormat.m_NumMipMaps; ++MipLevel) {
    ReadArray.Resize(ReadBytes);
    Stream.Read(ReadBytes, ReadArray.GetData());

#ifdef HAVE_GLES
    void* tmp = uncompressDXTc(Width, Height, GLFormat, ReadBytes, ReadArray.GetData());
    glTexImage2D(GL_TEXTURE_2D, MipLevel, GL_RGBA, Width, Height, 0,
                           GL_RGBA, GL_UNSIGNED_BYTE, tmp);
    free(tmp);
#else
    glCompressedTexImage2D(GL_TEXTURE_2D, MipLevel, GLFormat, Width, Height, 0,
                           ReadBytes, ReadArray.GetData());
#endif

    Width = Max(1, Width >> 1);
    Height = Max(1, Height >> 1);
    ReadBytes = Max(FormatBytes, ReadBytes >> 2);
  }
}