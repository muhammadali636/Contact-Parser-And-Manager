/*
    VCParser.c
    Name: Muhammad Ali
*/

//header files
#include "VCParser.h" //also contains LinkedListAPI.h
#include "VCHelpers.h"


//A2M2: Validates a Card object. Returns OK if card is valid, else returns an error.
VCardErrorCode validateCard(const Card *obj)
{
    /*validate two aspects of the Card struct:
    * It must match the specification in VCParser.h
    * It must further validate some - but not all - aspects of the vCard format.
    */
    //all prop names. considerd putting this in .h file but its fine here for now.
    char *validPropNames[31] = {
        "FN", "N", "GENDER", "NICKNAME", "KIND", "PHOTO", "ADR", "TEL", "EMAIL", "IMPP", "LANG", "TZ", "GEO", "TITLE", "ROLE", "LOGO", "ORG", 
        "MEMBER", "RELATED", "CATEGORIES", "NOTE", "PRODID", "REV",  "SOUND", "UID", "CLIENTPIDMAP", "URL", "KEY", "FBURL", "CALADRURI", "CALURI"
    };

    //NULL card --> INV CARD
    if (obj == NULL) {
        return INV_CARD;
    }
    //check fn exist (has 1 value must)
    if (obj->fn == NULL || obj->fn->name == NULL || obj->fn->values == NULL || getLength(obj->fn->values) == 0) {
        return INV_CARD;
    }
    
    char fnLowerName[256];    //lowercase copy of fn -> name and check it equals "fn".
    int i = 0;
    while (obj->fn->name[i] && i < 255) {
        fnLowerName[i] = tolower(obj->fn->name[i]);
        i++;
    }
    fnLowerName[i] = '\0';
    if (strcmp(fnLowerName, "fn") != 0) {
        return INV_CARD;
    }

    //FN prop check (existing)
    if (obj->fn->parameters != NULL) 
    {
        ListIterator paramIterator = createIterator(obj->fn->parameters);
        Parameter *param = NULL;
        while ((param = (Parameter *)nextElement(&paramIterator)) != NULL) 
        {
            if (param->name == NULL || param->value == NULL || strlen(param->value) == 0 || strlen(param->name) == 0) {
                return INV_PROP;
            }
        }
    }

    //check that optionalProperties list is set.
    if (obj->optionalProperties == NULL) {
        return INV_CARD;
    }

    int namePropertyOccurrenceCount = 0;
    int kindPropertyOccurrenceCount = 0; //count for KIND property
    ListIterator propertyIterator = createIterator(obj->optionalProperties);
    Property *prop = NULL;

    while ((prop = (Property *)nextElement(&propertyIterator)) != NULL) 
    {
        if (prop -> name == NULL || strlen(prop -> name) == 0) {
            return INV_PROP;
        }
        //make a lowercase copy of property name.
        char lowerName[256];
        int j = 0;
        while (prop -> name[j] && j < 255) {
            lowerName[j] = tolower(prop->name[j]);
            j++;
        }
        lowerName[j] = '\0';
        
        //DONT allow these property names.
        if ((strcmp(lowerName, "version") == 0) || (strcmp(lowerName, "fn") == 0) ||  (strcmp(lowerName, "bday") == 0) ||  (strcmp(lowerName, "anniversary") == 0)) 
        {
            if (strcmp(lowerName, "version") == 0) {
                return INV_CARD;
            }
            if (strcmp(lowerName, "fn") == 0) {
                return INV_PROP;
            }
            return INV_DT;
        }

        //checks if property name is allowed
        int allowed = 0;
        for (int k = 0; k < NUMBER_OF_PROPERTIES; k++) 
        {
            char lowerValid[256];
            int idx = 0;
            //lowercase them.
            while (validPropNames[k][idx] && idx < 255) 
            {
                lowerValid[idx] = tolower(validPropNames[k][idx]);
                idx++;
            }
            lowerValid[idx] = '\0';
            if (strcmp(lowerName, lowerValid) == 0) 
            {
                allowed = 1;
                break;
            }
        }
        if (!allowed) {
            return INV_PROP;
        }
        
        //for property n  appear once and have exact 5 val
        if (strcmp(lowerName, "n") == 0) 
        {
            namePropertyOccurrenceCount++;
            if (namePropertyOccurrenceCount > 1 || prop->values == NULL || getLength(prop -> values) != 5) {
                return INV_PROP;
            }
        }
        //proper kind canonly appear once
        if (strcmp(lowerName, "kind") == 0) 
        {
            kindPropertyOccurrenceCount++;
            if (kindPropertyOccurrenceCount > 1) {
                return INV_PROP;
            }
        }
        //makes sure property has a nonempty list of values.
        if (prop->values == NULL || getLength(prop->values) == 0) {
            return INV_PROP;
        }
        ListIterator valueIterator = createIterator(prop->values);
        char *valueStr = NULL;
        while ((valueStr = (char *)nextElement(&valueIterator)) != NULL) 
        {
            if (valueStr == NULL) {
                return INV_PROP;
            }
        }
        //check property parameters if they exist.
        if (prop->parameters != NULL) 
        {
            ListIterator propParamIterator = createIterator(prop->parameters);
            Parameter *propParam = NULL;
            while ((propParam = (Parameter *)nextElement(&propParamIterator)) != NULL) {
                if (propParam->name == NULL || strlen(propParam->name) == 0 ||
                    propParam->value == NULL || strlen(propParam->value) == 0) {
                    return INV_PROP;
                }
            }
        }
    }

    //val birthday DateTime (existing.)
    if (obj->birthday != NULL) 
    {
        if (obj->birthday->isText) 
        {
            //Text DateTime ONLY TEXT and no date or time, and UTC must be false.
            if (obj->birthday->text == NULL || strlen(obj->birthday->text) == 0) {
                return INV_DT;
            }
            if ((obj->birthday->date != NULL && strlen(obj->birthday->date) > 0) || (obj->birthday->time != NULL && strlen(obj->birthday->time) > 0) ||obj->birthday->UTC) {
                return INV_DT;
            }
        } 
        else 
        {
            //structed DateTime must not have text and ONLY date or time.
            if (obj->birthday->text != NULL && strlen(obj->birthday->text) > 0) {
                return INV_DT;
            }
            int dateEmpty = (obj->birthday->date == NULL || strlen(obj->birthday->date) == 0);
            int timeEmpty = (obj->birthday->time == NULL || strlen(obj->birthday->time) == 0);
            if (dateEmpty && timeEmpty) {
                return INV_DT;
            }
        }
    }
    //val anniversary DateTime (if exist)
    if (obj->anniversary != NULL) 
    {
        if (obj->anniversary->isText) 
        {
            //txt DateTime must have txt, and no date or time, and UTC FALSE.
            if (obj->anniversary->text == NULL || strlen(obj->anniversary->text) == 0) {
                return INV_DT;
            }
            if ((obj->anniversary->date != NULL && strlen(obj->anniversary->date) > 0) ||  (obj->anniversary->time != NULL && strlen(obj->anniversary->time) > 0) || obj->anniversary->UTC) {
                return INV_DT;
            }
        } 
        else 
        {
            //structed DateTime must not have text and must have a date or time.
            if (obj -> anniversary->text != NULL && strlen(obj->anniversary->text) > 0) {
                return INV_DT;
            }
            int dateEmpty = (obj->anniversary->date == NULL || strlen(obj->anniversary->date) == 0);
            int timeEmpty = (obj->anniversary->time == NULL || strlen(obj->anniversary->time) == 0);
            if (dateEmpty && timeEmpty) {
                return INV_DT;
            }
        }
    }
    return OK;
}

