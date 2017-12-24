# Makefile for tsxset
# Mark Street <marksmanuk@gmail.com>

CC		= gcc
RM		= rm
FLAGS	= -O3 -Wall
EXE		= tsxset
LIBS	= -l bluetooth

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

all: $(EXE)

clean:
	$(RM) *.o $(EXE)

$(EXE): tsxset.o
	$(CC) -o $@ $< $(LIBS)
