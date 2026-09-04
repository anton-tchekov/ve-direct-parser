PROJNAME := ve-direct-gui
OBJDIR   := obj

SRCFILES := $(wildcard *.c)
OBJFILES := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCFILES))
DEPFILES := $(patsubst %.c,$(OBJDIR)/%.d,$(SRCFILES))

LDLIBS   := -lSDL2 -lSDL2_ttf

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
			-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
			-Wredundant-decls -Wnested-externs -Winline \
			-Wuninitialized -Wstrict-prototypes

CFLAGS := -g -std=gnu99 $(WARNINGS)

.PHONY: all clean

all: $(PROJNAME)

$(PROJNAME): $(OBJFILES)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: %.c Makefile
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPFILES)

clean:
	rm -rf $(OBJDIR) $(PROJNAME)
