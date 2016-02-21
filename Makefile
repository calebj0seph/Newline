prefix := /usr/local
CPPFLAGS += --std=gnu99 -Wall -Wextra
REL_CPPFLAGS += -Os
DEBUG_CPPFLAGS += -DDEBUG -g
EXECUTABLE := newline
SRCS := newline.c args.c trim.c

ifeq ($(OS), Windows_NT)
  CC := gcc
  SRCS += tempfile-win32.c
  LDFLAGS += -municode -static
  LDLIBS += -lShlwapi
  CPPFLAGS += -DWIN32_LEAN_AND_MEAN -D_CRT_RAND_S -DUNICODE
  REL_LDFLAGS += -flto
  REL_CPPFLAGS += -flto
  EXECUTABLE_EXT := .exe
  PATHSEP := \ #
  PATHSEP := $(strip $(PATHSEP))
  RM := del /q
  MKDIR := md
  CP = copy /y $(1) $(2)
else
  PATHSEP := /
  RM := rm -f
  MKDIR := mkdir -p
  CP = cp $(1) -t $(2)
  UNAME := $(shell uname)
  ifeq ($(UNAME), Darwin)
    CC := clang
    OBJC := clang
    SRCS += tempfile-apple.m
    LDFLAGS += -framework Foundation
  else
    SRCS += tempfile-linux.c
    CPPFLAGS += -D_FILE_OFFSET_BITS=64
    REL_LDFLAGS += -flto
    REL_CPPFLAGS += -flto
  endif
endif

OBJS := $(addsuffix .o, $(basename $(SRCS)))

.PHONY: all clean debug release install uninstall

all: release

debug: CPPFLAGS += $(DEBUG_CPPFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: $(EXECUTABLE)

release: CPPFLAGS += $(REL_CPPFLAGS)
release: LDFLAGS += $(REL_LDFLAGS)
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)

clean:
	-$(RM) $(OBJS) $(EXECUTABLE)$(EXECUTABLE_EXT)

install: release
	-$(MKDIR) $(prefix)$(PATHSEP)bin
	$(call CP, $(EXECUTABLE)$(EXECUTABLE_EXT), $(prefix)$(PATHSEP)bin)

uninstall:
	$(RM) $(prefix)$(PATHSEP)bin$(PATHSEP)$(EXECUTABLE)$(EXECUTABLE_EXT)
