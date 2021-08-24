ifeq ($(OS),Windows_NT)
	CC:=gcc	# We ensure the current environment
	EXE:=bin2array.exe
else
	CC:=cc
	EXE:=bin2array
endif

OBJCOPY ?= objcopy
LD=ld.gold
OBJFILE=bin2array.o

$(EXE):
	$(CC) main.c -o $(EXE)

# FIX IN THE FUTURE
# link object file with ld.bfd(1) -pie -o bin2arrray -lc
# will get a ELF binary with "/lib/ld64.so.1" interpreter,
# and i did not know how could this happens.
#obj:
#	$(CC) -c main.c -o $(OBJFILE)

clean:
	rm -f $(EXE) $(OBJFILE)

test:	$(EXE)
	set -e; \
	size=`wc -c < $(EXE)`; \
	[ $$((size%8)) = 0 ] && size=; \
	for mode in uint8 uint16 uint32 uint64 int8 int16 int32 int64; do \
		echo "Testing C $$mode mode" 1>&2; \
		for base in "" --dec; do \
			./$(EXE) --c-$$mode $$base --file $(EXE) > test.c; \
			$(CC) --include stdint.h -c test.c -o test.o; \
			$(OBJCOPY) --output-target binary test.o; \
			[ -n "$$size" ] && truncate -s $$size test.o; \
			cmp $(EXE) test.o; \
			rm -f test.c test.o; \
		done; \
	done
