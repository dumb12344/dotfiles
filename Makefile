all: run

dev: cleanrun

build:
	gcc -o install install.c -lncurses -lpanel -lmenu

run: build
	./install

cleanrun: run
	rm install -f