#include "core.h"
#include "openalsound.h"
#include "packstream.h"
#include "openalsoundinstance.h"
#include "openalaudiosystem.h"
#include "stb_vorbis.c"

#define EXT_OGG '\0ggo'
#define EXT_WAV '\0vaw'

OpenALSound::OpenALSound(
	IAudioSystem* const pSystem,
	const SSoundInit& SoundInit )
{
	ASSERT( pSystem );
	SetAudioSystem( pSystem );
	//OpenALAudioSystem* const pOALAudioSystem = static_cast<OpenALAudioSystem*>( pSystem );

	ASSERT( SoundInit.m_Filename != "" );

	//Get the file extension
	size_t ExtOffset = strlen( SoundInit.m_Filename.CStr() ) - 3;
	unsigned int Ext = *(unsigned int*)( SoundInit.m_Filename.CStr() + ExtOffset );

	if ( SoundInit.m_IsStream )
	{
		CreateStream( PackStream( SoundInit.m_Filename.CStr() ), SoundInit.m_IsLooping );
	}
	else
	{
		switch ( Ext )
		{
			case EXT_OGG:
				CreateSampleFromOGG( PackStream( SoundInit.m_Filename.CStr() ), SoundInit.m_IsLooping );
			break;
			case EXT_WAV:
				CreateSampleFromWAV( PackStream( SoundInit.m_Filename.CStr() ), SoundInit.m_IsLooping );
			break;
			default:
				PRINTF("Error: Unknown file extension for sample loading!\n");
		}
	}
}

OpenALSound::~OpenALSound()
{
	alDeleteBuffers(1, &buffer);
}

/*virtual*/ ISoundInstance* OpenALSound::CreateSoundInstance()
{
    //should try to get an existing source rather than create a new one?
	OpenALSoundInstance* const	pInstance	= new OpenALSoundInstance( this );
	ALuint src = pInstance->source;

	alSourcei(src, AL_BUFFER, buffer);
	alSource3f(src, AL_POSITION, 0, 0, 0);
	alSourcei(src, AL_LOOPING, GetLooping() ? AL_TRUE : AL_FALSE);
	alSourcePlay(src);

	return pInstance;
}

void OpenALSound::CreateSampleFromOGG( const IDataStream& Stream, bool Looping )
{
	int Length = Stream.Size();
	byte* pBuffer = new byte[ Length ];
	Stream.Read( Length, pBuffer );

	stb_vorbis* ogg = stb_vorbis_open_memory( pBuffer, Length, NULL, NULL );

	ASSERT( ogg );

	if (ogg)
	{
		stb_vorbis_info info = stb_vorbis_get_info( ogg );
		ALenum format = ( ( info.channels == 1 ) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);

		int const length_samples = ( stb_vorbis_stream_length_in_samples(ogg) * info.channels );
		ALshort* ogg_buffer = new ALshort[ length_samples ];
		alGenBuffers( 1, &buffer );
		stb_vorbis_get_samples_short_interleaved( ogg, info.channels, ogg_buffer, length_samples );
		alBufferData( buffer, format, ogg_buffer, ( length_samples * sizeof( ALshort ) ), info.sample_rate );
		stb_vorbis_close( ogg );
		SafeDeleteArray( pBuffer );
		SafeDeleteArray( ogg_buffer );
	}
}

void OpenALSound::CreateSampleFromWAV( const IDataStream& Stream, bool Looping )
{

}

void OpenALSound::CreateStream( const PackStream& Stream, bool Looping )
{
    //PRINTF("OpenALSound::CreateStream() STUB, doing nothing.\n");
    //~ const char*	pFilename = Stream.GetPhysicalFilename();
	//~ uint		Offset = Stream.GetSubfileOffset();
	//~ uint		Length = Stream.GetSubfileLength();
//~ 
	//~ FMOD_CREATESOUNDEXINFO ExInfo = { 0 };
	//~ ExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
	//~ ExInfo.length = Length;
	//~ ExInfo.fileoffset = Offset;
//~ 
	//~ FMOD_MODE Mode = FMOD_CREATESTREAM | FMOD_SOFTWARE;	// Use software so we can apply reverb
	//~ Mode |= Looping ? FMOD_LOOP_NORMAL : 0;
	//~ FMOD_RESULT Result = m_FMODSystem->createSound( pFilename, Mode, &ExInfo, &m_Sound );
	//~ ASSERT( Result == FMOD_OK );
	//~ Unused( Result );
}

float OpenALSound::GetLength() const
{
	PRINTF("OpenALSound::GetLength() STUB, returning 1.\n");
	return (float)1.0f;
}
