#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <sys/wait.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <signal.h>
#include <fcntl.h>

#include "../socket/socket.h"
#include "../dominoes/dominoes.h"
#include "../dominoes/dominoes_rules.h"
#include "user.h"
#include "game.h"
#include "log.h"

#define BACKLOG 10
#define DOMINOES_SIZE 29

char *intToChar(int);

int generateRandomNum(int);

Dominoe ** sendRandomDominoe(Dominoe **, int);
Dominoe * parseDominoe(int);

Game *cmdTake(Game *, int);
Game *cmdPut(Game *, int);
Game *cmdSkipTurn(Game *);
void cmdNull(Game *game);

void daemonize();

const char *log_file = "file.log";

int main(void) {
    //const char *log_file = "log-file";
    char *log_msg = (char *) malloc(512);

    srand((unsigned)time(0));

    struct sockaddr_in their_addr;
    socklen_t addr_size;
    int new_fd;
    
    int serverSocket;
    int port = 1111;
    struct sockaddr_in serverAdress; // Server adress

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    // bindinimas
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = inet_addr("127.0.0.1");
    //serverAdress.sin_addr.s_addr = inet_addr("192.168.1.165");
    serverAdress.sin_port = htons(port);
    memset(&(serverAdress.sin_zero), '\0', 8);
    
    if (bind(serverSocket, (struct sockaddr *)&serverAdress, sizeof(serverAdress)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(serverSocket, 10) == -1) {
        perror("listen");
        exit(1);
    }

    fd_set readfds, testfds;

    FD_ZERO(&readfds); // empty descriptor
    FD_SET(serverSocket, &readfds); // add server's socket to descriptor list

    User ** users = (User **) malloc((11) * sizeof(User*));
    Game ** games = (Game **) malloc((6) * sizeof(Game *));
    users[0] = NULL;
    games[0] = NULL;

    daemonize();

    while (1) { 
        int fd; 
        int nread;
        int result;
        testfds = readfds; // copying
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0); 
        if (result < 1) { 
            perror("select"); 
            exit(1); 
        } 
        for (fd = 0; fd < FD_SETSIZE; fd++) { 
            if (FD_ISSET(fd, &testfds)) { // descriptor is ready
                if (fd == serverSocket) { 
                    addr_size = sizeof(their_addr);
                    new_fd = accept(serverSocket, (struct sockaddr *)&their_addr, &addr_size); 
                    FD_SET(new_fd, &readfds); // add new descriptor to list

                    /* Sending welcome message */
                    char *welcome_msg = "Welcome!";
                    sendMessage(new_fd, strlen(welcome_msg), welcome_msg);
	    
                    sprintf(log_msg, "Server: connection from %s on socket %d", inet_ntoa(their_addr.sin_addr), new_fd);
                    write_log(log_file, log_msg);

                    // Adding user to list 
                    User *user = createUser(new_fd);
                    users = addUser(users, user);
                } 
                else { 
                    ioctl(fd, FIONREAD, &nread); 
                    if (nread == 0) { 
                        close(fd); 
                        FD_CLR(fd, &readfds); // remove descriptor from list

                        User *user = findUserBySock(users, fd);
                        users = removeUser(users, user);
                        
                        /* Log new connection from */
                        sprintf(log_msg, "User: %s disconnected on socket %d", user->name, user->sock);
                        write_log(log_file, log_msg);

                        Game *game = findGameByUser(games, user);
                        if (game != NULL)
                            games = removeGame(games, game);
                    } 
                    else { 
                        User * user = findUserBySock(users, fd);
                        char * msg = getMessage(user->sock, 5);

                        int game_nr = getGameNumber(games, user);
                        Game * game = games[game_nr];

                        if (strcmp(msg, "/reg ") == 0) {
                            int user_index = getUsersIndex(users, user);
                            user->name = getMessage(fd, 0);
                            users[user_index] = user;

                            /* Log who connected (username) */
                            sprintf(log_msg, "User: %s joined on socket %d", user->name, user->sock);
                            write_log(log_file, log_msg);

                            // Can we connect two players? 
                            if (lastUserBusy(users, user) == false) {
                                Game *game = connectTwoPlayers(users, user, log_file);
                                games = addGame(games, game);

                                users[getUsersIndex(users, game->player_1)]->status = 1;
                                users[getUsersIndex(users, game->player_2)]->status = 1;
                            }

                        }
                        if (strcmp(msg, "/put ") == 0)
                            games[game_nr] = cmdPut(game, fd);

                        if (strcmp(msg, "/take") == 0)
                            games[game_nr] = cmdTake(game, fd);

                        if (strcmp(msg, "/null") == 0)
                            cmdNull(game);

                        if (strcmp(msg, "/skip") == 0)
                            games[game_nr] = cmdSkipTurn(game);
                    } 
                } 
            } 
        } 
    }

    close(serverSocket);
    write_log(log_file, "Server: server is closed");
    return 0;
    //exit(EXIT_SUCCESS);
}

Game *cmdTake(Game *game, int fd) {
    game->stack_dominoes = sendRandomDominoe(game->stack_dominoes, fd);

    /* Send puted dominoes and dominoes left */
    sendNecessaryInformation(game);

    /* Whose turn? */
    if (game->player_1->turn == 1)
        sendTurnStatus(game->player_2, game->player_1);
    else if (game->player_2->turn == 1)
        sendTurnStatus(game->player_1, game->player_2);

    return game;
}

