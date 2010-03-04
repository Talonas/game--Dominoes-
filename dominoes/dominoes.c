#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dominoes.h"

Dominoe * rotateDominoe(Dominoe *dominoe) {
    dominoe->value_1 ^= dominoe->value_2;
    dominoe->value_2 ^= dominoe->value_1;
    dominoe->value_1 ^= dominoe->value_2;

    return dominoe;
}

int getDominoesSize(Dominoe **dominoes) {
    int index = 0;
    int counter = 0;

    while (dominoes[index] != NULL) {
	counter++;
	index++;
    }

    return counter;
}

int countDominoesValues(Dominoe **dominoes) {
    int index = 0;
    int sum = 0;

    while (dominoes[index] != NULL) {
        sum += dominoes[index]->value_1;
        sum += dominoes[index]->value_2;
        index++;
    }

    return sum;
}

Dominoe ** replaceDominoes(Dominoe **dominoes, int index) {
    Dominoe ** replaced_dominoes = (Dominoe **) malloc((getDominoesSize(dominoes)) * sizeof(Dominoe*));
    int i = 0;
    int current = 0;

    while (current != index) {
	replaced_dominoes[current] = dominoes[current];
	current++;
    }

    i = current + 1;
    //i = current;
    while(dominoes[i] != NULL) {
	replaced_dominoes[current] = dominoes[i];
	current++;
	i++;
    }
    replaced_dominoes[current] = NULL;

    return replaced_dominoes;
}

Dominoe ***generateFirstDominoes(Dominoe ** dominoes) {
    Dominoe **new_dominoes = (Dominoe **) malloc((6) * sizeof(Dominoe*));
    Dominoe ***dominoe_arr = (Dominoe ***) malloc((2) * sizeof(Dominoe **));

    int i = 0;
    int dominoes_size = getDominoesSize(dominoes);

    for (i = 0; i < 6; i++) {
        int random_number;

        random_number = rand() % dominoes_size;
        Dominoe * dominoe = dominoes[random_number];	

        dominoes = removeDominoe(dominoes, random_number);

        new_dominoes[i] = dominoe;

        dominoes_size = dominoes_size - 1;
    }
    new_dominoes[6] = NULL;
  
    dominoe_arr[0] = dominoes;
    dominoe_arr[1] = new_dominoes;

    return dominoe_arr;
}

Dominoe ** addDominoe(Dominoe **dominoes, Dominoe *dominoe) {
    int size = getDominoesSize(dominoes);
    //Dominoe ** new_dominoes = (Dominoe **) malloc((size +2) * sizeof(Dominoe *));

    dominoes[size] = dominoe;
    dominoes[size+1] = NULL;

    return dominoes;
}

Dominoe ** generateDominoes() {
    Dominoe ** dominoes = (Dominoe **) malloc((29) * sizeof(Dominoe *));
    int i, j, pos = 0;
    
    for (i = 0; i < 7; i++) {
	for (j = i; j < 7; j++) {
	    Dominoe * dominoe = createDominoe(i, j);
	    dominoes[pos] = dominoe;
	    pos++;
	}
    }
    dominoes[pos] = NULL;

    return dominoes;
}

Dominoe ** removeMyDominoe(Dominoe **my_dominoes, int index) {
    my_dominoes = replaceDominoes(my_dominoes, index);

    return my_dominoes;
}

Dominoe ** removeDominoe(Dominoe **dominoes, int index) {
    dominoes = replaceDominoes(dominoes, index);
    return dominoes;
}

Dominoe ** putDominoe(Dominoe **dominoes, Dominoe *dominoe, int position) {
    int index = 0;
    Dominoe ** new_dominoes = (Dominoe **) malloc((getDominoesSize(dominoes) +2) * sizeof(Dominoe *));

    if (position == 0) {
	int new_index = 1;

	new_dominoes[0] = dominoe;
	while (dominoes[index] != NULL) {
	    new_dominoes[new_index] = dominoes[index];
	    new_index++;
	    index++;
	}
	new_dominoes[new_index] = NULL;
    }
    
    if (position == 1) {
	int size = getDominoesSize(dominoes);

	new_dominoes = dominoes;
	new_dominoes[size] = dominoe;
	size++;
	new_dominoes[size] = NULL;
    }

    return new_dominoes;
}

void printPutedDominoes(Dominoe **dominoes) {
    int index = 0;

    while (dominoes[index] != NULL) {
	printDominoe(dominoes[index]);
	index++;
    }
}

void printDominoes(Dominoe **dominoes) {
    int index = 0;

    while (dominoes[index] != NULL) {
	printDominoe(dominoes[index]);
	index++;
    }
}

Dominoe * createDominoe(int value_1, int value_2) {
    Dominoe * dominoe = (Dominoe*) malloc(sizeof(Dominoe));

    dominoe->value_1 = value_1;
    dominoe->value_2 = value_2;

    return dominoe;
}

void printDominoe(Dominoe *dominoe) {
    printf("[%d|%d] ", dominoe->value_1, dominoe->value_2);
}

char *dominoeToChar(int first, int second) {
    char *value = (char *) malloc(sizeof(char));
    sprintf(value, "%i%i", first, second);
    return value;
}
