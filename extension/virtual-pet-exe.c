#include <stdio.h>
#include <stdlib.h>
#include "pet.h"

int main(void) {
	hearts pet = new_pet();
        /*
         * INFINITE LOOP TO BE IMPLEMENTED
         */

	free_hearts(pet);
        return EXIT_SUCCESS;
}
