#ifndef DOMINOE_H
#define DOMINOE_H

typedef struct {
    int value_1;
    int value_2;
} Dominoe;

Dominoe ** putDominoe(Dominoe **, Dominoe *, int);

Dominoe ** removeMyDominoe(Dominoe **, int);

Dominoe ** removeDominoe(Dominoe **, int);

Dominoe ** replaceDominoes(Dominoe **, int);

Dominoe ** addDominoe(Dominoe **, Dominoe *);

Dominoe ** generateDominoes();

Dominoe ***generateFirstDominoes(Dominoe **);

Dominoe * createDominoe(int , int);

Dominoe * rotateDominoe(Dominoe *);

void printDominoe(Dominoe *);

void printPutedDominoes(Dominoe **);

void printDominoes(Dominoe **);

int getDominoesSize(Dominoe **);

int countDominoesValues(Dominoe **);

char *dominoeToChar(int, int);

#endif
