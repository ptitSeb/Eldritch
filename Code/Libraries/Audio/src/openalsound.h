#ifndef OPENALSOUND_H
#define OPENALSOUND_H

#include "soundcommon.h"
#include <AL/al.h>

class IDataStream;
class PackStream;
struct SSoundInit;
#ifdef __amigaos4__
#define EXT_OGG 'ogg\0'
#define EXT_WAV 'wav\0'
#else
#define EXT_OGG '\0ggo'
#define EXT_WAV '\0vaw'
#endif

class OpenALSound : public SoundCommon {
 public:
  OpenALSound(IAudioSystem* const pSystem, const SSoundInit& SoundInit);
  virtual ~OpenALSound();

  virtual ISoundInstance* CreateSoundInstance();

  virtual float GetLength() const;

 private:
  void CreateSampleFromOGG(const IDataStream& Stream, bool Looping);
  void CreateSampleFromWAV(const IDataStream& Stream, bool Looping);
  void CreateStream(const PackStream& Stream, bool Looping);

  ALuint buffer;
  ALenum GVorbisFormat;
};

#endif  // OPENALSOUND_H
