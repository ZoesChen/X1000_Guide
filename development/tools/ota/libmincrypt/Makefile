#############################################################
# Makefile for libminicrypt static library.
#############################################################
#set your own environment option
CROSSCOMPILE ?= mips-linux-gnu-
CC = $(CROSS)gcc
C_FLAG = -D_NOMNG -D_FILELINE -I ../include

#static library use 'ar' command
AR = $(CROSS)ar

#set your inc and lib
INC =

#make target lib and relevant obj
PRG = libmincrypt.a
OBJ = rsa.o  rsa_e_3.o  rsa_e_f4.o  sha.o

#all target
all:$(PRG)
$(PRG):$(OBJ)
	${AR} rv ${PRG} $^

.SUFFIXES: .c .o .cpp
.c.o:
	$(CC) $(C_FLAG) $(INC) -c $*.c -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
