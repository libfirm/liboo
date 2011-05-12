-include config.mak

LIBFIRM_CPPFLAGS ?= `pkg-config --cflags libfirm`
LIBFIRM_LFLAGS   ?= `pkg-config --libs libfirm`
INSTALL ?= install
DLLEXT ?= .so

BUILDDIR=build
GOAL = $(BUILDDIR)/liboo$(DLLEXT)
GOAL_RT = $(BUILDDIR)/liboo_rt$(DLLEXT)
CPPFLAGS = -I. -I./include/ $(LIBFIRM_CPPFLAGS) $(LIBUNWIND_CPPFLAGS)
CXXFLAGS = -Wall -W -O0 -g3
CFLAGS = -Wall -W -Wstrict-prototypes -Wmissing-prototypes -O0 -g3 -std=c99 -pedantic
# disabled the following warnings for now. They fail on OS/X Snow Leopard:
# the first one gives false positives because of system headers, the later one
# doesn't exist in the old gcc there
#CFLAGS += -Wunreachable-code -Wlogical-op
LFLAGS = 
SOURCES = $(wildcard src-cpp/*.c) $(wildcard src-cpp/adt/*.c)
SOURCES_RT = $(wildcard src-cpp/rt/*.c)
DEPS = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES))))
DEPS_RT = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES_RT))))
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
OBJECTS_RT = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES_RT))))

Q ?= @

all: $(GOAL) $(GOAL_RT)

# Make sure our build-directories are created
UNUSED := $(shell mkdir -p $(BUILDDIR)/src-cpp/adt $(BUILDDIR)/src-cpp/rt)

-include $(DEPS) $(DEPS_RT)

$(GOAL): $(OBJECTS)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS) $(LIBFIRM_LFLAGS)

$(GOAL_RT): $(OBJECTS_RT)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS) $(LIBUNWIND_LFLAGS)

$(BUILDDIR)/%.o: %.c
	@echo '===> CC $<'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(GOAL) $(GOAL_RT) $(DEPS)

