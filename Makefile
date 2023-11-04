CC=gcc
CFLAGS=-g -std=c11 -pedantic -Wall
#CFLAGS+= -Wextra
LDLIBS=-lm
DIRSOURCE=src
DIRH=$(DIRSOURCE)/header_files


parser: scanner.o dynamic_buffer.o parser.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

scanner.o: $(DIRSOURCE)/scanner.c $(DIRH)/scanner.h dynamic_buffer.o
	$(CC) $(CFLAGS) -c $<

dynamic_buffer.o: $(DIRSOURCE)/dynamic_buffer.c $(DIRH)/dynamic_buffer.h
	$(CC) $(CFLAGS) -c $<


#implicit rule
%.o: $(DIRSOURCE)/%.c $(DIRH)/%.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o parser
