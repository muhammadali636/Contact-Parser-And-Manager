/*
    VCParser.c
    Name: Muhammad Ali
*/

//header files
#include "VCParser.h" //also contains LinkedListAPI.h
#include "VCHelpers.h"


//validates a Card object. If the argument satisfies all Card rules, the function returns OK. Otherwise, the function returns an appropriate error.
VCardErrorCode validateCard(const Card *obj)
{
    /*validate two aspects of the Card struct:
    * It must match the specification in VCParser.h
    * It must further validate some - but not all - aspects of the vCard format.
    */
    
    //if card object is NULL return INV_CARD
    if (obj == NULL)
    {
        return INV_CARD;
    }
    //validate FN property check that fn || fn->name || fn->values exist and values are nonempty
    if (obj -> fn == NULL || obj -> fn -> name == NULL || obj -> fn -> values == NULL || getLength(obj -> fn -> values) == 0)
    {
        return INV_CARD;
    }
    //lowercase fn->name and compare to fn
    {
        char fnLowerName[256];
        int i = 0;
        while (obj -> fn -> name[i] != '\0' && i < 255)
        {
            fnLowerName[i] = tolower(obj -> fn -> name[i]);
            i++;
        }
        fnLowerName[i] = '\0';
        if (strcmp(fnLowerName, "fn") != 0)
        {
            return INV_CARD;
        }
    }
    //validate FN property parameters
    if (obj -> fn -> parameters != NULL)
    {
        ListIterator paramIterator = createIterator(obj -> fn -> parameters);
        Parameter *param = NULL;
        while ((param = (Parameter *)nextElement(&paramIterator)) != NULL)
        {
            if (param -> value == NULL || strlen(param -> value) == 0 || param -> name == NULL || strlen(param -> name) == 0)
            {
                return INV_PROP;
            }
        }
    }
    //check if optional properties list is initialized
    if (obj -> optionalProperties == NULL)
    {
        return INV_CARD;
    }
    int nPropertyOccurrenceCount = 0;
    ListIterator propertyIterator = createIterator(obj -> optionalProperties);
    Property *prop = NULL;
    while ((prop = (Property *)nextElement(&propertyIterator)) != NULL)
    {
        if (prop -> name == NULL || strlen(prop -> name) == 0)
        {
            return INV_PROP;
        }
        //create a lowercase copy of the property name manually
        char lowerName[256];
        int j = 0;
        while (prop -> name[j] != '\0' && j < 255)
        {
            lowerName[j] = prop -> name[j];
            j++;
        }
        lowerName[j] = '\0';
        for (int k = 0; lowerName[k] != '\0'; k++)
        {
            lowerName[k] = tolower(lowerName[k]);
        }
        //if property name is disallowed return corresponding error using a single compound condition
        if ((strcmp(lowerName, "version") == 0) || (strcmp(lowerName, "fn") == 0) || (strcmp(lowerName, "bday") == 0) || (strcmp(lowerName, "anniversary") == 0))
        {
            return (strcmp(lowerName, "version") == 0) ? INV_CARD : ((strcmp(lowerName, "fn") == 0) ? INV_PROP : INV_DT);
        }
        //if property is n it must appear only once and have exactly five values
        if (strcmp(lowerName, "n") == 0)
        {
            nPropertyOccurrenceCount++;
            if (nPropertyOccurrenceCount > 1 || prop -> values == NULL || getLength(prop -> values) != 5)
            {
                return INV_PROP;
            }
        }
        //check that property values list exists and is nonempty
        if (prop -> values == NULL || getLength(prop -> values) == 0)
        {
            return INV_PROP;
        }
        ListIterator valueIterator = createIterator(prop -> values);
        char *valueStr = NULL;
        while ((valueStr = (char *)nextElement(&valueIterator)) != NULL)
        {
            if (valueStr == NULL)
            {
                return INV_PROP;
            }
        }
        //validate property parameters if present
        if (prop -> parameters != NULL)
        {
            ListIterator propParamIterator = createIterator(prop -> parameters);
            Parameter *propParam = NULL;
            while ((propParam = (Parameter *)nextElement(&propParamIterator)) != NULL)
            {
                if (propParam -> name == NULL || strlen(propParam -> name) == 0 || propParam -> value == NULL || strlen(propParam -> value) == 0)
                {
                    return INV_PROP;
                }
            }
        }
    }
    //inline DateTime validation for birthday using compound condition
    if (obj -> birthday != NULL)
    {
        if ((obj -> birthday -> isText && (obj -> birthday -> date == NULL || obj -> birthday -> time == NULL || obj -> birthday -> text == NULL || strlen(obj -> birthday -> date) != 0 || strlen(obj -> birthday -> time) != 0 || obj -> birthday -> UTC)) ||
            (!obj -> birthday -> isText && (obj -> birthday -> text == NULL || strlen(obj -> birthday -> text) != 0 || obj -> birthday -> date == NULL || strlen(obj -> birthday -> date) == 0 || obj -> birthday -> time == NULL)))
        {
            return INV_DT;
        }
    }
    //inline DateTime validation for anniversary using compound condition
    if (obj -> anniversary != NULL)
    {
        if ((obj -> anniversary -> isText && (obj -> anniversary -> date == NULL || obj -> anniversary -> time == NULL || obj -> anniversary -> text == NULL || strlen(obj -> anniversary -> date) != 0 || strlen(obj -> anniversary -> time) != 0 || obj -> anniversary -> UTC)) ||
            (!obj -> anniversary -> isText && (obj -> anniversary -> text == NULL || strlen(obj -> anniversary -> text) != 0 || obj -> anniversary -> date == NULL || strlen(obj -> anniversary -> date) == 0 || obj -> anniversary -> time == NULL)))
        {
            return INV_DT;
        }
    }
    return OK;
}


