CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison

TARGET = def88186cc
SOURCES = main.c ast.c codegen.c preprocessor.c
GENERATED = lexer.c parser.tab.c parser.tab.h
OBJECTS = $(SOURCES:.c=.o) lexer.o parser.tab.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

parser.tab.c parser.tab.h: parser.y
	$(YACC) -d parser.y

lexer.c: lexer.l parser.tab.h
	$(LEX) -o lexer.c lexer.l

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJECTS) $(GENERATED)

.PHONY: all clean
