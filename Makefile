
all: viewer

viewer: viewer.c viewer.h
	./glcc viewer.c viewer

clean:
	rm -f viewer core
