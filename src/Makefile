C_FILES = ${wildcard *.c ./**/*.c}

cursedvga: ${C_FILES}
	gcc -o ../cursedvga $^ -lncursesw -lformw -lmenuw

clean: ; rm ../cursedvga
