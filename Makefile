build:
	rm install -f
	gcc -o install install.c -lncurses -lpanel -lmenu
dev: build
	./install
	rm install