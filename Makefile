CC:=cc
EXE:=bin2array

OBJCOPY ?= objcopy
OBJFILE=bin2array.o
C_TEST_SRC=test.c
C_TEST_OBJ=test.o
JAVA_TEST_CLASS=Test1

$(EXE): $(OBJFILE)
	$(CC) $(OBJFILE) -o $(EXE)

$(OBJFILE):
	$(CC) main.c -c -o $(OBJFILE)

clean:
	rm -f $(EXE) $(OBJFILE)
	rm -f $(C_TEST_SRC) 
	rm -f $(C_TEST_OBJ) 
	rm -f $(JAVA_TEST_CLASS).java
	rm -f $(JAVA_TEST_CLASS).class
	rm -f tmpfile
	rm -f tmpfile1

test:	$(EXE)
	set -e; \
	size=`wc -c < $(EXE)`; \
	[ $$((size%8)) = 0 ] && size=; \
	for mode in uint8 uint16 uint32 uint64 int8 int16 int32 int64; do \
		echo "Testing C $$mode mode" 1>&2; \
		for base in "" --dec; do \
			echo "int main(){" > $(C_TEST_SRC); \
			./$(EXE) --c-$$mode $$base --file $(EXE) >> $(C_TEST_SRC); \
			echo "write(2, \"Executing C $$mode mode test\\\n\", strlen(\"Executing C $$mode mode test\\\n\"));" >> $(C_TEST_SRC); \
			echo "write(1, array, ARRAY_LEN);" >> $(C_TEST_SRC); \
			echo "return 0;" >> $(C_TEST_SRC); \
			echo "}" >> $(C_TEST_SRC); \
			$(CC) --include string.h --include unistd.h --include stdint.h --include stddef.h $(C_TEST_SRC) -o test; \
			./test > output.dat; \
			cmp $(EXE) output.dat; \
			rm output.dat test test.c; \
		done; \
	done; \
	dd if=$(EXE) of=tmpfile bs=1024 count=1; \
	for mode in byte char; do \
		echo "Testing Java $$mode mode, will write to $(JAVA_TEST_CLASS).java" 1>&2; \
		for base in "" --dec; do \
			echo "class $(JAVA_TEST_CLASS) {" > $(JAVA_TEST_CLASS).java; \
			./$(EXE) --java-$$mode $$base --file tmpfile | sed -e 's/final/static\ final/g' >> $(JAVA_TEST_CLASS).java; \
			echo "public static void main(String[] args) {" >> $(JAVA_TEST_CLASS).java; \
			echo "try {" >> $(JAVA_TEST_CLASS).java; \
			echo "System.err.println(\"Executing Java $$mode mode test\");" >> $(JAVA_TEST_CLASS).java; \
			if [ "$$mode" = "byte" ]; then \
				echo "System.out.write(array);" >> $(JAVA_TEST_CLASS).java; \
			elif [ "$$mode" = "char" ]; then \
				echo "for (int i = 0; i < ARRAY_LEN; i++) System.out.write((byte)array[i]);" >> $(JAVA_TEST_CLASS).java; \
			fi; \
			echo "System.out.flush();" >> $(JAVA_TEST_CLASS).java; \
			echo "} catch (Throwable t) {}" >> $(JAVA_TEST_CLASS).java; \
			echo "}}" >> $(JAVA_TEST_CLASS).java; \
			javac $(JAVA_TEST_CLASS).java; \
			java $(JAVA_TEST_CLASS) > tmpfile1; \
			cmp tmpfile tmpfile1; \
			rm tmpfile1 $(JAVA_TEST_CLASS).java $(JAVA_TEST_CLASS).class; \
		done; \
	done;
	rm tmpfile \
