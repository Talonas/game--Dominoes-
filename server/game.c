#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "user.h"
#include "../dominoes/dominoes.h"
#include "../socket/socket.h"
#include "log.h"

Game **addGame(Game **games, Game *game) {
    int index = 0;
    
    while (games[index] != NULL)
        index++;

    games[index] = game;
    games[index+1] = NULL;

    return games;
}

Game **removeGame(Game **games, Game *game) {
    int index = 0;
    int new_index;

    while (games[index] != NULL) {
        if (game->player_1->sock == games[index]->player_1->sock && 
            game->player_2->sock == games[index]->player_2->sock) {
            break;
        }
        index++;
    }

    new_index = index + 1;
    while (games[new_index] != NULL) {
        games[index] = games[new_index];
        index++;
        new_index++;
    }

    return games;
}

Game *createGame(User **users, User *user, const char *log_file) {
    Game *game = (Game *) malloc(sizeof(Game));
    Dominoe **puted_dominoes = (Dominoe **) malloc((29) * sizeof(Dominoe*));
    puted_dominoes[0] = NULL;

    game->player_1 = user;
    game->player_2 = users[getFreeUserIndex(users, user)];

    game->stack_dominoes = generateDominoes();    
    game->puted_dominoes = puted_dominoes;

    char *log_msg = (char *) malloc(512);
    sprintf(log_msg, "Game: %s vs %s - new round", game->player_1->name, game->player_2->name);
    write_log(log_file, log_msg);

    return game;
}

Game *findGameByUser(Game **games, User *user) {
    int index = 0;

    while (games[index] != NULL) {
        if (games[index]->player_1->sock == user->sock)
            return games[index];
        if (games[index]->player_2->sock == user->sock)
            return games[index];
        index++;
    }

    return games[index];
}

Game *setWinner(Game *game, int fd, const char *log_file) {
    if (game->player_1->sock != fd)
        sendWhoWins(game->player_2, game->player_1, log_file);

    if (game->player_2->sock != fd)
        sendWhoWins(game->player_1, game->player_2, log_file);

    return game;
}

Game *connectTwoPlayers(User **users, User *user, const char *log_file) {
    Dominoe ***dominoe_arr = (Dominoe ***) malloc((2) * sizeof(Dominoe **));

    // Prepare 
    Game * game = createGame(users, user, log_file);
                         
    dominoe_arr = generateFirstDominoes(game->stack_dominoes);
    game->stack_dominoes = dominoe_arr[0];
    game->player_1->dominoes = dominoe_arr[1];
    sendFirstDominoes(game->player_1->dominoes, game->player_1->sock);
    sendPutedDominoes(game->puted_dominoes, game->player_1->sock);

    dominoe_arr = generateFirstDominoes(game->stack_dominoes);
    game->stack_dominoes = dominoe_arr[0];
    game->player_2->dominoes = dominoe_arr[1];
    sendFirstDominoes(game->player_2->dominoes, game->player_2->sock);
    sendPutedDominoes(game->puted_dominoes, game->player_2->sock);         

    sendDominoesLeft(game, game->player_1);
    sendDominoesLeft(game, game->player_2);

    // Who starts first?
    game = whoStarts(game);

    return game;
}

int getGameNumber(Game **games, User *user) {
    int index = 0;

    while (games[index] != NULL) {
        if (games[index]->player_1->sock == user->sock)
            return index;
        if (games[index]->player_2->sock == user->sock)
            return index;
        index++;
    }

    return index;
}


void sendNecessaryInformation(Game *game) {
    sendPutedDominoes(game->puted_dominoes, game->player_1->sock);
    sendDominoesLeft(game, game->player_1);
    sendPutedDominoes(game->puted_dominoes, game->player_2->sock);
    sendDominoesLeft(game, game->player_2);
}

