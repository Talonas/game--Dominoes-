#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdbool.h>

#include <sys/types.h>
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

#define DOMINOES_SIZE 29

char *readLine();
char * intToChar(int);

void gameMode(int, Dominoe **);

void printMyDominoes(Dominoe **);

void printPutedDominoesInfo(Dominoe **);

void printHelp();

int ident(char *);
int parseNr(char *, int);
int parsePos(char *);

Dominoe *parseDominoe(int);
Dominoe **getPutedDominoes(int);

char *getDominoesLeft(int);

Dominoe **initializeMyDominoes(int);

int main() {
    int sock;
    struct addrinfo hints, * res;
    char *msg;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("192.168.1.165", "1111", &hints, &res);

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
	perror("socket");
	exit(1);
    }

    connect(sock, res->ai_addr, res->ai_addrlen);
    
    msg = getMessage(sock, 0);
    printf("%s\n", msg);

    printf("Vardas: ");
    msg = readLine();
    sendMessage(sock, 5, "/reg ");
    sendMessage(sock, strlen(msg), msg);

    printf("Waiting for other user...\n");

    /*
     * Let's start the game!
     */
    
    /* Initialize my dominoes */
    Dominoe **my_dominoes = initializeMyDominoes(sock);
    
    /* Turn on game mode */
    gameMode(sock, my_dominoes);

    return 0;
}

void gameMode(int sock, Dominoe ** my_dominoes) {
    Dominoe ** puted_dominoes = (Dominoe **) malloc((DOMINOES_SIZE) * sizeof(Dominoe*));
    char * msg;
 
    printHelp();

    for (;;) {
        char * cmd = NULL;
        int put_number;
        int position = 0;
        int decision;

        for (;;) {
            bool is_aviable_turn = true;

            /* Get puted dominoes */
            puted_dominoes = getPutedDominoes(sock);
            printPutedDominoesInfo(puted_dominoes);
     
            /* Get how much dominoes left */
            char * dominoes_left = getDominoesLeft(sock);
            printf("Dominoes left: %s\n", dominoes_left);

            /* Print my dominoes */
            printMyDominoes(my_dominoes);
    
            msg = getMessage(sock, 4);

            if (strcmp(msg, "strt") == 0) {
                printf("Your turn!\n");                

                if (strcmp(dominoes_left, "00") == 0) {
                    /* Check if here is any aviable right turn */
                    if (isAviableTurn(puted_dominoes, my_dominoes))
                        sendMessage(sock, 5, "/cont");
                    else {
                        sendMessage(sock, 5, "/skip");
                        is_aviable_turn = false;
                    }
                }
                else {
                    sendMessage(sock, 5, "/cont");
                }
                if (is_aviable_turn ) {
                    cmd = readLine();
                    decision = ident(cmd);

                     if (decision == 0) {         
                         position = parsePos(cmd);
                         put_number = parseNr(cmd, 0) - 1;
            
                         if ((position == 0 || position == 1) && put_number > -1 && put_number < getDominoesSize(my_dominoes)) {
                             sendMessage(sock, 5, "/put ");
                             char to_send[3];
                             sprintf(to_send, "%i%i%i", my_dominoes[put_number]->value_1, my_dominoes[put_number]->value_2, position);
                             sendMessage(sock, 3, to_send);

                             msg = getMessage(sock, 1);

                             if (strcmp(msg, "1") == 0) {
                                 my_dominoes = removeMyDominoe(my_dominoes, put_number);
                                 if (getDominoesSize(my_dominoes) == 0) {
                                     sendMessage(sock, 5, "/iwin");
                                 }
                                 else
                                     sendMessage(sock, 5, "/cont");
                             }
                             else
                                 printf("IT IS WRONG TURN!!!");
                         }  
                         else {
                             sendMessage(sock, 5, "/null");
                         }
                    }
    
                    else if (decision == 1) {
                        sendMessage(sock, 5, "/null");
                        int nr;
                        nr = parseNr(cmd, 1);
                        if (nr > 0 && nr-1 < getDominoesSize(my_dominoes)) {
                            nr -= 1;
                            my_dominoes[nr] = rotateDominoe(my_dominoes[nr]);
                        }
                    }

                    else if (decision == 2) {
                        sendMessage(sock, 5, "/take");
                        msg = getMessage(sock, 1);
                        if (strcmp(msg, "1") == 0) {
                            Dominoe * dominoe = parseDominoe(sock);
                            my_dominoes = addDominoe(my_dominoes, dominoe);
                        }
                        else {
                            printf("Dominoes stack is empty!\n");
                        }
                    }
                    else if (decision == 3) {
                        sendMessage(sock, 5, "/null");
                        printHelp();
                        char *any_key;
                        scanf("%s", any_key);
                    }
                    else if (decision == 9)
                        exit(0);

                    else {
                        sendMessage(sock, 5, "/null");     
                    }
                }
            }
            else if (strcmp(msg, "lose") == 0) {
                printf("%s", getMessage(sock, 0));
                exit(0);
            }
            else if (strcmp(msg, "/win") == 0) {
                printf("%s", getMessage(sock, 0));
                exit(0);
            }
            else if (strcmp(msg, "/end") == 0) {
                int my_sum = countDominoesValues(my_dominoes);
                printf("My points count: %d\n", my_sum);
                char *sum = (char *) malloc(sizeof(char));
                if (my_sum > 9)
                    sprintf(sum, "%i", my_sum);
                if (my_sum < 10)
                    sprintf(sum, "0%i", my_sum);

                sendMessage(sock, 2, sum);

                msg = getMessage(sock, 4);

                if (strcmp(msg, "/win") == 0)
                    printf("%s", getMessage(sock, 0));
                if (strcmp(msg, "/eql") == 0)
                    printf("%s", getMessage(sock, 0));
                if (strcmp(msg, "lose") == 0)
                    printf("%s", getMessage(sock, 0));

                exit(0);
            }
            else {
                printf("Wait your turn!\n");
            }
        }
	printf("\n");
    }
}

