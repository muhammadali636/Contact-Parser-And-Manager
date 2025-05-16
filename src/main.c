/*  
    Name: Muhammad Ali
    main.c: Driver to test functions
*/

#include <stdio.h>
#include <stdlib.h>
#include "VCParser.h"
#include "VCHelpers.h"

int main(void) {
    char* filename = "cards/testCardGroup.vcf";   //change this to whatever file to test.

    //parse the input vCard file
    Card* myCard = NULL;
    VCardErrorCode result = createCard(filename, &myCard); 
    
    if (result != OK) {
        char* errStr = errorToString(result);
        printf("Error reading file: %s\n", errStr);
        free(errStr);
        return 1;
    }

    printf("Successfully parsed vCard from %s\n", filename);

    //validate parsed vCard
    result = validateCard(myCard);
    if (result != OK) {
        char* errStr = errorToString(result);
        printf("vCard validation failed: %s\n", errStr);
        free(errStr);
        deleteCard(myCard);
        return 1;
    }
    printf("vCard is valid.\n");

    //write the validated vCard to the output file
    result = writeCard(filename, myCard);
    if (result != OK) {
        char* errStr = errorToString(result);
        printf("Error writing file: %s\n", errStr);
        free(errStr);
        deleteCard(myCard);
        return 1;
    }

    printf("Successfully wrote vCard to %s\n", filename);

    //clean up
    deleteCard(myCard);
    return 0;
}