//A2M1: Writes a Card object to a file in vCard format. Do not use cardToString.
VCardErrorCode writeCard(const char* fileName, const Card* obj) 
{
    //check fileName NULL MEANS return WRITE_ERROR.
    if (fileName == NULL) {
        return WRITE_ERROR;
    }
    //check card. NULL MEANS return WRITE_ERROR.
    if (obj == NULL) {
        return WRITE_ERROR;
    }
    
    //open file in w
    FILE* fp = fopen(fileName, "w");
    if (fp == NULL) {
        return OTHER_ERROR; //file open error.
    }
    
    //card must have an FN ww/ atleast 1 val
    if (obj->fn == NULL || getLength(obj -> fn -> values) == 0) {
        fclose(fp);
        return INV_CARD;
    }
    
    //write BEGIN and VERSION lines.
    if (fprintf(fp, "BEGIN:VCARD\r\n") < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }
    if (fprintf(fp, "VERSION:4.0\r\n") < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }
    
    //FN property.
    char* fnValueString = (char*)getFromFront(obj -> fn ->values);
    if (fnValueString == NULL || strcmp(fnValueString, "") == 0) 
    {
        fclose(fp);
        return INV_CARD;
    }
    if (fprintf(fp, "FN:%s\r\n", fnValueString) < 0) {
        fclose(fp);
        return WRITE_ERROR;
    }
    
    //opt properties.
    ListIterator propIter = createIterator(obj -> optionalProperties);
    Property* prop = NULL;
    while ((prop = (Property*)nextElement(&propIter)) != NULL) 
    {
        char* propStr = propertyToString(prop);
        if (propStr != NULL) {
            if (fprintf(fp, "%s\r\n", propStr) < 0) 
            {
                free(propStr);
                fclose(fp);
                return WRITE_ERROR;
            }
            free(propStr);
        }
    }
    
    //birthday.
    if (obj -> birthday != NULL) 
    {
        if (obj->birthday->isText) 
        {
            //For text DateTime, use VALUE=text.
            if (obj->birthday->text != NULL && strlen(obj-> birthday -> text) > 0) 
            {
                if (fprintf(fp, "BDAY;VALUE=text:%s\r\n", obj-> birthday -> text) < 0) 
                {
                    fclose(fp);
                    return WRITE_ERROR;
                }
            }
        } 
        else 
        {
            //for structed datetime make str from date/time
            char dtStr[256] = "";
            int hasDate = (obj -> birthday -> date != NULL && strlen(obj-> birthday ->date) > 0);
            int hasTime = (obj ->birthday-> time != NULL && strlen(obj -> birthday -> time) > 0);
            if (hasDate) 
            {
                strcpy(dtStr, obj->birthday->date);
                if (hasTime)
                {
                    strcat(dtStr, "T");
                    strcat(dtStr, obj->birthday->time);
                    if (obj->birthday->UTC) {
                        strcat(dtStr, "Z");
                    }
                }
            } 
            else if (hasTime) 
            {
                //TIMEONLY output with lead 'T"
                strcpy(dtStr, "T");
                strcat(dtStr, obj->birthday->time);
            }
            if (strlen(dtStr) > 0) 
            {
                if (fprintf(fp, "BDAY:%s\r\n", dtStr) < 0) {
                    fclose(fp);
                    return WRITE_ERROR;
                }
            }
        }
    }
    
    //anniversaire.
    if (obj->anniversary != NULL) 
    {
        if (obj->anniversary->isText) 
        {
            if (obj->anniversary->text != NULL && strlen(obj -> anniversary-> text) > 0) 
            {
                if (fprintf(fp, "ANNIVERSARY;VALUE=text:%s\r\n", obj->anniversary->text) < 0) 
                {
                    fclose(fp);
                    return WRITE_ERROR;
                }
            }
        } 
        else 
        {
            char annStr[256] = "";
            int hasDate = (obj->anniversary->date != NULL && strlen(obj -> anniversary->date) > 0);
            int hasTime = (obj->anniversary->time != NULL && strlen(obj->anniversary->time) > 0);
            if (hasDate) 
            {
                strcpy(annStr, obj->anniversary->date);
                if (hasTime) {
                    strcat(annStr, "T");
                    strcat(annStr, obj->anniversary->time);
                    if (obj->anniversary->UTC) {
                        strcat(annStr, "Z");
                    }
                }
            } 
            else if (hasTime) {
                strcpy(annStr, obj->anniversary->time);
            }
            if (strlen(annStr) > 0) 
            {
                if (fprintf(fp, "ANNIVERSARY:%s\r\n", annStr) < 0) 
                {
                    fclose(fp);
                    return WRITE_ERROR;
                }
            }
        }
    }
    
    //Write footer.
    if (fprintf(fp, "END:VCARD\r\n") < 0) 
    {
        fclose(fp);
        return WRITE_ERROR;
    }
    
    fclose(fp);
    return OK;
}