//takes a Card object and saves it to a file in vCard format. Avoid calling cardToString because we need to serialize the Card.
VCardErrorCode writeCard(const char* fileName, const Card* obj) {
    //validate args
    if (fileName == NULL || obj == NULL) {
        return INV_CARD;
    }

    //open file
    FILE* fp = fopen(fileName, "w");
    if (fp == NULL) {
        return WRITE_ERROR;  //couldn't open file for writing
    }

    //Card must haveFN with at least one value
    if (obj->fn == NULL || getLength(obj->fn->values) == 0) {
        fclose(fp);
        return INV_CARD;
    }

    //write vCard lines (using fprintf, with \r\n line endings)
    if (fprintf(fp, "BEGIN:VCARD\r\n") < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }

    //Write VERSION line 4.0
    if (fprintf(fp, "VERSION:4.0\r\n") < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }

    //Write FN property
    char* fnValueString = (char*)getFromFront(obj->fn->values);
    //if fn is empty INVALID
    if (fnValueString == NULL || strcmp(fnValueString, "") == 0) {
        fclose(fp);
        return INV_CARD;
    }
    if (fprintf(fp, "FN:%s\r\n", fnValueString) < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }

    //optional properties
    ListIterator propIter = createIterator(obj->optionalProperties);
    Property* prop = NULL;
    while ((prop = (Property*)nextElement(&propIter)) != NULL) {
        char* propStr = propertyToString(prop);
        if (propStr != NULL) {
            if (fprintf(fp, "%s\r\n", propStr) < 0) {
                free(propStr);
                fclose(fp);
                return WRITE_ERROR;
            }
            free(propStr);
        }
    }

    //Write birthday (if existing)
    if (obj->birthday != NULL) {
        char* bdayStr = dateToString(obj->birthday);
        if (bdayStr != NULL && strcmp(bdayStr, "") != 0) {
            if (fprintf(fp, "BDAY:%s\r\n", bdayStr) < 0) {
                free(bdayStr);
                fclose(fp);
                return WRITE_ERROR;
            }
            free(bdayStr);
        }
    }

    //write Anniversary (if existing)
    if (obj->anniversary != NULL) {
        char* annStr = dateToString(obj->anniversary);
        if (annStr != NULL && strcmp(annStr, "") != 0) {
            if (fprintf(fp, "ANNIVERSARY:%s\r\n", annStr) < 0) {
                free(annStr);
                fclose(fp);
                return WRITE_ERROR;
            }
            free(annStr);
        }
    }

    //Write END:VCARD
    if (fprintf(fp, "END:VCARD\r\n") < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }
    //success, now close file and return OK
    fclose(fp); 
    return OK;
}


