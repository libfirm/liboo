-include config.mak

LIBFIRM_CPPFLAGS ?= `pkg-config --cflags libfirm`
LIBFIRM_LFLAGS   ?= `pkg-config --libs libfirm`
INSTALL ?= install

BUILDDIR=build
GOAL = $(BUILDDIR)/liboo.so
CPPFLAGS = -I. $(LIBFIRM_CPPFLAGS)
CXXFLAGS = -Wall -W -O0 -g3
CFLAGS = -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wunreachable-code -Wlogical-op -Werror -O0 -g3 -std=c99 -pedantic
LFLAGS = $(LIBFIRM_LFLAGS)
SOURCES = $(wildcard src-cpp/*.c) $(wildcard src-cpp/adt/*.c)
DEPS = $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $(SOURCES))))
OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

Q ?= @

all: $(GOAL)

-include $(DEPS)

$(GOAL): $(OBJECTS)
	@echo '===> LD $@'
	$(Q)$(CC) -shared -o $@ $^ $(LFLAGS)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	@echo '===> CC $<'
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MD -MF $(addprefix $(BUILDDIR)/, $(addsuffix .d, $(basename $<))) -c -o $@ $<

$(BUILDDIR):
	$(INSTALL) -d $(BUILDDIR)/src-cpp $(BUILDDIR)/src-cpp/adt

librts.o: librts/librts.cc
	@echo '===> Building $<'
	$(Q)$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(GOAL) $(DEPS)