Game *cmdSkipTurn(Game *game) {
    sendNecessaryInformation(game);

    if (game->player_1->turn == 1) {
        game->player_1->has_turn = 0;
        game->player_1->turn = 0;
        game->player_2->turn = 1;
        if (game->player_2->has_turn == 1)
            sendTurnStatus(game->player_1, game->player_2);
        else {
            sendMessage(game->player_1->sock, 4, "/end");
            sendMessage(game->player_2->sock, 4, "/end");
            isWinnerOrNot(game->player_1, game->player_2, log_file);
        }
    }

    else if (game->player_2->turn == 1) {
        game->player_1->turn = 1;
        game->player_2->has_turn = 0;
        game->player_2->turn = 0;
        if (game->player_1->has_turn == 1)
            sendTurnStatus(game->player_2, game->player_1);
        else {
            sendMessage(game->player_1->sock, 4, "/end");
            sendMessage(game->player_2->sock, 4, "/end");
            isWinnerOrNot(game->player_1, game->player_2, log_file);
        }
    }

    return game;
}

Game *cmdPut(Game *game, int fd) {
    int first, second, position;
    char *msg;

    msg = getMessage(fd, 3);
  
    first = msg[0] - '0';
    second = msg[1] - '0';
    position = msg[2] - '0';

    Dominoe * dominoe = createDominoe(first, second);
 
    bool can_put = false;
    bool end_game = false;
    if (canPutDominoe(game->puted_dominoes, dominoe, position)) {
        sendMessage(fd, 1, "1");
        game->puted_dominoes = putDominoe(game->puted_dominoes, dominoe, position);
        can_put = true;
        msg = getMessage(fd, 5);
        if (strcmp(msg, "/iwin") == 0) {
            end_game = true;
        }
    }
    else {
        sendMessage(fd, 1, "0");
    }
                     
    /* Send puted dominoes and dominoes left */
    sendNecessaryInformation(game);
                         
    if (can_put && !end_game) {
        if (game->player_1->turn == 1) {
            game->player_1->turn = 0;
            game->player_2->turn = 1;
            sendTurnStatus(game->player_1, game->player_2);

        }
        else if (game->player_2->turn == 1) {
            game->player_2->turn = 0;
            game->player_1->turn = 1;
            sendTurnStatus(game->player_2, game->player_1);
        }
    }
    else if (!end_game){
        /* Whose turn? */
        if (game->player_1->turn == 1)
            sendTurnStatus(game->player_2, game->player_1);
        else if (game->player_2->turn == 1)
            sendTurnStatus(game->player_1, game->player_2);
    }
    else if (end_game) {
        game = setWinner(game, fd, log_file);
    }

    return game;
}

void cmdNull(Game *game) {
    /* Send puted dominoes and dominoes left */
    sendNecessaryInformation(game);
                            
    /* Whose turn? */
    if (game->player_1->turn == 1)
        sendTurnStatus(game->player_2, game->player_1);
    else if (game->player_2->turn == 1)
        sendTurnStatus(game->player_1, game->player_2);
}

Dominoe ** sendRandomDominoe(Dominoe ** dominoes, int new_fd) {
    int random_number;
    int dominoes_size = getDominoesSize(dominoes);

    if (dominoes_size > 0) {
        sendMessage(new_fd, 1, "1");

        random_number = rand() % dominoes_size;
        Dominoe * dominoe = dominoes[random_number];

        char *dominoe_char = dominoeToChar(dominoe->value_1, dominoe->value_2);

        sendMessage(new_fd, 2, dominoe_char);

        dominoes = removeDominoe(dominoes, random_number);
    }
    else
        sendMessage(new_fd, 1, "0");

    return dominoes;
}

Dominoe * parseDominoe(int sock) {
    char *msg = getMessage(sock, 2);
    int first, second;

    first = msg[0] - '0';
    second = msg[1] - '0';

    Dominoe * dominoe = createDominoe(first, second);

    return dominoe;
}

char *intToChar(int value) {
    char *nr = (char *) malloc(sizeof(char));
    sprintf(nr, "%i", value);
    return nr;
}

void sendUsersList(User ** users, int new_fd) {
    int index = 0;
    while (users[index] != NULL) {
	sendMessage(new_fd, strlen(users[index]->name), users[index]->name);
	sendMessage(new_fd, 8, " status ");
	if (users[index]->status == 0)
	    sendMessage(new_fd, 7, "waiting");
	if (users[index]->status == 1)
	    sendMessage(new_fd, 7, "playing");
	sendMessage(new_fd, 1, "\n");
	index++;
    }
}

int generateRandomNum(int size) {
    int random_number;
    random_number = rand() % size;
    return random_number;
}

void daemonize() {
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
	perror("pid");
        exit(1);
    }
    if (pid > 0) {
	perror("pid");
        exit(0);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
	perror("sid");
        exit(1);
    }

    //chdir("/");
    //if ((chdir("/")) < 0) {
    //    exit(EXIT_FAILURE);
    //}
}
