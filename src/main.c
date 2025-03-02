/*  
    UPLOAD TO GITHUB
    FOR TESTING. NOT TO SUBMIT.
    Name: Muhammad Ali
    main.c: Driver to test createCard, cardToString, errorToString, deleteCard, and validateCard
*/
#include "VCParser.h"
#include "LinkedListAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
   Card *myCard = NULL;
   //VCardErrorCode err = createCard("testCard-TruncBday.vcf", &myCard);
   //VCardErrorCode err = createCard("testCardProps-Groups.vcf", &myCard);
   char* theFilesName = "../testCard.vcf";
   VCardErrorCode error = createCard(theFilesName, &myCard);

    //success
    if (error == OK) {
        printf("Card created successfully.\n");
    } 
    //fail
    else {
        char* errorString = errorToString(error);
        printf("Error creating card: %s\n", errorString);
        free(errorString);
        deleteCard(myCard);
        return 1;
    }

    //VALID CARD CHECK
    //test validateCard on the created card
    VCardErrorCode validationError = validateCard(myCard);
    //success
    if (validationError == OK) {
        printf("validateCard returned OK for the created card\n");
    } 
    //fail
    else {
        char* errorString = errorToString(validationError);
        printf("validateCard returned error: %s\n", errorString);
        free(errorString);
    }

    char* cardStr = cardToString(myCard);     //print card contents as string
    if (cardStr != NULL) {
        printf("%s", cardStr);
        free(cardStr);
    }

    //write the card to a file
    error = writeCard("../NEWCARD.vcf", myCard);
    if (error != OK) {
        char* errorString = errorToString(error);
        printf("Error writing card: %s\n", errorString);
        free(errorString);
    } 
    else {
        printf("Card written successfully to NEWCARD.vcf\n");
    }


    //INVALID CARD CHECK
    //create a manually invalid card to test validateCard failure
    Card *invalidCard = calloc(1, sizeof(Card));
    if (invalidCard == NULL) {
        printf("Memory allocation error for invalidCard\n");
        deleteCard(myCard);
        return 1;
    }
    invalidCard->fn = calloc(1, sizeof(Property));
    if (invalidCard->fn == NULL) {
        printf("Memory allocation error for invalidCard->fn\n");
        free(invalidCard);
        deleteCard(myCard);
        return 1;
    }

    //set an invalid FN name instead of FN
    invalidCard->fn->name = malloc(strlen("XX") + 1);
    if (invalidCard->fn->name == NULL) {
        printf("Memory allocation error for invalidCard->fn->name\n");
        free(invalidCard->fn);
        free(invalidCard);
        deleteCard(myCard);
        return 1;
    }
    strcpy(invalidCard->fn->name, "XX");
    invalidCard->fn->values = initializeList(valueToString, deleteValue, compareValues);
    if (invalidCard->fn->values == NULL) {
        printf("Memory allocation error for invalidCard->fn->values\n");
        free(invalidCard->fn->name);
        free(invalidCard->fn);
        free(invalidCard);
        deleteCard(myCard);
        return 1;
    }
    char *invalidValue = malloc(strlen("Invalid Card Test") + 1);
    if (invalidValue == NULL) {
        printf("Memory allocation error for invalidValue\n");
        freeList(invalidCard->fn->values);
        free(invalidCard->fn->name);
        free(invalidCard->fn);
        free(invalidCard);
        deleteCard(myCard);
        return 1;
    }
    strcpy(invalidValue, "Invalid Card Test");
    insertBack(invalidCard->fn->values, invalidValue);

    validationError = validateCard(invalidCard);
    //fail
    if (validationError == OK) {
        printf("validateCard unexpectedly returned OK for the invalid card\n");
    }
    //success
    else {
        char* errorString = errorToString(validationError);
        printf("validateCard correctly returned error for invalid card: %s\n", errorString);
        free(errorString);
    }

    //deletecards
    deleteCard(invalidCard);
    deleteCard(myCard);    //clean
    return 0;
}
