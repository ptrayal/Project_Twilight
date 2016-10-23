#Intermud-3
I3 = 1

EXE	= ../src/project
RM	= rm
# Old Options
CC	= ccache gcc
PROF	= -O3 -g -ggdb  -Wall  -Wno-unused-but-set-variable -funroll-loops -ffast-math
C_FLAGS	= -Dlinux $(PROF) -I$(INCLUDE) -DI3 -DI3ROM
L_FLAGS	= $(PROF) -lcrypt -lz

# Source Files
SRC_FILES := ${wildcard *.c}

# Include Files
INCLUDE	= include

ECHOCMD = /bin/echo

#Colours Supported
L_RED   = \x1B[1;31m
L_GREEN = \x1B[1;32m
L_YELL  = \x1B[1;33m
L_BLUE  = \x1B[1;34m
L_CYAN  = \x1B[1;36m
L_WHITE = \x1B[1;37m
L_NRM   = \x1B[0;00m

# Ok Messages
OK_S    = $(L_GREEN)OK$(L_NRM)
ERR_S   = with $(L_RED)ERRORS$(L_NRM)
WARN_S  = with $(L_YELL)WARNINGS$(L_NRM)

project     = ../src/project

# Object Files
OBJ_DIR	= obj
OBJ_FILES = $(patsubst %.c, $(OBJ_DIR)/%.o, ${SRC_FILES})
# OBJ_FILES:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))


../src/project: $(OBJ_FILES)
	@$(RM) -f files.txt
	@wc *.c >> files.txt
	@$(ECHOCMD) -e  "[$(L_WHITE)`date +%T`$(L_NRM)] Linking Executable..."
	@$(CC) -o $(EXE) $(OBJ_FILES) $(L_FLAGS) -lm
	@chmod 755 $(EXE)
	@$(ECHOCMD) -e  "[$(L_WHITE)`date +%T`$(L_NRM)] Compilation Complete"

# Increment .build and .version

	@if [ -f .build ]; then \
		expr `cat .build` + 1 > .build; \
	else \
	echo '1' > .build; \
	fi
	@if [ -f .version ]; then \
	expr `cat .version` + 1 > .version; \
	else \
	echo '10001' > .version; \
	fi
	@if [ `cat .build` -ge 101 ]; then \
	echo '1' > .build; \
	fi
	@echo '#ifndef BUILD_H' > build.h
	@echo '#define BUILD_H' >> build.h
	@echo "#define LINKCOUNT   `cat .build`" >> build.h
	@echo '#endif' >> build.h
	@echo '#ifndef VERSION_H' > version.h
	@echo '#define VERSION_H' >> version.h
	@echo "#define VERSION   `cat .version`" >> version.h
	@echo '#endif' >> version.h
	@$(ECHOCMD) -e "           -> $(L_NRM)Current Code Version: $(L_RED)$(shell echo "scale=2; `cat .version` / 10000" | bc)$(L_NRM). Build: $(L_RED)`cat .build`$(L_NRM).$(L_NRM)"
#	@$(ECHOCMD) -e "$(L_WHITE)Build number $(L_RED)`cat .build`$(L_WHITE), version $(L_RED)`cat .version` $(L_WHITE)set.$(L_NRM)"

	@$(ECHOCMD) -e  ""
	@$(ECHOCMD) -e  "If you wish to copy executable to live port type '$(L_WHITE)make copy$(L_NRM)'"
	@$(ECHOCMD) -e  "If you wish to copy executable to builder port type '$(L_WHITE)make dev$(L_NRM)'"
	@$(ECHOCMD) -e  ""


$(OBJ_DIR)/%.o: %.c
	@rm -f .temp.errors .temp.log
	@$(ECHOCMD) -e "[$(L_WHITE)`date +%T`$(L_NRM)] Compiling $< ..."
	@$(CC) -c -MMD -MP $(C_FLAGS) $< -o $@ 2> .temp.log || touch .temp.errors
	@$(ECHOCMD) -en "[$(L_WHITE)`date +%T`$(L_NRM)] $(L_NRM)$<$(L_NRM) compiled "
	@if test -e .temp.errors; then $(ECHOCMD) -e  "$(ERR_S)" && cat .temp.log; exit 2; elif test -s .temp.log; then $(ECHOCMD) -e "$(WARN_S)" && cat .temp.log; else $(ECHOCMD) -e  "$(OK_S)"; fi;
	@rm -f .temp.errors .temp.log


copy:
	@$(ECHOCMD) -e  ""
	@$(ECHOCMD) -e  "[$(L_WHITE)`date +%T`$(L_NRM)] Copying executable to live port"
	@$(ECHOCMD) -e  ""
	@cp -f project ../bin/

dev:
	@$(ECHOCMD) -e  ""
	@$(ECHOCMD) -e  "[$(L_WHITE)`date +%T`$(L_NRM)] Copying executable to builder port"
	@$(ECHOCMD) -e  ""
	@cp -f project ../../aadaria.test/src/

clean: 
	@$(RM) -fr $(OBJ_FILES) $(EXE)
	@$(ECHOCMD) -e  "[$(L_WHITE)`date +%T`$(L_NRM)] Done cleaning..."
	@make 
	@make copy
