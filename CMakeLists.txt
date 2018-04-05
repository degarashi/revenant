cmake_minimum_required(VERSION 2.8)
project(revenant)
include(aux.cmake)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++17 -Wall -Wextra -ftemplate-depth=1024")
add_definitions(-DUNIX=${UNIX})
add_definitions(-DSSE=${SSE})
# デバッグビルドの際はGLibのデバッグモードをONにする
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-D_GLIBCXX_DEBUG)
endif()
set(BKUP_UNITTEST ${without-unittest})
set(without-unittest YES)
add_subdirectory(beat)
set(without-unittest ${BKUP_UNITTEST})

# windows依存ファイル
set(DEPENDANT_SRC_WIN ./dir_depWin.cpp ./watch_depWin.cpp)
# unix依存ファイル
set(DEPENDANT_SRC_UNIX ./dir_depLinux.cpp ./watch_depLinux.cpp)
set(OS_DEPENDANT_SRC ${DEPENDANT_SRC_WIN} ${DEPENDANT_SRC_UNIX})

# OpenSL依存ファイル
set(DEPENDANT_OPENSL ./sound_depSL.cpp ./sound_depSL_error.cpp)
# OpenAL依存ファイル
set(DEPENDANT_OPENAL ./sound_depAL.cpp ./sound_depAL_error.cpp)
set(SOUND_DEPENDANT_SRC ${DEPENDANT_OPENSL} ${DEPENDANT_OPENAL})

if(${WITH_DEBUGGUI})
	add_definitions(-DDEBUGGUI_ENABLED)
	aux_source_directory(debug_gui LIBSRC_GUI)
	aux_source_directory(debug_gui/types LIBSRC_GUI)
endif()
# 依存ファイル以外のソースファイルを収集
aux_source_directory(. LIBSRC_MAIN)
aux_source_directory(drawtoken LIBSRC_MAIN)
aux_source_directory(util LIBSRC_MAIN)
aux_source_directory(lua_define LIBSRC_LUADEF)
aux_source_directory(lua LIBSRC_LUA)
aux_source_directory(glwrapper LIBSRC_GLWRAPPER)
aux_source_directory(imgui LIBSRC_MAIN)
list(REMOVE_ITEM LIBSRC_MAIN ${OS_DEPENDANT_SRC} ${SOUND_DEPENDANT_SRC})

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
set(LIBSRC_MAIN ${LIBSRC_MAIN} ${DEPSRC_FILE})

find_package(Boost 1.55 REQUIRED)
find_package(Freetype 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(Lua 5.3 REQUIRED)
include_directories(
	${CMAKE_SOURCE_DIR}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${VORBIS_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIRS}
	${LUA_INCLUDE_DIR}
	.
)

# Linux環境
add_definitions(-DINCLUDE_OPENGL_1_X
				-DINCLUDE_OPENGL_2_X
				-DINCLUDE_OPENGL_3_X
				-DINCLUDE_OPENGL_FRAMEBUFFER
				-DINCLUDE_OPENGL_SYNC
				-DINCLUDE_OPENGL_TIMER_QUERY)

function(MakeLib NAME SRC)
	add_library(${NAME} SHARED ${${SRC}})
	target_link_libraries(${NAME}
		${SDL2_IMAGE_LIBRARY}
		${SDL2_LIBRARY}
		${FREETYPE_LIBRARIES}
		${Boost_LIBRARIES}
		${VORBISFILE_LIBRARY}
		${VORBIS_LIBRARY}
		${OGG_LIBRARY}
		${OPENAL_LIBRARY}
		${LUA_LIBRARIES}
		GL
		beat
	)
	set_target_properties(${NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
	install(TARGETS ${NAME} LIBRARY DESTINATION lib)
endfunction()

MakeLib(revenant LIBSRC_MAIN)
MakeLib(revenant_gui LIBSRC_GUI)
MakeLib(revenant_luadef LIBSRC_LUADEF)
MakeLib(revenant_lua LIBSRC_LUA)
MakeLib(revenant_glwrapper LIBSRC_GLWRAPPER)

# ------ テストコード生成 ------
find_package(GTest REQUIRED)
set(TESTEXE_LIB
	revenant
	revenant_gui
	revenant_luadef
	revenant_lua
	revenant_glwrapper
	${GTEST_MAIN_LIBRARIES}
	${GTEST_LIBRARIES}
)
# ユニットテストはシングルファイル、そうでないものはサブディレクトリを使う
if(NOT without-unittest)
	enable_testing()
	include_directories(
		${GTEST_INCLUDE_DIRS}
	)
	aux_source_directory(tests/common COMMON)
	# testsディレクトリ以下のソースが対象
	aux_source_directory(tests TEST_SRC)
	foreach(SRC IN LISTS TEST_SRC)
		list(FIND COMMON ${SRC} OUTPUT)
		if(${OUTPUT} LESS 0)
			GetFileName(${SRC}, SRCNAME)
			AddTest(${SRCNAME} "${SRC};${COMMON}" "${TESTEXE_LIB}")
		endif()
	endforeach()
endif()

if(NOT without-test)
	add_subdirectory(tests/gameloop)
endif()
