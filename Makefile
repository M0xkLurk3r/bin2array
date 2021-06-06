CC:=cc
EXE:=bin2array

all:
	$(CC) -o $(EXE) main.c

clean:
	rm $(EXE)
