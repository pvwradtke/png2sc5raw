all: 
	gcc png2sc5raw.c -lSDL2 -lSDL2_image -o png2sc5raw
	gcc png2sc8raw.c -lSDL2 -lSDL2_image -o png2sc8raw
clean:
	rm png2sc5raw
	rm png2sc8raw

install: 
	cp png2sc5raw /usr/local/bin
	cp png2sc8raw /usr/local/bin

