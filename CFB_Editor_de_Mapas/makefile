CC = gcc

OBJS = basics.c vec2d.c transform.c primitives.c tess.c TX_Font.c UI.c tools.c CFBEDT.c
# tinyfiledialogs.c
# -w (suppresses all warnings)
# -Wl,-subsystem (windows gets rid of the console window)

COMPILER_FLAGS_RELEASE = -w -Wl,-subsystem,windows
COMPILER_FLAGS_QUICK = -w
COMPILER_FLAGS_DEBUG = -fmax-errors=3 -Waddress -Warray-bounds=1 -Wbool-compare -Wformat -Wimplicit -Wlogical-not-parentheses -Wmaybe-uninitialized -Wmemset-transposed-args -Wmissing-braces -Wopenmp-simd -Wparentheses -Wpointer-sign -Wreturn-type -Wsequence-point -Wsizeof-pointer-memaccess -Wstrict-aliasing -Wstrict-overflow=1 -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wvolatile-register-var -Wmissing-field-initializers -Wmissing-parameter-type -Woverride-init -Wsign-compare -Wtype-limits -Wshift-negative-value
COMPILER_FLAGS_MAX = -Wall -Wextra -Werror -O2 -std=c99 -pedantic

ifeq ($(OS),Windows_NT) # Windows_NT is the identifier for all versions of Windows
	DETECTED_OS := Windows
else
	DETECTED_OS := $(shell uname)
endif

ifeq ($(DETECTED_OS),Windows)

	INCLUDE_PATHS = -IC:/SDL/SDL2-2.28.2/i686-w64-mingw32/include/SDL2
	INCLUDE_PATHS += -IC:/SDL/SDL2_image-2.8.2/i686-w64-mingw32/include/SDL2
	INCLUDE_PATHS += -IC:/SDL/SDL2_ttf-2.22.0/i686-w64-mingw32/include/SDL2

	LIBRARY_PATHS = -LC:/SDL/SDL2-2.28.2/i686-w64-mingw32/lib
	LIBRARY_PATHS += -LC:/SDL/SDL2_image-2.8.2/i686-w64-mingw32/lib
	LIBRARY_PATHS += -LC:/SDL/SDL2_ttf-2.22.0/i686-w64-mingw32/lib

	LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
else
	INCLUDE_PATHS = -I/usr/include/SDL2
	LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf
endif

OBJ_NAME = CFB_Editor_de_Mapas

release : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_RELEASE) $(LINKER_FLAGS) -std=c11 -o $(OBJ_NAME)
quick : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_QUICK) $(LINKER_FLAGS) -std=c11 -o $(OBJ_NAME)
debug : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_DEBUG) $(LINKER_FLAGS) -std=c11 -o $(OBJ_NAME)
max : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_MAX) $(LINKER_FLAGS) -std=c11 -o $(OBJ_NAME)