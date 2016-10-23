#####################################################################################
#                    Project Twilight MUD Powered by ROM 2.4                        #
#####################################################################################

ECHOCMD = /bin/echo

#Colors Supported
L_RED   = \x1B[1;31m
L_GREEN = \x1B[1;32m
L_YELL  = \x1B[1;33m
L_BLUE  = \x1B[1;34m
L_CYAN  = \x1B[1;36m
L_WHITE = \x1B[1;37m
L_NRM   = \x1B[0;00m

#####################################################################################
# COMPILER SPECIFICS
COMPILER		= gcc

#####################################################################################
# FLAGS and LIBRARIES

# Old Profiler
# PROFILER      	= -ggdb3 #-Werror -pedantic -DASSERT
# New Profiler
PROFILER		= -O3 -g -ggdb  -Wall  -funroll-loops -fcaller-saves -ffast-math
NOCRYPT			=
C_FLAGS			=  -Wall -g $(PROFILER) -I$(INCLUDE) #$(NOCRYPT)
LIBRARY_FLAGS	=  $(PROFILER)

# Source Files
C_FILES	= ${wildcard *.c}

O_FILES = $(patsubst %.c, %.o, ${C_FILES})

O_TESTFILES = $(patsubst %.c, %.o, ${C_FILES})

#####################################################################################
# Engine
ENGINE	= project

# Directories
INCLUDE	= include

pt: $(O_FILES)
	rm -f $(ENGINE)
	rm -f $(ENGINE).exe
	$(COMPILER) $(LIBRARY_FLAGS) -o $(ENGINE) $(O_FILES) -lm
	# @if ! test -f $(BVERSION_FILE); then echo 0 > $(BVERSION_FILE); fi
	# @$(ECHOCMD) -e $$(($$(cat $(BVERSION_FILE)) + 1)) > $(BVERSION_FILE)
	# @$(ECHOCMD) -e "#ifndef __Version_H" > ./include/version.h
	# @$(ECHOCMD) -e "#define __Version_h" >> ./include/version.h
	# @$(ECHOCMD) -e '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> ./include/version.h
	# @$(ECHOCMD) -e "#endif" >> ./include/version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"
	

ptdev: $(O_TESTFILES)
	rm -f projectdev
	rm -f projectdev.exe
	$(COMPILER) $(C_FLAGS) -o projectdev $(O_TESTFILES) #-lm
	@if ! test -f $(BVERSION_FILE); then echo 0 > $(BVERSION_FILE); fi
	@echo $$(($$(cat $(BVERSION_FILE)) + 1)) > $(BVERSION_FILE)
	@echo "#ifndef __Version_H" > ./include/version.h
	@echo "#define __Version_h" >> ./include/version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> ./include/version.h
	@echo "#endif" >> ./include/version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

.c.o: twilight.h
	@$(ECHOCMD) -e "[$(L_WHITE)`date +%T`$(L_NRM)] Compiling $< ..."
	@$(COMPILER) -c $(C_FLAGS) $<

####################################################################################################
# clean out our folder of any/all un-necessary files allowing us to ensure our directory is clean.
clean:
	@rm -f *.o
	@$(ECHOCMD) -e "$(L_RED).o files removed from directory.$(L_NRM)"
####################################################################################################

####################################################################################################
# Wipes the directory clean of .o files and the executibles.  Helps ensure a clean directory.
wipe:
	@rm -f *.o $(ENGINE)
	@rm -f $(ENGINE).exe
	@rm -f projectdev
	@rm -f projectdev.exe
	@$(ECHOCMD) -e "$(L_RED)Project Twilight is wiped.$(L_NRM)"
####################################################################################################

####################################################################################################
# VERSION SUPPORT: allow us to control our revisions of the executable being built.
# name of our version file
BVERSION_FILE = ./include/.bversion
MVERSION_FILE = ./include/.mversion
MJVERSION_FILE = ./include/.mjversion

# Ensure we are built properly with the correct data
BUILD_DATE = $(shell date +'%Y%m%d')
BUILD_NUM = $(shell cat $(VERSION_FILE))

# define our version (within the compiler flags)
COMPILE_FLAGS += -DBUILD_DATE=$(BUILD_DATE) -DBUILD_NUM=$(BUILD_NUM)

#####################################################################################################
#Generate our build version file (generates a .version file, and a .h file)
.PHONY: bversion
bversion:
	@if ! test -f $(BVERSION_FILE); then echo 0 > $(BVERSION_FILE); fi
	@echo $$(($$(cat $(BVERSION_FILE)) + 1)) > $(BVERSION_FILE)
	@echo "#ifndef __Version_H" > ./include/version.h
	@echo "#define __Version_h" >> ./include/version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> ./include/version.h
	@echo "#endif" >> ./include/version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

#####################################################################################################
#Generate our minor version file (generates a .version file, and a .h file)
.PHONY: mversion
mversion:
	@if ! test -f $(MVERSION_FILE); then echo 0 > $(MVERSION_FILE); fi
	@echo $$(($$(cat $(MVERSION_FILE)) + 1)) > $(MVERSION_FILE)
	@echo "#ifndef __Version_H" > ./include/version.h
	@echo "#define __Version_h" >> ./include/version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> ./include/version.h
	@echo "#endif" >> ./include/version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

#####################################################################################################
#Generate our major version file (generates a .version file, and a .h file)
.PHONY: mjversion
mjversion:
	@if ! test -f $(MJVERSION_FILE); then echo 0 > $(MJVERSION_FILE); fi
	@echo $$(($$(cat $(MJVERSION_FILE)) + 1)) > $(MJVERSION_FILE)
	@echo "#ifndef __Version_H" > ./include/version.h
	@echo "#define __Version_h" >> ./include/version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> ./include/version.h
	@echo "#endif" >> ./include/version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

# valgrind -v --tool=memcheck --leak-check=full --show-reachable=yes --demangle=yes --db-attach=yes --num-callers=10 --track-origins=yes --track-fds=yes ../src/$(ENGINE) -v

#####################################################################################
# Grab our u-name for giggles; used later on to detect possible support
# for the game engine
UNAME = $(shell uname -s)
#####################################################################################
