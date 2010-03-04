#ifndef GAME_H
#define GAME_H

#include "user.h"
#include "../dominoes/dominoes.h"

typedef struct {
    User * player_1;
    User * player_2;
    Dominoe ** stack_dominoes;
    Dominoe ** puted_dominoes;
} Game;

Game **addGame(Game **, Game *);

Game **removeGame(Game **, Game *);

Game *createGame(User **, User *, const char *);

Game *findGameByUser(Game **, User *);

Game *setWinner(Game *, int, const char *);

Game *connectTwoPlayers(User **, User *, const char *);

int getGameNumber(Game **, User *);

void sendNecessaryInformation(Game *);

void sendPutedDominoes(Dominoe **, int);

void sendDominoesLeft(Game *, User *);

void sendFirstDominoes(Dominoe **, int);

Game *whoStarts(Game *);

void isWinnerOrNot(User *, User *, const char *);

void sendEqual(User *, User *, const char *);

#endif