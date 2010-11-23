-include config.mak

LIBFIRM_CPPFLAGS ?= `pkg-config --cflags libfirm`
LIBFIRM_LFLAGS   ?= `pkg-config --libs libfirm`
INSTALL ?= install
DLLEXT ?= .so

BUILDDIR=build
GOAL = $(BUILDDIR)/liboo$(DLLEXT)
CPPFLAGS = -I. -I./include/ $(LIBFIRM_CPPFLAGS)
CXXFLAGS = -Wall -W -O0 -g3
CFLAGS = -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Werror -O0 -g3 -std=c99 -pedantic
# disabled the following warnings for now. They fail on OS/X Snow Leopard:
# the first one gives false positives because of system headers, the later one
# doesn't exist in the old gcc there
#CFLAGS += -Wunreachable-code -Wlogical-op
LFLAGS = $(LIBFIRM_LFLAGS)
SOURCES = $(wildcard src-cpp/*.c) $(wildcard src-cpp/adt/*.c)
DEPS = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES))))
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

Q ?= @

all: $(GOAL)

# Make sure our build-directories are created
UNUSED := $(shell mkdir -p $(BUILDDIR)/src-cpp/adt)

-include $(DEPS)

$(GOAL): $(OBJECTS)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS)

$(BUILDDIR)/%.o: %.c
	@echo '===> CC $<'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(GOAL) $(DEPS)