void printMyDominoes(Dominoe **my_dominoes) {
    int my_dominoes_count, i;

    printf("+---- My dominoes ----+\n");
    my_dominoes_count = getDominoesSize(my_dominoes);
    for (i = 1; i <= my_dominoes_count; i++)
        printf("  %d   ", i);
    printf("\n");
    printDominoes(my_dominoes);
    printf("\n\n");
}

void printPutedDominoesInfo(Dominoe **puted_dominoes) {
    printf("\n\n+---- Puted dominoes ----+\n");
    printDominoes(puted_dominoes);
    printf("\n\n");
}

void printHelp() {
    printf("Instructions:\n");
    printf("  - put [number] [position] - put domineo\n");
    printf("    * [number] - your dominoe number\n");
    printf("    * [position] - default is left. r - right, l - left\n");
    printf("  - rotate [number] - rotate dominoe\n");
    printf("    * [number] - your dominoe number\n");
    printf("  - take - take new dominoe from stack\n");
    printf("  - /help - help\n\n");
}

Dominoe **initializeMyDominoes(int sock) {
    Dominoe ** my_dominoes = (Dominoe **) malloc((DOMINOES_SIZE) * sizeof(Dominoe*));
    int i;
    my_dominoes[0] = NULL;
    for (i = 0; i < 6; i++) {
        Dominoe * dominoe = parseDominoe(sock);
        my_dominoes[i] = dominoe;
    }
    my_dominoes[i] = NULL;

    return my_dominoes;
}

Dominoe *parseDominoe(int sock) {
    char *msg = getMessage(sock, 2);
    int first, second;

    first = msg[0] - '0';
    second = msg[1] - '0';

    Dominoe * dominoe = createDominoe(first, second);

    return dominoe;
}

Dominoe **getPutedDominoes(int sock) {
    Dominoe ** dominoes = (Dominoe **) malloc((DOMINOES_SIZE) * sizeof(Dominoe*));

    char *msg = getMessage(sock, 1);
    int index = 0;

    if (strcmp(msg, "0") == 0) {
        dominoes[index] = NULL;
    }
    if (strcmp(msg, "1") == 0) {
        msg = "";
        while (strcmp(msg, "99") != 0) {
            msg = getMessage(sock, 2);
            if (strcmp(msg, "99")) {
                int first, second;

                first = msg[0] - '0';
                second = msg[1] - '0';

                Dominoe * dominoe = createDominoe(first, second);
                dominoes[index] = dominoe;
                index++;
            }
        }
        dominoes[index] = NULL;
    }

    return dominoes;
}

char *getDominoesLeft(int sock) {
    char *msg = getMessage(sock, 2);
    return msg;
}

char *readLine() {
    char *readbuff = (char *)malloc(512);
    int pos = 0;
    
    while((readbuff[pos] = (char)getc(stdin)) != '\n') {
	pos++;
    }
    readbuff[pos] = '\0';
    
    return readbuff;
}

int ident(char *cmd) {
    if (strstr(cmd, "put"))
	return 0;
    if (strstr(cmd, "rotate "))
	return 1;
    if (strstr(cmd, "take"))
        return 2;
    if (strstr(cmd, "/help"))
        return 3;
    if (strstr(cmd, "/exit"))
        return 9;
    return -1;
}

int parseNr(char *cmd, int decision) {    
    int pos = 0;
    if (decision == 1)
	cmd += 7;
    if (decision == 0)
	cmd += 4;
    
    while (cmd[pos] != ' ' && cmd[pos] != '\0')
	pos++;

    if (pos > 0) {
        char * nr = (char *) malloc(pos + 1);	
        memcpy(nr, cmd, pos);
    
        if (atoi(nr))
            return atoi(nr);
    }

    return -1;
}

int parsePos(char *cmd) {
    int pos = 0;
    char ch_pos;

    cmd += 4;

    while (cmd[pos] != ' ' && cmd[pos] != '\0') {
	pos++;
    }
    cmd += pos + 1;

    pos = 0;
    while (cmd[pos] != ' ' && cmd[pos] != '\0') {
	ch_pos = cmd[pos];
	pos++;
    }
    
    if (ch_pos == '\0')
	return 0;
    if (ch_pos == 'l')
	return 0;
    if (ch_pos == 'L')
	return 0;
    if (ch_pos == 'r')
	return 1;
    if (ch_pos == 'R')
	return 1;

    return 2;
}

char *intToChar(int value) {
    char *nr = (char *) malloc(sizeof(char));
    sprintf(nr, "%i", value);
    return nr;
}
