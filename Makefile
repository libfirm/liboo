-include config.mak

LIBFIRM_CPPFLAGS ?= `pkg-config --cflags libfirm`
LIBFIRM_LFLAGS   ?= `pkg-config --libs libfirm`
INSTALL ?= install
DLLEXT ?= .so
CC ?= gcc
AR ?= ar

guessed_target := $(shell $(CC) -dumpmachine)
target         ?= $(guessed_target)

BUILDDIR=build
RUNTIME_BUILDDIR=$(BUILDDIR)/$(target)
GOAL = $(BUILDDIR)/liboo$(DLLEXT)
GOAL_RT_SHARED = $(RUNTIME_BUILDDIR)/liboo_rt$(DLLEXT)
GOAL_RT_STATIC = $(RUNTIME_BUILDDIR)/liboo_rt.a
CPPFLAGS = -I. -I./include/ $(LIBFIRM_CPPFLAGS) $(LIBUNWIND_CPPFLAGS)
CFLAGS = -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Werror -O0 -g3 -std=c99 -pedantic
# disabled the following warnings for now. They fail on OS/X Snow Leopard:
# the first one gives false positives because of system headers, the later one
# doesn't exist in the old gcc there
#CFLAGS += -Wunreachable-code -Wlogical-op
LFLAGS = 
PIC_FLAGS = -fpic
SOURCES = $(wildcard src-cpp/*.c) $(wildcard src-cpp/adt/*.c)
SOURCES_RT = $(wildcard src-cpp/rt/*.c)
DEPS = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES))))
DEPS_RT = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES_RT))))
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
OBJECTS_RT_SHARED = $(addprefix $(RUNTIME_BUILDDIR)/shared/, $(addsuffix .o, $(basename $(SOURCES_RT))))
OBJECTS_RT_STATIC = $(addprefix $(RUNTIME_BUILDDIR)/static/, $(addsuffix .o, $(basename $(SOURCES_RT))))

Q ?= @

.PHONY: all runtime clean

all: $(GOAL) runtime

runtime: $(GOAL_RT_SHARED) $(GOAL_RT_STATIC)

# Make sure our build-directories are created
UNUSED := $(shell mkdir -p $(BUILDDIR)/src-cpp/rt $(BUILDDIR)/src-cpp/adt $(RUNTIME_BUILDDIR)/shared/src-cpp/rt $(RUNTIME_BUILDDIR)/static/src-cpp/rt)

-include $(DEPS) $(DEPS_RT)

$(GOAL): $(OBJECTS)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS) $(LIBFIRM_LFLAGS)

$(GOAL_RT_SHARED): $(OBJECTS_RT_SHARED)
	@echo '===> LD $@'
	$(Q)$(CC) -shared $(PIC_FLAGS) -o $@ $^ $(LFLAGS) $(LIBUNWIND_LFLAGS)

$(GOAL_RT_STATIC): $(OBJECTS_RT_STATIC)
	@echo '===> AR $@'
	$(Q)$(AR) -cru $@ $^

$(RUNTIME_BUILDDIR)/shared/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(PIC_FLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

$(RUNTIME_BUILDDIR)/static/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(PIC_FLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

$(BUILDDIR)/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(GOAL) $(GOAL_RT_SHARED) $(GOAL_RT_STATIC) $(DEPS) $(DEPS_RT) $(OBJECTS_RT_SHARED) $(OBJECTS_RT_STATIC)

