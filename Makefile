CC = gcc
CFLAGS = -Wall -pedantic -g
OBJS = mush.o parsefuncs.o mainfuncs.o
TOBJS = tests.o mush.o parsefuncs.o mainfuncs.o
DEPS = mush.h
NAME = mush
UNIX4 = tnewma03@unix4.csc.calpoly.edu
all : $(NAME)

$(NAME) : $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -lm

mush.o : mush.c mush.h
	$(CC) $(CFLAGS) -c mush.c

parsefuncs.o : parsefuncs.c mush.h
	$(CC) $(CFLAGS) -c parsefuncs.c

mainfuncs.o : mainfuncs.c mush.h
	$(CC) $(CFLAGS) -c mainfuncs.c

tests.o : tests.c mush.h
	$(CC) $(CFLAGS) -c tests.c

testm : $(DEPS) $(TOBS)
	$(CC) $(CFLAGS) -o tests $(TOBJS) -lm

testr : 
	./tests

testv :
	valgrind ./tests

grader :
	~getaylor-grader/tryAsgn6 > graded.txt

ssh : 
	ssh $(UNIX4)

scpf :
	scp ./* $(UNIX4):~/cpe357/hw6

scpm :
	scp *.c $(UNIX4):~/cpe357/hw6

handin : 
	handin getaylor-grader 357hw6-11 Makefile *.c *.h README.md

clean :
	rm *.o $(NAME)

#ctr x ctr a

