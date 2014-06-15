#ifndef OPENALSOUNDINSTANCE_H
#define OPENALSOUNDINSTANCE_H

#include "soundinstancecommon.h"
#include "iaudiosystem.h"
#include "vector.h"

#include <AL/al.h>

class OpenALSoundInstance : public SoundInstanceCommon {
 public:
  OpenALSoundInstance(ISound* const pSound);
  virtual ~OpenALSoundInstance();

  virtual void Play() { SetPaused(false); }
  virtual void Stop();
  virtual void SetPaused(bool Paused);
  virtual void SetVolume(float Volume);
  virtual void SetPan(float Pan);
  virtual void SetPriority(ESoundPriority Priority) const;

  virtual bool IsPlaying() const;
  virtual bool IsFinished() const;
  virtual float GetTimeElapsed() const;

  ALuint source;
};

#endif  // OPENALSOUNDINSTANCE_H