//reads a vCard file, unfolds any folded lines parses each line, and checks for the required BEGIN, VERSION, END, and FN properties.
VCardErrorCode createCard(char* fileName, Card** newCardObj) {
    //checks if the newCardObj pointer is NULL which if it return an invalid file error.
    if (newCardObj == NULL) {
        return INV_FILE;
    }
    *newCardObj = NULL;    //init newCardObj 
    //check if fileName is NULL or empty, return an invalid file error.
    if (fileName == NULL || strlen(fileName) == 0) {
        return INV_FILE;
    }

    //extension check
    int fileNameLength = (int)strlen(fileName);   //length of the file name.
    int dotIndex = -1;
    //find last dot in the file name to locate the extension.
    for (int i = fileNameLength - 1; i >= 0; i--) 
    {
        if (fileName[i] == '.') {
            dotIndex = i;
            break;
        }
    }
    //if no dot is found  MEANS NO extension; return an invalid file error.
    if (dotIndex < 0) {
        return INV_FILE;
    }
    //copy file ext into a buffer so we can check
    char extensionBuffer[32];
    int extensionIndex = 0;
    for (int j = dotIndex; fileName[j] != '\0' && extensionIndex < 31; j++, extensionIndex++) {
        extensionBuffer[extensionIndex] = fileName[j];
    }
    extensionBuffer[extensionIndex] = '\0';
    //-> lowercase
    for (int k = 0; extensionBuffer[k] != '\0'; k++) {
        if (extensionBuffer[k] >= 'A' && extensionBuffer[k] <= 'Z') {
            extensionBuffer[k] = (char)(extensionBuffer[k] + ('a' - 'A'));
        }
    }
    //check if the extension is .vcf or .vcard (I think we are just dealing with .vcf) if not, return an invalid file error.
    if (strcmp(extensionBuffer, ".vcf") != 0 && strcmp(extensionBuffer, ".vcard") != 0) {
        return INV_FILE;
    }

    FILE* fp = fopen(fileName, "r");    //open file in read, if fail return an invalid file error.
    if (fp == NULL) {
        return INV_FILE;
    }

    //alloc mem for a new Card obj. 
    Card* cardObj = calloc(1, sizeof(Card));
    //if alloc fails, close file and return error.
    if (!cardObj) {
        fclose(fp);
        return OTHER_ERROR;
    }
    
    //calloc mem for fn property of the card. if fail free the card,close  file and return error.
    cardObj->fn = calloc(1, sizeof(Property));
    if (!cardObj->fn) 
    {
        free(cardObj);
        fclose(fp);
        return OTHER_ERROR;
    }
    
    //init the list for optional properties (from linkedlistapi.h). if fails free allocated memory, close  file, and return error.
    cardObj->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);
    if (!cardObj->optionalProperties) 
    {
        free(cardObj->fn);
        free(cardObj);
        fclose(fp);
        return OTHER_ERROR;
    }

    bool foundBegin = false, foundEnd = false, foundVersion = false, foundFn = false; //flags to track  required properties
    VCardErrorCode lastErr = OK; //variable for the last error.

    char lineBuffer[2048] = "";    //buffer to store unfolded lines
    int currentBufferLength = 0; //counter for the current buffer length.

    //Read every line of the file.
    while (!feof(fp)) {
        char inputLine[1024];
        inputLine[0] = '\0';
        if (fgets(inputLine, sizeof(inputLine), fp) == NULL) {
            break;
        }
        //check if line contains \r (Carriage return - go to start of line). if not, close file, delete the card, and return an error.
        if (strchr(inputLine, '\r') == NULL) 
        {
            fclose(fp);
            deleteCard(cardObj);
            return INV_CARD;
        }
        //remove \n and \r (carriage return) from the end of the line.
        int inputLineLength = (int)strlen(inputLine);
        while (inputLineLength > 0 && (inputLine[inputLineLength - 1] == '\n' || inputLine[inputLineLength - 1] == '\r')) {
            inputLine[inputLineLength - 1] = '\0';
            inputLineLength--;
        }
        //skip empty lines
        if (inputLineLength == 0) {
            continue;
        }
        //check if the line starts with space or tab.
        if (inputLine[0] == ' ' || inputLine[0] == '\t') 
        {
            //skip first whitespce
            int skipCount = 0;
            while (inputLine[skipCount] == ' ' || inputLine[skipCount] == '\t') {
                skipCount++;
            }
            //append  rest of the line to the lineBuffer.
            int copyIndex = skipCount;
            while (inputLine[copyIndex] != '\0' && currentBufferLength < 2047) {
                lineBuffer[currentBufferLength++] = inputLine[copyIndex++];
            }
            lineBuffer[currentBufferLength] = '\0';
        } 
        else 
        {
            //process data in lineBuffer from prev unfolded lines
            if (strlen(lineBuffer) > 0) 
            {
                parseLine(cardObj, lineBuffer,&foundBegin, &foundEnd,&foundVersion, &foundFn, &lastErr,newCardObj);
                //if parsing returns error close file and return error.
                if (lastErr != OK) {
                    fclose(fp);
                    return lastErr;
                }
                //If END property found, exit the loop.
                if (foundEnd) {
                    break;
                } 
            }
            //reset buffer for new line.
            currentBufferLength = 0;
            lineBuffer[0] = '\0';
            int index = 0;
            while (inputLine[index] != '\0' && currentBufferLength < 2047) {
                lineBuffer[currentBufferLength++] = inputLine[index++];
            }
            lineBuffer[currentBufferLength] = '\0';
            
            //check if line starts with "END:VCARD" and set flag if it does.
            char* endTagPosition = strstr(lineBuffer, "END:VCARD");
            if (endTagPosition != NULL && endTagPosition == lineBuffer) {
                foundEnd = true;
            }
        }
    }

    //after reading file, if there is any remaining buffered line and the END property hasn't been found PARSE the remaining line.
    if (!foundEnd && strlen(lineBuffer) > 0) 
    {
        parseLine(cardObj,lineBuffer, &foundBegin,&foundEnd,  &foundVersion, &foundFn, &lastErr, newCardObj);
        //if parsing returns error, close file and return error.
        if (lastErr != OK) {
            fclose(fp);
            return lastErr;
        }
    }
    fclose(fp); //close since we dont need it anymore.

    //check if any of the required properties: BEGIN, END, or VERSION, are missing. If they are return invalid. 
    if (!foundBegin || !foundEnd || !foundVersion) 
    {
        deleteCard(cardObj);
        *newCardObj = NULL;
        return INV_CARD;
    }
    //check if required FN property is missing.
    if (!foundFn) {
        deleteCard(cardObj);
        *newCardObj = NULL;
        return INV_PROP;
    }

    *newCardObj = cardObj;   //set newCardObj to successfully created card 
    return OK; //return success if everything went well
}


