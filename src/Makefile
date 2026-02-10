C_FILES = ${wildcard *.c ./**/*.c}

cursedvga: cursedvga.c tga.c palette.c UI/ui.c UI/ncursutil.c UI/nav.c UI/disp.c
	gcc -o ../cursedvga $^ -lncursesw -lformw -lmenuw

clean: ; rm ../cursedvga
