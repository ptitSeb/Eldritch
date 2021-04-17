cmake_minimum_required(VERSION 3.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(zlib_src
	src/adler32.c
	src/compress.c
	src/crc32.c
	src/deflate.c
	src/gzio.c
	src/infback.c
	src/inffast.c
	src/inflate.c
	src/inftrees.c
	src/trees.c
	src/uncompr.c
	src/zutil.c
)

add_library(ZLib ${zlib_src})
