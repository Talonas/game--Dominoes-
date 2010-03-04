#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dominoes.h"

bool canPutDominoe(Dominoe **dominoes, Dominoe *dominoe, int position) {
    
    if (getDominoesSize(dominoes) == 0)
	return true;

    if (position == 0) {
	if (dominoes[0]->value_1 == dominoe->value_2)
	    return true;
	return false;
    }
    if (position == 1) {
	int last_position = getDominoesSize(dominoes) - 1;

	if (dominoes[last_position]->value_2 == dominoe->value_1)
	    return true;
	return false;
    }
    return false;
}

bool isAviableTurn(Dominoe **stack_dominoes, Dominoe **dominoes) {
    int index = 0;
    int last_pos = getDominoesSize(stack_dominoes) - 1;

    if (last_pos > -1) {
	while (dominoes[index] != NULL) {
	    if (stack_dominoes[0]->value_1 == dominoes[index]->value_1)
		return true;
	    if (stack_dominoes[0]->value_1 == dominoes[index]->value_2)
		return true;
	    if (stack_dominoes[last_pos]->value_2 == dominoes[index]->value_1)
		return true;
	    if (stack_dominoes[last_pos]->value_2 == dominoes[index]->value_2)
		return true;
	    index++;
	}
	return false;
    }
    return true;
}
