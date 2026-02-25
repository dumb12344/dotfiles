build:
	rm install -f
	gcc -o install install.c -lncurses -lpanel
dev: build
	./install
	rm install