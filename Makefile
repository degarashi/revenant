PWD			:= $(shell pwd)
WORK_DIR	:= /var/tmp/revenant
JOBS		:= 5

BUILD_TYPE			?= Debug
SSE					?= 2
CXX					?= g++
SOUND_API			?= openal
WITHOUT_UNITTEST	?= NO

OPT_BUILD_TYPE			= -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
OPT_SSE					= -DSSE=$(SSE)
OPT_COMPILER			= -DCMAKE_CXX_COMPILER=$(CXX)
OPT_SOUNDAPI			= -DSOUND_API=$(SOUND_API)
OPT_WITHOUT_UNITTEST	= -Dwithout-unittest=$(WITHOUT_UNITTEST)

define Options =
	-G 'CodeBlocks - Unix Makefiles'\
	-DUNIX=1\
	$(OPT_SOUNDAPI)\
	$(OPT_SSE)\
	$(OPT_BUILD_TYPE)\
	$(OPT_COMPILER)\
	$(OPT_WITHOUT_UNITTEST)
endef

CMake = mkdir -p $(WORK_DIR); cd $(WORK_DIR); cmake $(PWD) $(Options);
Make = cd $(WORK_DIR); make -j$(JOBS);
Clean = cd $(WORK_DIR); make clean; rm -f Makefile CMakeCache.txt;

.PHONY: cmake clean tags
all: $(WORK_DIR)/Makefile
	$(call Make)
	# リソースディレクトリへのシンボリックリンク
	ln -sf $(PWD)/resource $(WORK_DIR)/tests/gameloop
cmake:
	$(call CMake)
$(WORK_DIR)/Makefile:
	$(call CMake)
clean:
	$(call Clean)
tags:
	@ctags -R -f ./.git/ctags .
	@cscope -b -f ./.git/cscope.out
