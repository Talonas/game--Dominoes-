NAME   = server
 
CC     = gcc
CFLAGS = -Wall -c
 
SRC = ../socket/socket.c ../dominoes/dominoes.c ../dominoes/dominoes_rules.c user.c game.c log.c
OBJ = $(SRC:.c=.o) main.o
 
compile: $(OBJ)
	$(CC) $(LFLAGS) -o $(NAME) $(OBJ)

clean:
	rm -fr *~ *.o *.gch
	rm -f $(NAME)
	rm -f $(TAME)
