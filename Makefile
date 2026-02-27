all: run

build: clean
	gcc -o install install.c -lncurses -lpanel -lmenu
clean:
	rm install -f
run: build
	./install
#dev: build
#	./install
#	rm install