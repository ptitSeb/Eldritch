cmake_minimum_required(VERSION 3.0)

include_directories(../Math/src)
include_directories(../Core/src)
include_directories(../GLEW/src)
include_directories(src/)

link_directories(${CMAKE_SOURCE_DIR}/Libraries/Core/)
link_directories(${CMAKE_SOURCE_DIR}/Libraries/Math/)

set(3d_src
	#src/D3D9/d3d9constanttable.cpp
	#src/D3D9/d3d9cubemap.cpp
	#src/D3D9/d3d9cuberendertarget.cpp
	#src/D3D9/d3d9indexbuffer.cpp
	#src/D3D9/d3d9pixelshader.cpp
	#src/D3D9/d3d9renderer.cpp
	#src/D3D9/d3d9rendertarget.cpp
	#src/D3D9/d3d9shaderprogram.cpp
	#src/D3D9/d3d9texture.cpp
	#src/D3D9/d3d9vertexbuffer.cpp
	#src/D3D9/d3d9vertexdeclaration.cpp
	#src/D3D9/d3d9vertexshader.cpp
	src/GL2/gl2cubemap.cpp
	src/GL2/gl2cuberendertarget.cpp
	src/GL2/gl2indexbuffer.cpp
	src/GL2/gl2pixelshader.cpp
	src/GL2/gl2renderer.cpp
	src/GL2/gl2rendertarget.cpp
	src/GL2/gl2shaderprogram.cpp
	src/GL2/gl2texture.cpp
	src/GL2/gl2vertexbuffer.cpp
	src/GL2/gl2vertexdeclaration.cpp
	src/GL2/gl2vertexshader.cpp
	src/SDPs/sdpbase.cpp
	src/3d.cpp
	src/animation.cpp
	src/animationstate.cpp
	src/animevent.cpp
	src/animeventfactory.cpp
	src/bonearray.cpp
	src/bucket.cpp
	src/cubemapcommon.cpp
	src/dynamicmeshmanager.cpp
	src/font.cpp
	src/fontmanager.cpp
	src/linebatcher.cpp
	src/material.cpp
	src/mesh.cpp
	src/meshfactory.cpp
	src/renderercommon.cpp
	src/renderstates.cpp
	src/sdpfactory.cpp
	src/sectorbound.cpp
	src/shaderdataprovider.cpp
	src/shadermanager.cpp
	src/surface.cpp
	src/texturecommon.cpp
	src/texturemanager.cpp
	src/tga.cpp
	src/vertexdeclarationmanager.cpp
	src/view.cpp
)

if(GLES)
	list(APPEND 3d_src
		src/GL2/decompress.cpp
	)
endif(GLES)

add_library(3D ${3d_src})

if(GLES)
	target_link_libraries(3D Core Math GLESv2)
else(GLES)
	target_link_libraries(3D Core Math GL GLEW)
endif(GLES)
