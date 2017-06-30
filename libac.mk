# step 0 makefile for building quick

OBJ = acism_create.o acism_file.o acism.o
CFLAGS = -O3 -fpic --std=c99 -DNDEBUG -Wall -Wextra -Wno-unused

#add -s switch to strip symbols
lib: $(OBJ)
	ar csr libac.a $(OBJ)

clean:
	rm -rf $(OBJ) $(TEST_OBJ) libac.so libac.a ac.dll ac.lib acism_test
