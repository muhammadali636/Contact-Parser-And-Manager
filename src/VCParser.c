/*
    VCParser.c
    Name: Muhammad Ali
    Student ID: 1115336
*/

//header files
#include "VCParser.h" //also contains LinkedListAPI.h
#include "VCHelpers.h"

//A2M2: Validates a Card object. Returns OK if card is valid, else returns an error.
VCardErrorCode validateCard(const Card* obj) {
    if (obj == NULL)
        return INV_CARD;

    /* Inline lower-case conversion for obj->fn->name */
    if (obj->fn == NULL || obj->fn->group == NULL || obj->fn->parameters == NULL ||
        getFromFront(obj->fn->values) == NULL) {
        return INV_CARD;
    }
    {
        int len = strlen(obj->fn->name);
        char lowerFn[len + 1];
        for (int i = 0; i < len; i++) {
            lowerFn[i] = tolower((unsigned char)obj->fn->name[i]);
        }
        lowerFn[len] = '\0';
        if (strcmp(lowerFn, "fn") != 0)
            return INV_CARD;
    }
    char* fnValue = (char*)getFromFront(obj->fn->values);
    if (fnValue == NULL || strlen(fnValue) == 0)
        return INV_CARD; // Primary FN must have a non-empty value

    if (obj->optionalProperties == NULL)
        return INV_CARD;

    /* Allowed property names (all lower-case) per vCard Sections 6.1 - 6.9.3 */
    const char* validProps[] = {
        "begin", "end", "source", "kind", "xml", "fn", "n", "nickname", "photo", "bday",
        "anniversary", "gender", "adr", "tel", "email", "impp", "lang", "tz", "geo",
        "title", "role", "logo", "org", "member", "related", "categories", "note",
        "prodid", "rev", "sound", "uid", "clientpidmap", "url", "version", "key",
        "fburl", "caladruri", "caluri"
    };
    const int numValidProps = sizeof(validProps) / sizeof(validProps[0]);

    ListIterator propIter = createIterator(obj->optionalProperties);
    Property* prop;
    int kindCount = 0, nCount = 0, prodidCount = 0, revCount = 0, uidCount = 0;
    int beginCount = 0, endCount = 0, genderCount = 0;

    while ((prop = nextElement(&propIter)) != NULL) {
        /* Basic structural check: property name and values must be non-NULL */
        if (prop->name == NULL || prop->values == NULL)
            return INV_PROP;
        if (getLength(prop->values) == 0)
            return INV_PROP;

        /* Check that not all entries in values are empty */
        int allEmpty = 1;
        ListIterator valIter = createIterator(prop->values);
        void* val;
        while ((val = nextElement(&valIter)) != NULL) {
            if (val != NULL && strlen((char*)val) > 0) {
                allEmpty = 0;
                break;
            }
        }
        if (allEmpty)
            return INV_PROP;

        /* Validate parameters: names and values must be non-NULL and non-empty */
        if (prop->parameters != NULL) {
            ListIterator paramIter = createIterator(prop->parameters);
            Parameter* param;
            while ((param = nextElement(&paramIter)) != NULL) {
                if (param->name == NULL || param->value == NULL ||
                    strlen(param->name) == 0 || strlen(param->value) == 0) {
                    return INV_PROP;
                }
            }
        }

        /* Create a lower-case copy of the property name inline */
        int nameLen = strlen(prop->name);
        char lowerName[nameLen + 1];
        for (int i = 0; i < nameLen; i++) {
            lowerName[i] = tolower((unsigned char)prop->name[i]);
        }
        lowerName[nameLen] = '\0';

        /* BDAY and ANNIVERSARY must not appear in optionalProperties */
        if (strcmp(lowerName, "bday") == 0 || strcmp(lowerName, "anniversary") == 0)
            return INV_DT;

        /* Cardinality and structural validations */
        if (strcmp(lowerName, "begin") == 0) {
            beginCount++;
            if (beginCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
        }
        if (strcmp(lowerName, "end") == 0) {
            endCount++;
            if (endCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
        }
        if (strcmp(lowerName, "version") == 0) {
            return INV_CARD;
        }
        if (strcmp(lowerName, "kind") == 0) {
            kindCount++;
            if (kindCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
        }
        if (strcmp(lowerName, "gender") == 0) {
            genderCount++;
            if (genderCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
        }
        if (strcmp(lowerName, "fn") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
        }
        if (strcmp(lowerName, "n") == 0) {
            nCount++;
            if (nCount > 1 || getLength(prop->values) != 5)
                return INV_PROP;
            int allEmptyN = 1;
            Node* valueNode = prop->values->head;
            while (valueNode != NULL) {
                char* value = (char*)valueNode->data;
                if (value != NULL && strlen(value) > 0) {
                    allEmptyN = 0;
                    break;
                }
                valueNode = valueNode->next;
            }
            if (allEmptyN)
                return INV_PROP;
        }
        if (strcmp(lowerName, "nickname") == 0) {
            if (getLength(prop->values) < 1)
                return INV_PROP;
            ListIterator nickIter = createIterator(prop->values);
            while ((val = nextElement(&nickIter)) != NULL) {
                if (strlen((char*)val) == 0)
                    return INV_PROP;
            }
        }
        if (strcmp(lowerName, "photo") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
        }
        if (strcmp(lowerName, "adr") == 0) {
            if (getLength(prop->values) != 7)
                return INV_PROP;
            int nonEmptyCount = 0;
            ListIterator adrIter = createIterator(prop->values);
            while ((val = nextElement(&adrIter)) != NULL) {
                if (strlen((char*)val) > 0)
                    nonEmptyCount++;
            }
            if (nonEmptyCount < 4)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
            ListIterator paramIter = createIterator(prop->parameters);
            Parameter* param;
            int labelCount = 0;
            while ((param = nextElement(&paramIter)) != NULL) {
                /* Inline lower-case conversion for parameter name */
                int plen = strlen(param->name);
                char lowerParam[plen + 1];
                for (int j = 0; j < plen; j++) {
                    lowerParam[j] = tolower((unsigned char)param->name[j]);
                }
                lowerParam[plen] = '\0';
                if (strcmp(lowerParam, "label") == 0) {
                    labelCount++;
                    if (labelCount > 1 || strlen(param->value) == 0)
                        return INV_PROP;
                }
            }
        }
        if (strcmp(lowerName, "tel") == 0) {
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "email") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "impp") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "lang") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "tz") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "geo") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
        }
        if (strcmp(lowerName, "title") == 0 || strcmp(lowerName, "role") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "logo") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "org") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "member") == 0) {
            int isGroup = 0;
            ListIterator kindIter = createIterator(obj->optionalProperties);
            Property* kindProp;
            while ((kindProp = nextElement(&kindIter)) != NULL) {
                int kindLen = strlen(kindProp->name);
                char lowerKind[kindLen + 1];
                for (int i = 0; i < kindLen; i++) {
                    lowerKind[i] = tolower((unsigned char)kindProp->name[i]);
                }
                lowerKind[kindLen] = '\0';
                if (strcmp(lowerKind, "kind") == 0) {
                    /* Convert the first value of the KIND property to lower-case */
                    char* kindVal = (char*)getFromFront(kindProp->values);
                    if (kindVal != NULL) {
                        int kvLen = strlen(kindVal);
                        char lowerKindVal[kvLen + 1];
                        for (int i = 0; i < kvLen; i++) {
                            lowerKindVal[i] = tolower((unsigned char)kindVal[i]);
                        }
                        lowerKindVal[kvLen] = '\0';
                        if (strcmp(lowerKindVal, "group") == 0) {
                            isGroup = 1;
                            break;
                        }
                    }
                }
            }
            if (!isGroup)
                return INV_PROP;
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "related") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "categories") == 0 || strcmp(lowerName, "note") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
        }
        if (strcmp(lowerName, "prodid") == 0) {
            prodidCount++;
            if (prodidCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "rev") == 0) {
            revCount++;
            if (revCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "sound") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            char* value = (char*)getFromFront(prop->values);
            if (value == NULL || strlen(value) == 0)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "uid") == 0) {
            uidCount++;
            if (uidCount > 1 || getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "clientpidmap") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "url") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "key") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        if (strcmp(lowerName, "fburl") == 0 ||
            strcmp(lowerName, "caladruri") == 0 ||
            strcmp(lowerName, "caluri") == 0) {
            if (getLength(prop->values) != 1)
                return INV_PROP;
            if (prop->parameters == NULL)
                return INV_PROP;
        }
        
        /* Finally, ensure the property name is one of the allowed names. */
        int isValidProp = 0;
        for (int i = 0; i < numValidProps; i++) {
            if (strcmp(lowerName, validProps[i]) == 0) {
                isValidProp = 1;
                break;
            }
        }
        if (!isValidProp)
            return INV_PROP;
    }

    /* Validate the DateTime fields for birthday and anniversary. */
    if (obj->birthday != NULL) {
        if (obj->birthday->isText) {
            if (obj->birthday->text == NULL || strlen(obj->birthday->text) == 0)
                return INV_DT;
            if (obj->birthday->UTC)
                return INV_DT;
            if (strlen(obj->birthday->date) > 0 || strlen(obj->birthday->time) > 0)
                return INV_DT;
        } else {
            if (strlen(obj->birthday->date) == 0 && strlen(obj->birthday->time) == 0)
                return INV_DT;
            if (strlen(obj->birthday->text) > 0)
                return INV_DT;
            if (strlen(obj->birthday->date) > 0) {
                int dateLen = strlen(obj->birthday->date);
                if (dateLen != 8 &&
                    !(dateLen == 6 && strcmp(obj->birthday->date, "--") == 0) &&
                    !(dateLen == 5 && strcmp(obj->birthday->date, "---") == 0))
                    return INV_DT;
            }
            if (strlen(obj->birthday->time) > 0 && strlen(obj->birthday->time) != 6)
                return INV_DT;
        }
    }
    if (obj->anniversary != NULL) {
        if (obj->anniversary->isText) {
            if (obj->anniversary->text == NULL || strlen(obj->anniversary->text) == 0)
                return INV_DT;
            if (obj->anniversary->UTC)
                return INV_DT;
            if (strlen(obj->anniversary->date) > 0 || strlen(obj->anniversary->time) > 0)
                return INV_DT;
        } else {
            if (strlen(obj->anniversary->date) == 0 && strlen(obj->anniversary->time) == 0)
                return INV_DT;
            if (strlen(obj->anniversary->text) > 0)
                return INV_DT;
            if (strlen(obj->anniversary->time) > 0 && strlen(obj->anniversary->date) == 0 && !obj->anniversary->UTC)
                return INV_DT;
            if (strlen(obj->anniversary->date) > 0) {
                int dateLen = strlen(obj->anniversary->date);
                if (dateLen != 8 &&
                    !(dateLen == 6 && strcmp(obj->anniversary->date, "--") == 0) &&
                    !(dateLen == 5 && strcmp(obj->anniversary->date, "---") == 0))
                    return INV_DT;
            }
            if (strlen(obj->anniversary->time) > 0 && strlen(obj->anniversary->time) != 6)
                return INV_DT;
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

