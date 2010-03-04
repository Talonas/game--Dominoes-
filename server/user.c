#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "../dominoes/dominoes.h"
#include "../socket/socket.h"
#include "user.h"
#include "game.h"
#include "log.h"

User *createUser(int new_fd) {
    User *user = (User *) malloc(sizeof(User));

    user->name     = "/empty";
    user->sock     = new_fd;
    user->status   = 0;
    user->has_turn = 1;
    user->turn     = 0;

    return user;
}

User **removeUser(User **users, User *user) {
    int index = 0;
    int new_index;

    while (users[index]->sock != user->sock)
        index++;

    new_index = index + 1;

    while (users[new_index] != NULL) {
        users[index] = users[new_index];
        index++;
        new_index++;
    }
    users[index] = NULL;

    return users;
}

User *findUserBySock(User **users, int sock) {
    int index = 0;
    while (users[index] != NULL) {
	if (users[index]->sock == sock)
	    return users[index];
	index++;
    }
    return NULL;
}

User *findUserByName(User **users, char *name) {
    int index = 0;
    while (users[index] != NULL) {
	if (strcmp(users[index]->name, name) == 0)
	    return users[index];
	index++;
    }
    return NULL;
}

/*
 * user_1 - waits
 * user_2 - starts
 */
void sendTurnStatus(User *user_1, User *user_2) {
    sendMessage(user_1->sock, 4, "wait");
    sendMessage(user_2->sock, 4, "strt");
}
/*
 * user_1 - wins
 * user_2 - loses
 */
void sendWhoWins(User *user_1, User *user_2, const char *log_file) {
    sendMessage(user_1->sock, 4, "/win");
    sendMessage(user_1->sock, 8, "You win!");
    sendMessage(user_2->sock, 4, "lose");
    sendMessage(user_2->sock, 10, "You lose!\n");
    sendMessage(user_2->sock, strlen(user_1->name), user_1->name);
    sendMessage(user_2->sock, 4, " win");

    char *log_msg = (char *) malloc(512);
    sprintf(log_msg, "Game: %s vs %s - %s won", user_1->name, user_2->name, user_1->name);
    write_log(log_file, log_msg);
}

bool doesUserExists(User **users, char *name) {
    User *user = findUserByName(users, name);
    if (user != NULL)
	return true;
    return false;
}

bool lastUserBusy(User **users, User *user) {
    int index = 0;

    while (users[index] != NULL) {
        if (users[index]->status == 0 && users[index]->sock != user->sock && strcmp(users[index]->name, "/empty"))
            return false;
        index++;
    }

    return true;
}

bool hasEvenDominoe(User *user) {
    int index = 0;

    while (user->dominoes[index] != NULL) {
        if (user->dominoes[index]->value_1 == user->dominoes[index]->value_2)
            return true;
        index++;
    }

    return false;
}

int getLowestEvenDominoe(User *user) {
    int index = 0;
    int index_min = 0; 
    int min = 6;

    while (user->dominoes[index] != NULL) {
        if (user->dominoes[index]->value_1 == user->dominoes[index]->value_2)
            if (user->dominoes[index]->value_1 < min)
                index_min = index;
        index++;
    }

    return index_min;
}

int getLowestDominoe(User *user) {
    int index = 0;
    int min = 12;

    while (user->dominoes[index] != NULL) {
        int value = user->dominoes[index]->value_1 + user->dominoes[index]->value_2;
        if (value < min)
            min = value;
        index++;
    }

    return min;
}

int getUsersSize(User **users) {
    int index = 0;

    while (users[index] != NULL)
	index++;

    return index;
}

User **addUser(User **users, User *user) {
    int index = 0;
    while (users[index] != NULL)
	index++;
    
    users[index] = user;
    index++;
    users[index] = NULL;

    return users;
}

int getUsersIndex(User **users, User *user) {
    int index = 0;

    while (users[index] != NULL) {
        if (users[index]->sock == user->sock)
            return index;
        index++;
    }

    return index;
}

int getFreeUserIndex(User **users, User *user) {
    int index = 0;

    while (users[index] != NULL) {
        if (users[index]->status == 0 && users[index]->sock != user->sock && strcmp(users[index]->name, "/empty"))
            return index;
        index++;
    }

    return index;
} 
