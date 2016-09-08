#
# I am not at all against seeing something that looks good and then using the concept, so here we go.
# The makefile is getting a make-over to try and help things out.

#####
## ## Compile our game engine for our new mud
## ## Force the muds development forward!
#####

####################################################################################################
# Grab our u-name for giggles; used later on to detect possible support
# for the game engine
UNAME = $(shell uname -s)
####################################################################################################

ECHOCMD = /bin/echo

#Colors Supported
L_RED   = \x1B[1;31m
L_GREEN = \x1B[1;32m
L_YELL  = \x1B[1;33m
L_BLUE  = \x1B[1;34m
L_CYAN  = \x1B[1;36m
L_WHITE = \x1B[1;37m
L_NRM   = \x1B[0;00m

CC      = gcc
PROF    = -ggdb3 #-Werror -pedantic -DASSERT
NOCRYPT =
C_FLAGS =  -Wall -g $(PROF) #$(NOCRYPT)
L_FLAGS =  $(PROF)

C_FILES	= ${wildcard *.c}

O_FILES = $(patsubst %.c, %.o, ${C_FILES})

O_TESTFILES = $(patsubst %.c, %.o, ${C_FILES})

pt: $(O_FILES)
	rm -f project
	rm -f project.exe
	$(CC) $(L_FLAGS) -o project $(O_FILES) -lm
	@if ! test -f $(BVERSION_FILE); then echo 0 > $(BVERSION_FILE); fi
	@echo $$(($$(cat $(BVERSION_FILE)) + 1)) > $(BVERSION_FILE)
	@echo "#ifndef __Version_H" > version.h
	@echo "#define __Version_h" >> version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> version.h
	@echo "#endif" >> version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"
	

ptdev: $(O_TESTFILES)
	rm -f projectdev
	rm -f projectdev.exe
	$(CC) $(C_FLAGS) -o projectdev $(O_TESTFILES) #-lm
	@if ! test -f $(BVERSION_FILE); then echo 0 > $(BVERSION_FILE); fi
	@echo $$(($$(cat $(BVERSION_FILE)) + 1)) > $(BVERSION_FILE)
	@echo "#ifndef __Version_H" > version.h
	@echo "#define __Version_h" >> version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> version.h
	@echo "#endif" >> version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

.c.o: twilight.h
	@$(ECHOCMD) -e "[$(L_WHITE)`date +%T`$(L_NRM)] Compiling $< ..."
	@$(CC) -c $(C_FLAGS) $<

####################################################################################################
# clean out our folder of any/all un-necessary files allowing us to ensure our directory is clean.
clean:
	@rm -f *.o
	@$(ECHOCMD) -e "$(L_RED).o files removed from directory.$(L_NRM)"
####################################################################################################

####################################################################################################
# Wipes the directory clean of .o files and the executibles.  Helps ensure a clean directory.
wipe:
	@rm -f *.o project
	@rm -f project.exe
	@rm -f projectdev
	@rm -f projectdev.exe
	@$(ECHOCMD) -e "$(L_RED)Project Twilight is wiped.$(L_NRM)"
####################################################################################################

####################################################################################################
# VERSION SUPPORT: allow us to control our revisions of the executable being built.
# name of our version file
BVERSION_FILE = .bversion
MVERSION_FILE = .mversion
MJVERSION_FILE = .mjversion

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
	@echo "#ifndef __Version_H" > version.h
	@echo "#define __Version_h" >> version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> version.h
	@echo "#endif" >> version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

#####################################################################################################
#Generate our minor version file (generates a .version file, and a .h file)
.PHONY: mversion
mversion:
	@if ! test -f $(MVERSION_FILE); then echo 0 > $(MVERSION_FILE); fi
	@echo $$(($$(cat $(MVERSION_FILE)) + 1)) > $(MVERSION_FILE)
	@echo "#ifndef __Version_H" > version.h
	@echo "#define __Version_h" >> version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> version.h
	@echo "#endif" >> version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

#####################################################################################################
#Generate our major version file (generates a .version file, and a .h file)
.PHONY: mjversion
mjversion:
	@if ! test -f $(MJVERSION_FILE); then echo 0 > $(MJVERSION_FILE); fi
	@echo $$(($$(cat $(MJVERSION_FILE)) + 1)) > $(MJVERSION_FILE)
	@echo "#ifndef __Version_H" > version.h
	@echo "#define __Version_h" >> version.h
	@echo '#define mudVersion "1.$(shell cat $(MJVERSION_FILE)).$(shell cat $(MVERSION_FILE)).$(shell cat $(BVERSION_FILE))"' >> version.h
	@echo "#endif" >> version.h
	@$(ECHOCMD) -e "$(L_WHITE)Revision control completed!$(L_NRM)"

#####################################################################################################

# valgrind -v --tool=memcheck --leak-check=full --show-reachable=yes --demangle=yes --db-attach=yes --num-callers=10 --track-origins=yes --track-fds=yes ../src/project -v
