all: 
	gcc png2sc5raw.c -lSDL2 -lSDL2_image -o png2sc5raw

clean: 
	rm png2sc5raw

install: 
	cp png2sc5raw /usr/local/bin

