ifeq ($(OS),Windows_NT)
	LIBS=-lmingw32 -lSDL2main -lSDL2
	OUTPUT=arcade_volleybal.exe
else
	LIBS=-lSDL2
	OUTPUT=arcade_volleybal
endif


all: main.c
	gcc -g -o $(OUTPUT) main.c -std=c18 -pedantic -Wall -Wextra $(LIBS)

clean:
	$(RM) $(OUTPUT)
