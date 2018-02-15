LIB_NAME			:= revenant
COMMON_MAKE_PATH	:= lubee
WITHOUT_UNITTEST	?= NO
SSE					?= 2
SOUND_API			?= openal
MAKE_GDBINIT		:= NO

OPT_SSE					= -DSSE=$(SSE)
OPT_SOUNDAPI			= -DSOUND_API=$(SOUND_API)
OPT_WITHOUT_UNITTEST	= -Dwithout-unittest=$(WITHOUT_UNITTEST)

ADDITIONAL_CMAKE_OPTION		= $(OPT_SSE) $(OPT_SOUNDAPI) $(OPT_WITHOUT_UNITTEST)
# リソースディレクトリへのシンボリックリンク
ADDITIONAL_CMD				= ln -sf $(PWD)/resource $(WORK_DIR)/tests/gameloop;
include lubee/common.make
