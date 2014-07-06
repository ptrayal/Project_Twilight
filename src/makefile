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

####################################################################################################
#Here we list our colours for use within our makefile, note that if we want to use our colours that
#we must define MK_COLOURS, else wise, we won't use our colours
#if your system does not accept colour-codes, then change MK_COLOURS To 0.
MK_COLOURS = 0

ifeq ($(MK_COLOURS), 1)
COLOUR_NORMAL = \e[0;00m
COLOUR_DRED = \e[0;31m
COLOUR_DGREEN = \e[0;32m
COLOUR_DBROWN = \e[0;33m
COLOUR_DBLUE = \e[0;34m
COLOUR_DPURPLE = \e[0;35m
COLOUR_DCYAN = \e[0;36m

COLOUR_LRED = \e[1;31m
    COLOUR_LGREEN = \e[1;32m
    COLOUR_LYELLOW = \e[1;33m
    COLOUR_LBLUE = \e[1;34m
COLOUR_LMAGENTA = \e[1;35m
    COLOUR_LGREY = \e[0;37m
    COLOUR_LWHITE = \e[1;37m
    COLOUR_LCYAN = \e[1;36m
else
COLOUR_NORMAL =
COLOUR_DRED =
	 COLOUR_DGREEN =
	 COLOUR_DBROWN =
	 COLOUR_DBLUE =
	 COLOUR_DPURPLE =
	 COLOUR_DCYAN =
	
	 COLOUR_LRED =
	 COLOUR_LGREEN =
	 COLOUR_LYELLOW =
	 COLOUR_LBLUE =
	 COLOUR_LMAGENTA =
	 COLOUR_LGREY =
	 COLOUR_LWHITE =
	 COLOUR_LCYAN =
endif
####################################################################################################

CC      = gcc
PROF    = -ggdb3 #-Werror -pedantic -DASSERT
NOCRYPT =
C_FLAGS =  -Wall -g $(PROF) #$(NOCRYPT)
L_FLAGS =  $(PROF)


O_FILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
			alias.o comm.o const.o db.o fight.o fight_old.o handler.o interp.o \
			lookup.o magic.o magic3.o mob_cmds.o mob_prog.o olc.o olc_act.o \
			olc_mpcode.o olc_save.o protocol.o quest.o recycle.o save.o social-edit.o \
			string.o subcmds.o tables.o update.o

O_TESTFILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
			alias.o comm.o const.o db.o fight.o fight_old.o handler.o interp.o \
			lookup.o magic.o magic3.o mob_cmds.o mob_prog.o olc.o olc_act.o \
			olc_mpcode.o olc_save.o protocol.o quest.o recycle.o save.o social-edit.o \
			string.o subcmds.o tables.o update.o

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
	@echo "$(COLOUR_LRED)Revision control completed!$(COLOUR_NORMAL)"
	

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
	@echo "$(COLOUR_LRED)Revision control completed!$(COLOUR_NORMAL)"

.c.o: twilight.h
	$(CC) -c $(C_FLAGS) $<

####################################################################################################
# clean out our folder of any/all un-necessary files allowing us to ensure our directory is clean.
clean:
	@rm -f *.o
	@echo "$(COLOUR_LRED).o files removed from directory.$(COLOUR_NORMAL)"
####################################################################################################

####################################################################################################
# Wipes the directory clean of .o files and the executibles.  Helps ensure a clean directory.
wipe:
	@rm -f *.o project
	@rm -f project.exe
	@rm -f projectdev
	@rm -f projectdev.exe
	@echo "$(COLOUR_LRED)Project Twilight is wiped.$(COLOUR_NORMAL)"
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
	@echo "$(COLOUR_LRED)Revision control completed!$(COLOUR_NORMAL)"

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
	@echo "$(COLOUR_LRED)Revision control completed!$(COLOUR_NORMAL)"

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
	@echo "$(COLOUR_LRED)Revision control completed!$(COLOUR_NORMAL)"

#####################################################################################################

# valgrind -v --tool=memcheck --leak-check=full --show-reachable=yes --demangle=yes --db-attach=yes --num-callers=10 --track-origins=yes --track-fds=yes ../src/project -v
