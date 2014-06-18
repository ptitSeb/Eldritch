#ifndef OPENALSOUND_H
#define OPENALSOUND_H

#include "soundcommon.h"
#include <AL/al.h>

class IDataStream;
class PackStream;
struct SSoundInit;
#define EXT_OGG '\0ggo'
#define EXT_WAV '\0vaw'

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
