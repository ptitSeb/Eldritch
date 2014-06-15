#ifndef OPENALAUDIOSYSTEM_H
#define OPENALAUDIOSYSTEM_H

#include "audiosystemcommon.h"
#include "map.h"
#include "set.h"
#include "list.h"
#include "hashedstring.h"
#include "array.h"
#include "interpolator.h"

#include <AL/al.h>
#include <AL/alc.h>

class OpenALAudioSystem : public AudioSystemCommon {
 public:
  OpenALAudioSystem();
  virtual ~OpenALAudioSystem();

  virtual void Tick(float DeltaTime, bool GamePaused);
  virtual ISound* CreateSound(const SSoundInit& SoundInit);
  virtual ISoundInstance* Play(const SimpleString& DefinitionName,
                               const Vector& Location);
  virtual void SetReverbParams(const SimpleString& DefinitionName) const;
  virtual void ConditionalApplyReverb(
      ISoundInstance* const pSoundInstance) const;

  ALCdevice* GetALCDevice() const { return device; }

 private:
  ALCdevice* device;
  ALCcontext* context;

  Array<HashedString> m_ReverbCategories;  // Categories that has teh reverbs
                                           // (probably generally the same as
                                           // pause)
};

#endif  // OPENALAUDIOSYSTEM_H