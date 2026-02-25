C_FILES = ${wildcard *.c ./**/*.c}

NCURSES_VERSION = 6.6
HOMEBREW_NCURSES_INCLUDE = -I/opt/homebrew/Cellar/ncurses/${NCURSES_VERSION}/include
HOMEBREW_NCURSES_LIB = -L/opt/homebrew/Cellar/ncurses/${NCURSES_VERSION}/lib

HOMEBREW_INCLUDE = -I/opt/homebrew/include
HOMEBREW_LIB = -L/opt/homebrew/lib

LINKER_FLAGS = -lncursesw -lformw -lmenuw -lSDL3

cursedvga: ${C_FILES}
	gcc -o ../cursedvga $^ ${LINKER_FLAGS}

macos: ${C_FILES}
	gcc -o ../cursedvga $^ ${HOMEBREW_NCURSES_INCLUDE} ${HOMEBREW_NCURSES_LIB} ${HOMEBREW_INCLUDE} ${HOMEBREW_LIB} ${LINKER_FLAGS}

clean: ; rm ../cursedvga
