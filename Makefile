LIB_NAME			:= revenant
COMMON_MAKE_PATH	:= lubee
WITHOUT_UNITTEST	?= NO
SSE					?= 2
SOUND_API			?= openal
MAKE_GDBINIT		:= NO
WITH_DEBUGGUI		?= YES
WITH_PROFILER		?= YES

OPT_SSE					= -DSSE=$(SSE)
OPT_SOUNDAPI			= -DSOUND_API=$(SOUND_API)
OPT_WITHOUT_UNITTEST	= -Dwithout-unittest=$(WITHOUT_UNITTEST)
OPT_DEBUGGUI			= -DWITH_DEBUGGUI=$(WITH_DEBUGGUI)
OPT_PROFILER			= -DWITH_PROFILER=$(WITH_PROFILER)

ADDITIONAL_CMAKE_OPTION		= $(OPT_SSE) $(OPT_SOUNDAPI) $(OPT_WITHOUT_UNITTEST) $(OPT_DEBUGGUI) $(OPT_PROFILER)
# リソースディレクトリへのシンボリックリンク
ADDITIONAL_CMD				= ln -sf $(PWD)/resource $(WORK_DIR)/testprog/gameloop; \
							  ln -sf $(PWD)/resource $(WORK_DIR);
include lubee/common.make

# 動作テスト
gameloop:
	-pkill gameloop
	$(WORK_DIR)/testprog/gameloop/gameloop