//deletecard: free all memory allocated for the Card.
void deleteCard(Card* obj) 
{
    //if the Card object is NULL --> do nothing and return immediately (since we have nothing).
    if (obj == NULL) {
        return;
    }
    //if fn property exists --> free its memory and set to NULL.
    if (obj->fn) {
        deleteProperty(obj->fn);
        obj->fn = NULL;
    }
    //if list of optional properties exists --> free the list aand SET TO NULL.
    if (obj->optionalProperties) {
        freeList(obj->optionalProperties);
        obj->optionalProperties = NULL;
    }
    //if bday property exists --> free its memory (using deleteDate) and SET TO NULLL.
    if (obj->birthday) {
        deleteDate(obj->birthday);
        obj->birthday = NULL;
    }
    //if ann property --> free its memory (using deleteDate) and SET TO NULL.
    if (obj->anniversary) {
        deleteDate(obj->anniversary);
        obj->anniversary = NULL;
    }
    free(obj); //free mem for card object itself.
}

//card to string literally.
char* cardToString(const Card* obj) {
    if (!obj) 
    {
        char* nullCardString = calloc(25, sizeof(char));
        if (nullCardString) {
            strcpy(nullCardString, "NULL Card\n");
        }
        return nullCardString;
    }
    char buffer[2048] = "";
    if (!obj->fn || !obj->fn->values) {
        strcat(buffer, "FN: <not set>\n");
    } 
    else {
        char* fnValueString = (char*)getFromFront(obj->fn->values);
        if (!fnValueString) {
            strcat(buffer, "FN: <empty>\n");
        } 
        else 
        {
            strcat(buffer, "FN: ");
            strcat(buffer, fnValueString);
            strcat(buffer, "\n");
        }
    }
    ListIterator listIterator = createIterator(obj->optionalProperties);
    Property* propertyData = NULL;
    while ((propertyData = (Property*)nextElement(&listIterator)) != NULL) 
    {
        char* propertyString = propertyToString(propertyData);
        if (propertyString) 
        {
            strcat(buffer, propertyString);
            strcat(buffer, "\n");
            free(propertyString);
        }
    }
    if (obj->birthday) 
    {
        char* birthdayString = dateToString(obj->birthday);
        strcat(buffer, "BIRTHDAY: ");
        strcat(buffer, birthdayString);
        strcat(buffer, "\n");
        free(birthdayString);
    }
    if (obj->anniversary) 
    {
        char* anniversaryString = dateToString(obj->anniversary);
        strcat(buffer, "ANNIVERSARY: ");
        strcat(buffer, anniversaryString);
        strcat(buffer, "\n");
        free(anniversaryString);
    }
    char* resultString = calloc(strlen(buffer) + 1, sizeof(char));
    if (resultString) {
        strcpy(resultString, buffer);
    }
    return resultString;
}

//string rep of a VCardErrorCode.
char* errorToString(VCardErrorCode err) 
{
    const char* message = NULL;
    switch (err) {
        case OK:          
            message = "OK"; 
            break;
        case INV_FILE:    
            message = "INV_FILE"; 
            break;
        case INV_CARD:    
            message = "INV_CARD"; 
            break;
        case INV_PROP:    
            message = "INV_PROP"; 
            break;
        case INV_DT:      
            message = "INV_DT"; 
            break;
        case WRITE_ERROR: 
            message = "WRITE_ERROR"; 
            break;
        case OTHER_ERROR: 
            message = "OTHER_ERROR"; 
            break;
        default:          
            message = "Invalid error code"; 
            break;
    }
    char* returnMessage = calloc(strlen(message) + 1, sizeof(char));
    if (returnMessage) {
        strcpy(returnMessage, message);
    }
    return returnMessage;
}
