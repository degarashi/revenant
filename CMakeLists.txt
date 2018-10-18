cmake_minimum_required(VERSION 2.8)
project(revenant)
include(aux.cmake)

set(CXX_VERSION 17)
include(lubee/cxx_options.cmake)
string(CONCAT CMAKE_CXX_FLAGS " -Wno-c99-extensions")
string(CONCAT CMAKE_CXX_FLAGS " -Wno-gnu-zero-variadic-macro-arguments")
string(CONCAT CMAKE_CXX_FLAGS " -Wno-gnu-anonymous-struct")
string(CONCAT CMAKE_CXX_FLAGS " -Wno-nested-anon-types")

add_definitions(-DUNIX=${UNIX})
add_definitions(-DSSE=${SSE})

set(BKUP_UNITTEST ${without-unittest})
set(without-unittest YES)
add_subdirectory(beat)
set(without-unittest ${BKUP_UNITTEST})

# windows依存ファイル
set(DEPENDANT_SRC_WIN src/dir_depWin.cpp src/watch_depWin.cpp)
# unix依存ファイル
set(DEPENDANT_SRC_UNIX src/dir_depLinux.cpp src/watch_depLinux.cpp)
set(OS_DEPENDANT_SRC ${DEPENDANT_SRC_WIN} ${DEPENDANT_SRC_UNIX})

# OpenSL依存ファイル
set(DEPENDANT_OPENSL src/sound_depSL.cpp src/sound_depSL_error.cpp)
# OpenAL依存ファイル
set(DEPENDANT_OPENAL src/sound_depAL.cpp src/sound_depAL_error.cpp)
set(SOUND_DEPENDANT_SRC ${DEPENDANT_OPENSL} ${DEPENDANT_OPENAL})

if(${WITH_DEBUGGUI})
	add_definitions(-DDEBUGGUI_ENABLED)
	aux_source_directory(src/debug_gui LIBSRC_GUI)
	aux_source_directory(src/debug_gui/types LIBSRC_GUI)
	aux_source_directory(src/debug_gui/gltf/v1 LIBSRC_GLTF_V1)
	aux_source_directory(src/debug_gui/gltf/v2 LIBSRC_GLTF_V2)
	aux_source_directory(src/debug_gui/dc LIBSRC_DC)
endif()
if(${WITH_PROFILER})
	add_definitions(-DPROFILER_ENABLED)
endif()
# 依存ファイル以外のソースファイルを収集
aux_source_directory(src LIBSRC_MAIN)
aux_source_directory(src/drawcmd LIBSRC_MAIN)
aux_source_directory(src/drawcmd/types LIBSRC_MAIN)
aux_source_directory(src/util LIBSRC_MAIN)
aux_source_directory(src/lua_define LIBSRC_LUADEF)
aux_source_directory(src/lua LIBSRC_LUA)
aux_source_directory(src/glwrapper LIBSRC_GLWRAPPER)
aux_source_directory(src/parse LIBSRC_GLXPARSE)
aux_source_directory(src/backup LIBSRC_BACKUP)
aux_source_directory(src/imgui LIBSRC_MAIN)
aux_source_directory(src/gltf LIBSRC_GLTF)
aux_source_directory(src/external LIBSRC_GLTF)
aux_source_directory(src/gltf/v1 LIBSRC_GLTF_V1)
aux_source_directory(src/gltf/v2 LIBSRC_GLTF_V2)
aux_source_directory(src/dc LIBSRC_DC)
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
	add_definitions(-DDIR_HEADER="dir_depWin.hpp")
	add_definitions(-DWATCH_HEADER="watch_depWin.hpp")
elseif(${UNIX})
	set(DEPSRC_FILE ${DEPSRC_FILE} ${DEPENDANT_SRC_UNIX})
	add_definitions(-DDIR_HEADER="dir_depLinux.hpp")
	add_definitions(-DWATCH_HEADER="watch_depLinux.hpp")
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
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
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
MakeLib(revenant_glxparse LIBSRC_GLXPARSE)
MakeLib(revenant_backup LIBSRC_BACKUP)
MakeLib(revenant_gltf LIBSRC_GLTF)
MakeLib(revenant_gltfv1 LIBSRC_GLTF_V1)
MakeLib(revenant_gltfv2 LIBSRC_GLTF_V2)
MakeLib(revenant_dc LIBSRC_DC)

# ------ テストコード生成 ------
find_package(GTest REQUIRED)
set(TESTEXE_LIB
	revenant
	revenant_gui
	revenant_luadef
	revenant_lua
	revenant_glwrapper
	revenant_glxparse
	revenant_backup
	revenant_gltfv2
	revenant_gltfv1
	revenant_gltf
	revenant_dc
	${GTEST_MAIN_LIBRARIES}
	${GTEST_LIBRARIES}
)
# ユニットテストはシングルファイル、そうでないものはサブディレクトリを使う
if(NOT without-unittest)
	enable_testing()
	include_directories(
		${GTEST_INCLUDE_DIRS}
	)
	aux_source_directory(src/tests/common COMMON)
	# testsディレクトリ以下のソースが対象
	aux_source_directory(src/tests TEST_SRC)
	foreach(SRC IN LISTS TEST_SRC)
		list(FIND COMMON ${SRC} OUTPUT)
		if(${OUTPUT} LESS 0)
			GetFileName(${SRC}, SRCNAME)
			AddTest(${SRCNAME} "${SRC};${COMMON}" "${TESTEXE_LIB}")
		endif()
	endforeach()
endif()

if(NOT without-test)
	add_subdirectory(src/testprog/sprite)
	add_subdirectory(src/testprog/cube)
	add_subdirectory(src/testprog/gltf1)
	add_subdirectory(src/testprog/gltf2)
endif()
