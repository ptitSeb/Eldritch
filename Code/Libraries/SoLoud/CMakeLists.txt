cmake_minimum_required(VERSION 3.1)

set(soloud_src
	src/audiosource/wav/soloud_wav.cpp
	src/audiosource/wav/soloud_wavstream.cpp
	src/core/soloud.cpp
	src/core/soloud_audiosource.cpp
	src/core/soloud_bus.cpp
	src/core/soloud_core_3d.cpp
	src/core/soloud_core_basicops.cpp
	src/core/soloud_core_faderops.cpp
	src/core/soloud_core_filterops.cpp
	src/core/soloud_core_getters.cpp
	src/core/soloud_core_setters.cpp
	src/core/soloud_core_voicegroup.cpp
	src/core/soloud_core_voiceops.cpp
	src/core/soloud_fader.cpp
	src/core/soloud_fft.cpp
	src/core/soloud_fft_lut.cpp
	src/core/soloud_filter.cpp
	src/core/soloud_file.cpp
	src/core/soloud_thread.cpp
	src/filter/soloud_bassboostfilter.cpp
	src/filter/soloud_biquadresonantfilter.cpp
	src/filter/soloud_dcremovalfilter.cpp
	src/filter/soloud_echofilter.cpp
	src/filter/soloud_fftfilter.cpp
	src/filter/soloud_flangerfilter.cpp
	src/filter/soloud_lofifilter.cpp

	src/audiosource/wav/stb_vorbis.c
)
if(WITH_ALSA)
	list(APPEND soloud_src src/backend/alsa/soloud_alsa.cpp)
endif()
if(WITH_COREAUDIO)
	list(APPEND soloud_src src/backend/coreaudio/soloud_coreaudio.cpp)
endif()
if(WITH_NULL)
	list(APPEND soloud_src src/backend/null/soloud_null.cpp)
endif()
if(WITH_OPENAL)
	list(APPEND soloud_src src/backend/openal/soloud_openal.cpp src/backend/openal/soloud_openal_dll.c)
endif()
if(WITH_OPENSLES)
	list(APPEND soloud_src src/backend/opensles/soloud_opensles.cpp)
endif()
if(WITH_OSS)
	list(APPEND soloud_src src/backend/oss/soloud_oss.cpp)
endif()
if(WITH_PORTAUDIO)
	list(APPEND soloud_src src/backend/portaudio/soloud_portaudio.cpp src/backend/portaudio/soloud_portaudio_dll.c)
endif()
if(WITH_SDL)
	list(APPEND soloud_src src/backend/sdl/soloud_sdl.cpp src/backend/sdl/soloud_sdl_dll.c)
endif()
if(WITH_SDL_STATIC)
	list(APPEND soloud_src src/backend/sdl_static/soloud_sdl_static.cpp)
endif()
if(WITH_SDL2_STATIC)
	list(APPEND soloud_src src/backend/sdl2_static/soloud_sdl2_static.cpp)
endif()
if(WITH_WASAPI)
	list(APPEND soloud_src src/backend/wasapi/soloud_wasapi.cpp)
endif()
if(WITH_WINMM)
	list(APPEND soloud_src src/backend/winmm/soloud_winmm.cpp)
endif()
if(WITH_XAUDIO2)
	list(APPEND soloud_src src/backend/xaudio2/soloud_xaudio2.cpp)
endif()

include_directories(src)

add_library(SoLoud ${soloud_src})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(SoLoud PRIVATE Threads::Threads)

if(WITH_OPENAL OR WITH_PORTAUDIO OR WITH_SDL)
	target_link_libraries(SoLoud INTERFACE -ldl)
endif()
