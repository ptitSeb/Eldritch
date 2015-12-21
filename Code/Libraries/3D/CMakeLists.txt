cmake_minimum_required(VERSION 2.6)

include_directories(../Math/src)
include_directories(../Core/src)
include_directories(../GLEW/src)
include_directories(src/)

link_directories(${CMAKE_SOURCE_DIR}/Libraries/Core/)
link_directories(${CMAKE_SOURCE_DIR}/Libraries/Math/)

set(3d_src
src/3d.cpp
src/3d.h
src/GL2/gl2.h
src/GL2/gl2indexbuffer.cpp
src/GL2/gl2indexbuffer.h
src/GL2/gl2pixelshader.cpp
src/GL2/gl2pixelshader.h
src/GL2/gl2renderer.cpp
src/GL2/gl2renderer.h
src/GL2/gl2rendertarget.cpp
src/GL2/gl2rendertarget.h
src/GL2/gl2shaderprogram.cpp
src/GL2/gl2shaderprogram.h
src/GL2/gl2texture.cpp
src/GL2/gl2texture.h
src/GL2/gl2vertexbuffer.cpp
src/GL2/gl2vertexbuffer.h
src/GL2/gl2vertexdeclaration.cpp
src/GL2/gl2vertexdeclaration.h
src/GL2/gl2vertexshader.cpp
src/GL2/gl2vertexshader.h
src/SDPs/sdpbase.cpp
src/SDPs/sdpbase.h
src/animation.cpp
src/animation.h
src/animationstate.cpp
src/animationstate.h
src/animevent.cpp
src/animevent.h
src/animeventfactory.cpp
src/animeventfactory.h
src/bonearray.cpp
src/bonearray.h
src/bucket.cpp
src/bucket.h
src/dynamicmeshmanager.cpp
src/dynamicmeshmanager.h
src/font.cpp
src/font.h
src/fontmanager.cpp
src/fontmanager.h
src/ibonemodifier.h
src/iindexbuffer.h
src/ipixelshader.h
src/irenderer.h
src/irendertarget.h
src/irradiancevolumes.cpp
src/irradiancevolumes.h
src/ishaderprogram.h
src/itexture.h
src/ivertexbuffer.h
src/ivertexdeclaration.h
src/ivertexshader.h
src/kdtree.cpp
src/kdtree.h
src/linebatcher.cpp
src/linebatcher.h
src/material.cpp
src/material.h
src/mesh.cpp
src/mesh.h
src/meshfactory.cpp
src/meshfactory.h
src/renderercommon.cpp
src/renderercommon.h
src/renderstates.cpp
src/renderstates.h
src/sdpfactory.cpp
src/sdpfactory.h
src/sdps.h
src/sectorbound.cpp
src/sectorbound.h
src/shaderdataprovider.cpp
src/shaderdataprovider.h
src/shadermanager.cpp
src/shadermanager.h
src/targetmanager.cpp
src/targetmanager.h
src/texturecommon.cpp
src/texturecommon.h
src/texturemanager.cpp
src/texturemanager.h
src/vertexdeclarationmanager.cpp
src/vertexdeclarationmanager.h
src/view.cpp
src/view.h
src/viewportpass.cpp
src/viewportpass.h
)

if(GLES)
 set(3d_src ${3d_src}
	src/GL2/decompress.h
	src/GL2/decompress.cpp
 )
endif(GLES)

add_library(3D ${3d_src})

if(GLES)
	target_link_libraries(3D Core Math)
else(GLES)
	target_link_libraries(3D Core Math GL GLEW)
endif(GLES)