ifeq ($(OS),Windows_NT)
	CC:=gcc	# We ensure the current environment
	EXE:=bin2array.exe
else
	CC:=cc
	EXE:=bin2array
endif

LD=ld.gold
OBJFILE=bin2array.o

all:
	$(CC) main.c -o $(EXE)

# FIX IN THE FUTURE
# link object file with ld.bfd(1) -pie -o bin2arrray -lc
# will get a ELF binary with "/lib/ld64.so.1" interpreter,
# and i did not know how could this happens.
#obj:
#	$(CC) -c main.c -o $(OBJFILE)

clean:
	rm -f $(EXE) $(OBJFILE)
