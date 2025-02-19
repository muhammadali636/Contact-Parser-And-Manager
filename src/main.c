/*  
    FOR TESTING.
    Name: Muhammad Ali
    main.c: Driver to test createCard, cardToString, errorToString, and deleteCard. Just alter the test .vcf file
*/
#include "VCParser.h"
#include "LinkedListAPI.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
   Card *myCard = NULL;
   //VCardErrorCode err = createCard("testCard-TruncBday.vcf", &myCard);
   //VCardErrorCode err = createCard("testCardProps-Groups.vcf", &myCard);
   char* theFilesName = "testCard.vcf";
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

    char* cardStr = cardToString(myCard);     //print card contents as string
    if (cardStr != NULL) {
        printf("%s", cardStr);
        free(cardStr);
    }

    //write the card to a file
    error = writeCard("NEWCARD.vcf", myCard);
    if (error != OK) {
        char* errorString = errorToString(error);
        printf("Error writing card: %s\n", errorString);
        free(errorString);
    } 
    else {
        printf("Card written successfully to NEWCARD.vcf\n");
    }

    deleteCard(myCard);    //clean
    return 0;
}
