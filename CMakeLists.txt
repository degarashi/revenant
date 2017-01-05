cmake_minimum_required(VERSION 2.8)
project(revenant)
include(aux.cmake)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14 -Wall -Wextra -ftemplate-depth=1024")
add_definitions(-DUNIX)
add_definitions(-DSSE=${SSE})
# デバッグビルドの際はGLibのデバッグモードをONにする
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-D_GLIBCXX_DEBUG)
endif()

# windows依存ファイル
set(DEPENDANT_SRC_WIN ./dir_depWin.cpp)
# unix依存ファイル
set(DEPENDANT_SRC_UNIX ./dir_depLinux.cpp)
set(OS_DEPENDANT_SRC ${DEPENDANT_SRC_WIN} ${DEPENDANT_SRC_UNIX})

# OpenSL依存ファイル
set(DEPENDANT_OPENSL ./sound_depSL.cpp ./sound_depSL_error.cpp)
# OpenAL依存ファイル
set(DEPENDANT_OPENAL ./sound_depAL.cpp ./sound_depAL_error.cpp)
set(SOUND_DEPENDANT_SRC ${DEPENDANT_OPENSL} ${DEPENDANT_OPENAL})

# 依存ファイル以外のソースファイルを収集
aux_source_directory(. LIBSRC_FILE)
aux_source_directory(drawtoken LIBSRC_FILE)
aux_source_directory(util LIBSRC_FILE)
list(REMOVE_ITEM LIBSRC_FILE ${OS_DEPENDANT_SRC} ${SOUND_DEPENDANT_SRC})

# サウンド依存ファイルの設定
if(SOUND_API STREQUAL "openal")
	message(STATUS "using OpenAL...")
	find_package(OpenAL REQUIRED)
	set(DEPSRC_FILE ${DEPSRC_FILE} ${DEPENDANT_OPENAL})
	add_definitions(-DSOUND_HEADER="sound_depAL.hpp")
elseif(SOUND_API STREQUAL "opensl")
	message(STATUS "using OpenSL ES...")
	set(DEPSRC_FILE ${DEPSRC_FILE} ${DEPENDANT_OPENSL})
	add_definitions(-DSOUND_HEADER="sound_depSL.hpp")
else()
	message(FATAL_ERROR "unknown sound API")
endif()

# OS依存ファイルの設定
if(${WIN32})
	set(DEPSRC_FILE ${DEPSRC_FILE} ${DEPENDANT_SRC_WIN})
elseif(${UNIX})
	set(DEPSRC_FILE ${DEPSRC_FILE} ${DEPENDANT_SRC_UNIX})
else()
	message(FATAL_ERROR "couldn't detect OS type")
endif()
set(LIBSRC_FILE ${LIBSRC_FILE} ${DEPSRC_FILE})

find_package(Boost 1.55 REQUIRED)
find_package(Freetype 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(Lua 5.3 REQUIRED)
include_directories(
	.
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${VORBIS_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIRS}
	${LUA_INCLUDE_DIR}
)
# Linux環境
add_definitions(-DINCLUDE_OPENGL_1_X
				-DINCLUDE_OPENGL_2_X
				-DINCLUDE_OPENGL_3_X
				-DINCLUDE_OPENGL_FRAMEBUFFER
				-DINCLUDE_OPENGL_SYNC
				-DINCLUDE_OPENGL_TIMER_QUERY)
add_library(revenant SHARED ${LIBSRC_FILE})
target_link_libraries(revenant
	${SDL2_IMAGE_LIBRARY}
	${SDL2_LIBRARY}
	${FREETYPE_LIBRARIES}
	${Boost_LIBRARIES}
	${VORBISFILE_LIBRARY}
	${VORBIS_LIBRARY}
	${OGG_LIBRARY}
	${OPENAL_LIBRARY}
	liblua.a
	GL
)
set_target_properties(revenant PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS revenant LIBRARY DESTINATION lib)

if(NOT without-test)
	enable_testing()
	find_package(Threads REQUIRED)
	find_package(GTest REQUIRED)
	include_directories(
		${GTEST_INCLUDE_DIRS}
	)
	set(TESTEXE_LIB
		revenant
		${GTEST_MAIN_LIBRARIES}
		${GTEST_LIBRARIES}
	)

	set(EXPLICIT
		tests/luatest.cpp
		tests/lcv_classtest_value.cpp
		tests/lcv_test_value.cpp
	)
	# ちょっとしたテストコードはシングルファイル、そうでないものはサブディレクトリ
	# シングルファイルのテストコード: testsディレクトリ以下のソースが対象
	aux_source_directory(tests TEST_SRC)
	foreach(SRC IN LISTS TEST_SRC)
		list(FIND EXPLICIT ${SRC} OUTPUT)
		if(${OUTPUT} LESS 0)
			GetFileName(${SRC}, SRCNAME)
			AddTest(${SRCNAME} "${SRC};${EXPLICIT}" "${TESTEXE_LIB}")
		endif()
	endforeach()
	# サブディレクトリのテストコード
	add_subdirectory(tests/gameloop)
endif()
