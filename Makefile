CC=gcc
CFLAGS=-g -std=c11 -pedantic -Wall -Wno-unused-variable -fno-stack-protector -Wno-newline-eof -Wno-unused-command-line-argument #-fsanitize=address
#CFLAGS+= -Wextra
LDLIBS=-lm
DIRSOURCE=src
DIRH=$(DIRSOURCE)/header_files


parser: scanner.o dynamic_buffer.o parser.o symtable.o semantic.o stack.o code_gen.o dynamic_array.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

scanner.o: $(DIRSOURCE)/scanner.c $(DIRH)/scanner.h dynamic_buffer.o
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

dynamic_buffer.o: $(DIRSOURCE)/dynamic_buffer.c $(DIRH)/dynamic_buffer.h
	$(CC) $(CFLAGS) -c $<

symtable.o: $(DIRSOURCE)/symtable.c $(DIRH)/symtable.h
	$(CC) $(CFLAGS) -c $<

semantic: parser.o semantic.o symtable.o dynamic_buffer.o scanner.o stack.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

semantic.o: $(DIRSOURCE)/semantic.c
	$(CC) $(CFLAGS) -c $<

stack.o: $(DIRSOURCE)/stack.c $(DIRH)/stack.h
	$(CC) $(CFLAGS) -c $<

code_gen.o: $(DIRSOURCE)/code_gen.c $(DIRH)/code_gen.h
	$(CC) $(CFLAGS) -c $<

dynamic_array.o: $(DIRSOURCE)/dynamic_array.c $(DIRH)/dynamic_array.h
	$(CC) $(CFLAGS) -c $<

#implicit rule
%.o: $(DIRSOURCE)/%.c $(DIRH)/%.h
	$(CC) $(CFLAGS) -c $<

code_gen.o: src/code_gen.c src/header_files/code_gen.h \
 src/header_files/parser.h src/header_files/token.h \
 src/header_files/dynamic_buffer.h src/header_files/error.h \
 src/header_files/scanner.h src/header_files/symtable.h \
 src/header_files/stack.h src/header_files/semantic.h \
 src/header_files/dynamic_array.h src/header_files/code_gen.h
dynamic_array.o: src/dynamic_array.c src/header_files/dynamic_array.h
dynamic_buffer.o: src/dynamic_buffer.c src/header_files/dynamic_buffer.h
parser.o: src/parser.c src/header_files/parser.h src/header_files/token.h \
 src/header_files/dynamic_buffer.h src/header_files/error.h \
 src/header_files/scanner.h src/header_files/symtable.h \
 src/header_files/stack.h src/header_files/semantic.h \
 src/header_files/dynamic_array.h src/header_files/code_gen.h
scanner.o: src/scanner.c src/header_files/scanner.h \
 src/header_files/token.h src/header_files/dynamic_buffer.h \
 src/header_files/error.h
semantic.o: src/semantic.c src/header_files/semantic.h \
 src/header_files/parser.h src/header_files/token.h \
 src/header_files/dynamic_buffer.h src/header_files/error.h \
 src/header_files/scanner.h src/header_files/symtable.h \
 src/header_files/stack.h src/header_files/dynamic_array.h \
 src/header_files/code_gen.h
stack.o: src/stack.c src/header_files/stack.h
symtable.o: src/symtable.c src/header_files/symtable.h


clean:
	rm -f *.o parser semantic
