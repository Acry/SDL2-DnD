CFLAGS   = -Wall -Wextra -mtune=native `sdl2-config --cflags`
LDFLAGS  = `sdl2-config --libs` -lSDL2_image -lm
LDFLAGS1 = $(LDFLAGS)  -lcurl
LDFLAGS2 = $(LDFLAGS)  -ljansson
LDFLAGS3 = $(LDFLAGS1) $(LDFLAGS2)

.SUFFIXES:
.SUFFIXES: .c .o

srcdir	 =src/
TARGETS	 = drag_drop tokenize shader_drop

.PHONY: all
all: $(TARGETS)

# take file and text drop
drag_drop: $(srcdir)helper.c $(srcdir)main.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

# tokenize the output
tokenize: $(srcdir)helper.c $(srcdir)main2.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

# drop shadertoy shader and download it, plus texture if applicable
shader_drop: $(srcdir)helper.c $(srcdir)main3.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS3)

.PHONY: clean
clean:
	@rm $(TARGETS) 2>/dev/null || true

