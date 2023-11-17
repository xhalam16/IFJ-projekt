CC=gcc
CFLAGS=-g -std=c11 -pedantic -Wall -Wno-unused-variable
#CFLAGS+= -Wextra
LDLIBS=-lm
DIRSOURCE=src
DIRH=$(DIRSOURCE)/header_files


parser: scanner.o dynamic_buffer.o parser.o symtable.o semantic.o stack.o
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

#implicit rule
%.o: $(DIRSOURCE)/%.c $(DIRH)/%.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o parser semantic
