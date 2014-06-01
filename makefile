include disable_implicit_rules.mk
include cproject.mk
include gmsl.mk

C_CPP_COMMON_COMPILE_FLAGS:= -O3 -g -Wall -Wextra -Wuninitialized -Winit-self -Wstrict-aliasing -Wfloat-equal -Wshadow -Wconversion -Werror -fpack-struct=4

C_COMPILE:=gcc -c
C_COMPILE_FLAGS:=-ansi -pedantic-errors

C_LINK:=g++
C_LINK_FLAGS:=-g -lm -lpthread

C_PROJECTS:=lpa_test

LPA_TEST_SRCFILES:=lpa_bcd.c lpa.c

.PHONY: all clean nuke FORCE

# make sure the all target is first target in the file
PROJECTS:=$(C_PROJECTS)
all: $(PROJECTS)

%.o: %.c
	@echo C Compiling $<
	@$(C_COMPILE) -MMD $(C_CPP_COMMON_COMPILE_FLAGS) $(C_COMPILE_FLAGS) -o $*.o $<

%: %.o
	@echo C Linking $<
	@$(C_LINK) -o $@ $^ $(C_LINK_FLAGS)

FORCE:

test:
	@echo C_PROJECTS=$(C_PROJECTS)
	@echo C_TARGETS=$(C_TARGETS)
	@echo C_SRCFILES=$(C_SRCFILES)
	@echo C_OBJFILES=$(C_OBJFILES)
	@echo PROJECTS=$(PROJECTS)
	@echo TARGETS=$(TARGETS)
	@echo SRCFILES=$(SRCFILES)
	@echo OBJFILES=$(OBJFILES)
	@echo LPA_TEST_OBJFILES=$(LPA_TEST_OBJFILES)

clean: FORCE
	rm -f $(OBJFILES)
	rm -f *.d

nuke: clean
	rm -f $(TARGETS)

# this will create targets
$(foreach project,$(C_PROJECTS),$(eval $(call C_PROJECT_template,$(project),$(call uc,$(project)))))

TARGETS:=$(C_TARGETS)
SRCFILES:=$(C_SRCFILES)
OBJFILES:=$(C_OBJFILES)

sinclude *.d
