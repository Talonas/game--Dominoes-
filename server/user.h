#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include "user.h"
#include "../dominoes/dominoes.h"

typedef struct {
    char * name;
    int sock;
    int status;  // 0 - isn't playing, 1 - is playing
    int has_turn; // 1 - has, 0 - don't have
    int turn;
    Dominoe ** dominoes;
} User;

User **addUser(User **, User *);

User **removeUser(User **, User *);

User *createUser(int);

User *prepareUserForPlay(User *);

User *findUserBySock(User **, int);

User *findUserByName(User **, char *);

void sendUsersList(User **, int);

/*
 * user_1 - waits
 * user_2 - starts
 */
void sendTurnStatus(User *, User *);

/*
 * user_1 - wins
 * user_2 - loses
 */
void sendWhoWins(User *, User *, const char *);

bool doesUserExists(User **, char *);

bool lastUserBusy(User **, User *);

bool hasEvenDominoe(User *);

int getLowestEvenDominoe(User *);

int getLowestDominoe(User *);

int getUsersSize(User **);

int getUsersIndex(User **, User *);

int getFreeUserIndex(User **, User *);

#endif