//reads a vCard file, unfolds any folded lines parses each line, and checks for required BEGIN, VERSION, END, and FN properties.
VCardErrorCode createCard(char* fileName, Card** newCardObj) 
{
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
        if (fileName[i] == '.') 
        {
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

    bool foundBegin = false, foundEnd = false, foundVersion = false, foundFn = false; //flags to track req properties
    VCardErrorCode lastErr = OK; //variable for the last error.

    char lineBuffer[2048] = "";    //buffer to store unfolded lines
    int currentBufferLength = 0; //counter for the current buffer length.

    //Read every line of the file.
    while (!feof(fp)) 
    {
        char inputLine[1024];
        inputLine[0] = '\0';
        if (fgets(inputLine, sizeof(inputLine), fp) == NULL) {
            break;
        }
        //check if line contains \r (Carriage return - go to start of line). if not, close file, delete the card, and return an error.
        if (strchr(inputLine, '\r') == NULL) {
            fclose(fp);
            deleteCard(cardObj);
            return INV_CARD;
        }
        //remove \n and \r (carriage return) from the end of the line.
        int inputLineLength = (int)strlen(inputLine);
        while (inputLineLength > 0 && (inputLine[inputLineLength - 1] == '\n' || inputLine[inputLineLength - 1] == '\r')) 
        {
            inputLine[inputLineLength - 1] = '\0';
            inputLineLength--;
        }
        //skip empty lines
        if (inputLineLength == 0) {
            continue;
        }
        //check if the line starts with space or tab.
        //CASE lineunfold1 and lineunfold2: make sure it parses right no matter how it appears.
        if (inputLine[0] == ' ' || inputLine[0] == '\t') 
        {
            int skipCount = 0;
            //count all leading whitespace chars
            while (inputLine[skipCount] == ' ' || inputLine[skipCount] == '\t') {
                skipCount++;
            }
            //append any extra whitespace beyond first char PRESERVE INTENDED SPACES
            for (int i = 1; i < skipCount && currentBufferLength < 2047; i++) {
                lineBuffer[currentBufferLength++] = inputLine[i];
            }
            //append the rest of the line starting at the char after the 1st whitespace
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
        if (lastErr != OK) 
        {
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

