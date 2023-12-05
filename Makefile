CC=gcc
CFLAGS=-g -std=c11 -pedantic -Wall -Wno-unused-variable -fno-stack-protector #-fsanitize=address
#CFLAGS+= -Wextra
LDLIBS=-lm
DIRSOURCE=src
DIRH=$(DIRSOURCE)/header_files


compiler: main.o parser.o scanner.o dynamic_buffer.o symtable.o semantic.o stack.o code_gen.o dynamic_array.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

main.o: $(DIRSOURCE)/main.c
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

parser.o: $(DIRSOURCE)/parser.c $(DIRH)/parser.h $(DIRH)/scanner.h $(DIRH)/dynamic_buffer.h $(DIRH)/symtable.h $(DIRH)/dynamic_array.h 
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

scanner.o: $(DIRSOURCE)/scanner.c $(DIRH)/scanner.h dynamic_buffer.o
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

dynamic_buffer.o: $(DIRSOURCE)/dynamic_buffer.c $(DIRH)/dynamic_buffer.h
	$(CC) $(CFLAGS) -c $<

symtable.o: $(DIRSOURCE)/symtable.c $(DIRH)/symtable.h
	$(CC) $(CFLAGS) -c $<

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

clean:
	rm -f *.o compiler