void sendPutedDominoes(Dominoe ** puted_dominoes, int new_fd) {
    int dominoes_size = getDominoesSize(puted_dominoes);
    
    if (dominoes_size == 0) {
	sendMessage(new_fd, 1, "0");
    }

    if (dominoes_size > 0) {
	int index = 0;

        sendMessage(new_fd, 1, "1");

	while (puted_dominoes[index] != NULL) {
            char * dominoe_char = dominoeToChar(puted_dominoes[index]->value_1, puted_dominoes[index]->value_2);
	    sendMessage(new_fd, strlen(dominoe_char), dominoe_char );
            index++;
	}
        sendMessage(new_fd, 2, "99");
    }
}

void sendDominoesLeft(Game *game, User *user) {
    int size = getDominoesSize(game->stack_dominoes);
    char * size_char = malloc(sizeof(char));

    if (size < 10)
        sprintf(size_char, "0%i", size);
    else
        sprintf(size_char, "%i", size);

    sendMessage(user->sock, 2, size_char);
}

Game *whoStarts(Game *game) {
    if (hasEvenDominoe(game->player_1) && hasEvenDominoe(game->player_2)) {
        int index_1 = getLowestEvenDominoe(game->player_1);
        int index_2 = getLowestEvenDominoe(game->player_2);

        if (game->player_1->dominoes[index_1]->value_1 > game->player_2->dominoes[index_2]->value_1) {
            game->player_1->turn = 0;
            game->player_2->turn = 1;
            sendTurnStatus(game->player_1, game->player_2);
        }
        if (game->player_1->dominoes[index_1]->value_1 < game->player_2->dominoes[index_2]->value_1) {
            game->player_1->turn = 1;
            game->player_2->turn = 0;
            sendTurnStatus(game->player_2, game->player_1);
        }
    }
    else if (hasEvenDominoe(game->player_1) && !hasEvenDominoe(game->player_2)) {
        game->player_1->turn = 1;
        game->player_2->turn = 0;
        sendTurnStatus(game->player_2, game->player_1);
    }
    else if (hasEvenDominoe(game->player_2) && !hasEvenDominoe(game->player_1)) {
        game->player_1->turn = 0;
        game->player_2->turn = 1;
        sendTurnStatus(game->player_1, game->player_2);
    }
    else {
        int lowest_1 = getLowestDominoe(game->player_1);
        int lowest_2 = getLowestDominoe(game->player_2);
 
        if (lowest_1 > lowest_2) {
            game->player_1->turn = 0;
            game->player_2->turn = 1;
            sendTurnStatus(game->player_1, game->player_2);
        }
        if (lowest_1 < lowest_2) {
            sendTurnStatus(game->player_2, game->player_1);
            game->player_1->turn = 1;
            game->player_2->turn = 0;
        }
    }

    return game;
}

void sendFirstDominoes(Dominoe **dominoes, int new_fd) {
    int index = 0;

    while (dominoes[index] != NULL) {
        char *dominoe_char = dominoeToChar(dominoes[index]->value_1, dominoes[index]->value_2);
        sendMessage(new_fd, strlen(dominoe_char), dominoe_char);
        index++;
    }
}

void isWinnerOrNot(User *player_1, User *player_2, const char *log_file) {
    char *sum_1 = getMessage(player_1->sock, 2);
    char *sum_2 = getMessage(player_2->sock, 2);

    if (sum_1[0] > sum_2[0])
        sendWhoWins(player_1, player_2, log_file);
    else if (sum_1[0] < sum_2[0])
        sendWhoWins(player_2, player_1, log_file);
    else if (sum_1[0] == sum_2[0]) {
        if (sum_1[1] > sum_2[1])
            sendWhoWins(player_1, player_2, log_file);
        else if (sum_1[1] < sum_2[1])
            sendWhoWins(player_2, player_1, log_file);
        else
            sendEqual(player_2, player_1, log_file);
    }
}

void sendEqual(User *user_1, User *user_2, const char *log_file) {
    sendMessage(user_1->sock, 4, "/eql");
    sendMessage(user_2->sock, 4, "/eql");

    sendMessage(user_1->sock, 18, "The part is equal!");
    sendMessage(user_2->sock, 18, "The part is equal!");

    char *log_msg = (char *) malloc(512);
    sprintf(log_msg, "Game: %s vs %s - round draw", user_1->name, user_2->name);
    write_log(log_file, log_msg);
}