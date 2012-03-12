-include config.mak

LIBFIRM_CPPFLAGS ?= `pkg-config --cflags libfirm`
LIBFIRM_LFLAGS   ?= `pkg-config --libs libfirm`
INSTALL ?= install
DLLEXT ?= .so
CC ?= gcc
AR ?= ar
CFLAGS ?= -O0 -g3

guessed_target := $(shell $(CC) -dumpmachine)
TARGET         ?= $(guessed_target)

ifeq ($(findstring x86_64, $(TARGET)), x86_64)
	# compile library normally but runtime in 32bit mode
	RT_CFLAGS += -m32
	RT_LFLAGS += -m32
endif
ifeq ($(findstring darwin11, $(TARGET)), darwin11)
	# compile library normally but runtime in 32bit mode
	RT_CFLAGS += -m32
	RT_LFLAGS += -m32
endif
ifeq ($(TARGET), i686-invasic-octopos)
	OCTOPOS_BASE=../octopos-app
	GCC_INCLUDE:=$(shell $(CC) --print-file-name=include)
	RT_CFLAGS += -m32 -fno-stack-protector -mfpmath=sse -msse2 -nostdinc -isystem $(GCC_INCLUDE) -I $(OCTOPOS_BASE)/include -D__OCTOPOS__
	RT_LFLAGS += -m32 -nostdlib -Wl,-T,$(OCTOPOS_BASE)/sections.x $(OCTOPOS_BASE)/liboctopos.a
endif

BUILDDIR=build
RUNTIME_BUILDDIR=$(BUILDDIR)/$(TARGET)
GOAL = $(BUILDDIR)/liboo$(DLLEXT)
GOAL_RT_SHARED = $(RUNTIME_BUILDDIR)/liboo_rt$(DLLEXT)
GOAL_RT_STATIC = $(RUNTIME_BUILDDIR)/liboo_rt.a
CPPFLAGS = -I. -I./include/ $(LIBFIRM_CPPFLAGS)
CFLAGS += -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Werror -std=c99 -pedantic
# disabled the following warnings for now. They fail on OS/X Snow Leopard:
# the first one gives false positives because of system headers, the later one
# doesn't exist in the old gcc there
#CFLAGS += -Wunreachable-code -Wlogical-op
LFLAGS +=
PIC_FLAGS = -fpic
SOURCES = $(wildcard src-cpp/*.c) $(wildcard src-cpp/adt/*.c)
SOURCES_RT = $(wildcard src-cpp/rt/*.c)
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
OBJECTS_RT_SHARED = $(addprefix $(RUNTIME_BUILDDIR)/shared/, $(addsuffix .o, $(basename $(SOURCES_RT))))
OBJECTS_RT_STATIC = $(addprefix $(RUNTIME_BUILDDIR)/static/, $(addsuffix .o, $(basename $(SOURCES_RT))))
DEPS = $(OBJECTS:%.o=%.d) $(OBJECTS_RT_SHARED:%.o=%.d) $(OBJECTS_RT_STATIC:%.o=%.d)

Q ?= @

.PHONY: all runtime clean

all: $(GOAL) runtime

runtime: $(GOAL_RT_SHARED) $(GOAL_RT_STATIC)

# Make sure our build-directories are created
UNUSED := $(shell mkdir -p $(BUILDDIR)/src-cpp/rt $(BUILDDIR)/src-cpp/adt $(RUNTIME_BUILDDIR)/shared/src-cpp/rt $(RUNTIME_BUILDDIR)/static/src-cpp/rt)

-include $(DEPS)

$(GOAL): $(OBJECTS)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS) $(LIBFIRM_LFLAGS)

$(GOAL_RT_SHARED): $(OBJECTS_RT_SHARED)
	@echo '===> LD $@'
	$(Q)$(CC) -shared $(RT_LFLAGS) $(PIC_FLAGS) -o $@ $^ $(LFLAGS)

$(GOAL_RT_STATIC): $(OBJECTS_RT_STATIC)
	@echo '===> AR $@'
	$(Q)$(AR) -cru $@ $^

$(RUNTIME_BUILDDIR)/shared/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(RT_CFLAGS) $(PIC_FLAGS) -MMD -c -o $@ $<

$(RUNTIME_BUILDDIR)/static/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(RT_CFLAGS) -MMD -c -o $@ $<

$(BUILDDIR)/%.o: %.c
	@echo '===> CC $@'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(PIC_FLAGS) -MMD -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(OBJECTS_RT_SHARED) $(OBJECTS_RT_STATIC) $(GOAL) $(GOAL_RT_STATIC) $(GOAL_RT_SHARED) $(DEPS) $(DEPS_RT)

