/*
 * vchelpers.c: helper functions
 * name: Muhammad Ali
 * Student ID: 1115336
 */

//header files
#include "VCHelpers.h"


//prop
//free all mem alloced for prop structure
void deleteProperty(void* toBeDeleted) {
    //ret if the ptr is NULL
    if (!toBeDeleted) {
        return;
    }
    Property* propertyPtr = (Property*)toBeDeleted;     //input ptr to a prop ptr
    //free name str
    if (propertyPtr->name) {
        free(propertyPtr->name);
    }
    //free group str
    if (propertyPtr->group) {
        free(propertyPtr->group);
    }
    //free the list of params 
    if (propertyPtr->parameters) {
        freeList(propertyPtr->parameters);
    }
    //free list of vals 
    if (propertyPtr->values) {
        freeList(propertyPtr->values);
    }
    free(propertyPtr); //free prop struct
}

//cmp two prop structs (case insense), rets a negative, zero, or positive integer from cmp result.
int compareProperties(const void* first, const void* second) {
    //ret 0 if either ptr is NULL
    if (!first || !second) {
        return 0;
    }
    //input ptrs to prop ptrs
    const Property* property1 = (const Property*)first;
    const Property* property2 = (const Property*)second;

    //cmp group fields (treat NULL as empty str) 
    const char* group1;
    if (property1->group != NULL) {
        group1 = property1->group;
    }
    else {
        group1 = "";
    }
    const char* group2;
    if (property2->group != NULL)
        group2 = property2->group;
    else {
        group2 = "";
    }
    //cmp the two group strs char by char (ignore case)
    int index = 0;
    while (group1[index] != '\0' && group2[index] != '\0') {
        char char1 = group1[index];
        char char2 = group2[index];
        //to lowercase
        if (char1 >= 'A' && char1 <= 'Z') {
            char1 = (char)(char1 + ('a' - 'A'));
        }
        if (char2 >= 'A' && char2 <= 'Z') {
            char2 = (char)(char2 + ('a' - 'A'));
        }
        //ret the difference if chars differ
        if (char1 != char2) {
            return char1 - char2;
        }
        index++;
    }
    //if one group str is shorter ret the length diff
    if (group1[index] != group2[index]) {
        return group1[index] - group2[index];
    }

    //if groups match, cmp the prop names (case insense
    if (!property1->name || !property2->name) {
        return 0;
    }
    index = 0;
    while (property1->name[index] != '\0' && property2->name[index] != '\0') {
        char char1 = property1->name[index];
        char char2 = property2->name[index];
        //convert tolower
        if (char1 >= 'A' && char1 <= 'Z') {
            char1 = (char)(char1 + ('a' - 'A'));
        }
        if (char2 >= 'A' && char2 <= 'Z') {
            char2 = (char)(char2 + ('a' - 'A'));
        }
        //ret difference if chars differ
        if (char1 != char2) {
            return char1 - char2;
        }
        index++;
    }
    return property1->name[index] - property2->name[index];    //ret the difference in lengths 
}

//change a prop to a str in proper vCard format
char* propertyToString(void* prop) {
    //if prop pointer is NULL ret a default string
    if (!prop) {
        char* tempStr = calloc(25, sizeof(char));
        if (tempStr) {
            strcpy(tempStr, "(NULL property)");
        }
        //err handling
        else {
            return NULL;
        }
        return tempStr;
    }

    Property* propertyPtr = (Property*)prop; //input ptr to a prop ptr and 
    char buffer[1024] = ""; //create a buffer for the string

    //BUILDING header str
    //start directly with the property name.
    if (propertyPtr->name) {
        strcpy(buffer, propertyPtr->name);
    } 
    //append the prop name if it doesn't exist. 
    else {
        strcpy(buffer, "(no name)");
    }

    ///append param to str (if exists) in proper vCard format: ;paramName=paramValue
    if (propertyPtr->parameters && getLength(propertyPtr->parameters) > 0) {
        ListIterator iterator = createIterator(propertyPtr->parameters);
        void* parameterData = nextElement(&iterator);
        while (parameterData) {
            Parameter* parameterPtr = (Parameter*)parameterData;
            strcat(buffer, ";");
            strcat(buffer, parameterPtr->name);
            strcat(buffer, "=");
            strcat(buffer, parameterPtr->value);
            parameterData = nextElement(&iterator);
        }
    }

    strcat(buffer, ":");    //append colon to separate header from values

    //append val to str in proper vCard format (separate multiple values with semicolons)
    if (propertyPtr->values && getLength(propertyPtr->values) > 0) {
        ListIterator valueIterator = createIterator(propertyPtr->values);
        void* valueData = nextElement(&valueIterator);
        bool first = true;
        while (valueData) {
            char* valueStr = (char*)valueData;
            if (!first) {
                strcat(buffer, ";");
            }
            strcat(buffer, valueStr);
            first = false;
            valueData = nextElement(&valueIterator);
        }
    } 
    else {
        strcat(buffer, "");
    }

    //alloc mem for resulting str
    char* retStr = calloc(strlen(buffer) + 1, sizeof(char));
    if (retStr) {
        strcpy(retStr, buffer);
    }
    //err handling
    else {
        return NULL;
    }
    return retStr; //ret resulting str
}


//PARAMS
//free ALL mem allocated for a Parameter structure 
void deleteParameter(void* toBeDeleted) {
    //ret if the ptr is NULL
    if (!toBeDeleted) {
        return;
    }
    Parameter* parameterPtr = (Parameter*)toBeDeleted;     //input ptr to a param ptr
    //free the name str 
    if (parameterPtr->name) {
        free(parameterPtr->name);
    }
    //free the val str 
    if (parameterPtr->value) {
        free(parameterPtr->value);
    }
    free(parameterPtr); //free param struct
}

//cmp two param structs (case insense), rets a negative zero or positive integer based on the cmp
int compareParameters(const void* first, const void* second) {
    //ret 0 if either ptr is NULL
    if (!first || !second) {
        return 0;
    }
    //input ptrs to param ptrs
    const Parameter* parameter1 = (const Parameter*)first;
    const Parameter* parameter2 = (const Parameter*)second;
    //ret 0 if either param name is NULL
    if (!parameter1->name || !parameter2->name) {
        return 0;
    }

    //cmp param names (case insense)
    int index = 0;
    while (parameter1->name[index] != '\0' && parameter2->name[index] != '\0') {
        char char1 = parameter1->name[index];
        char char2 = parameter2->name[index];
        //convert tolower
        if (char1 >= 'A' && char1 <= 'Z') {
            char1 = (char)(char1 + ('a' - 'A'));
        }
        if (char2 >= 'A' && char2 <= 'Z') {
            char2 = (char)(char2 + ('a' - 'A'));
        }
        //ret -1 or 1 if chars differ
        if (char1 < char2) {
            return -1;
        }
        if (char1 > char2) {
            return 1;
        }
        index++; //increment index
    }
    return parameter1->name[index] - parameter2->name[index];    //ret the difference in length (if any)
}

//convert a Parameter to a string in the format "name=value"
char* parameterToString(void* param) {
    //if param is NULL, ret a def str
    if (!param) {
        char* tempStr = calloc(20, sizeof(char));
        if (tempStr) {
            strcpy(tempStr, "(NULL param)");
        }
        //err handling
        else {
            return NULL;
        }
        return tempStr;
    }
    //input ptr to param ptr and build str
    Parameter* parameterPtr = (Parameter*)param;
    char buff[256] = "";
    if (parameterPtr->name) {
        strcat(buff, parameterPtr->name);
    } else {
        strcat(buff, "(no name)");
    }
    strcat(buff, "=");
    if (parameterPtr->value) {
        strcat(buff, parameterPtr->value);
    } else {
        strcat(buff, "(no value)");
    }
    //alloc mem for and ret the resulting string
    char* retStr = calloc(strlen(buff) + 1, sizeof(char));
    if (retStr) {
        strcpy(retStr, buff);
    }
    else {
        //err handling
        return NULL;
    }
    return retStr;
}

//VALUE
//free val (which is just a string)
void deleteValue(void* toBeDeleted) {
    if (toBeDeleted) {
        free((char*)toBeDeleted);
    }
}

//cmp two strs (case insensitive)
int compareValues(const void* first, const void* second) {
    //ret 0 if either ptr is NULL
    if (!first || !second) {
        return 0;
    }
    //input ptrs to strs.
    const char* string1 = (const char*)first;
    const char* string2 = (const char*)second;
    //cmp char by char, ignore case
    int index = 0;
    while (string1[index] != '\0' && string2[index] != '\0') {
        char char1 = string1[index];
        char char2 = string2[index];
        if (char1 >= 'A' && char1 <= 'Z') {
            char1 = (char)(char1 + ('a' - 'A'));
        }
        if (char2 >= 'A' && char2 <= 'Z') {
            char2 = (char)(char2 + ('a' - 'A'));
        }
        if (char1 < char2) {
            return -1;
        }
        if (char1 > char2) {
            return 1;
        }
        index++;
    }
    return string1[index] - string2[index];     //ret the difference in length 
}

//ret cpy of a value str
char* valueToString(void* val) {
    //if val is NULL ret def string
    if (!val) {
        char* tempStr = calloc(20, sizeof(char));
        if (tempStr) {
            strcpy(tempStr, "(NULL value)");
        }
        else {
            //err handling
            return NULL;
        }
        return tempStr;
    }
    const char* originalStr = (const char*)val;    //input to str cast
    char* copyStr = calloc(strlen(originalStr) + 1, sizeof(char));
    if (copyStr) {
        strcpy(copyStr, originalStr);
    }
    return copyStr; //ret input str copy
}

//DATE:
//free all mam alloced for a DateTime struct and its fields (IF they exist)
void deleteDate(void* toBeDeleted) {
    //ret if ptr is NULL
    if (!toBeDeleted) {
        return;
    }
    DateTime* dt = (DateTime*)toBeDeleted; //input ptr to datetime ptr
    //free date string if it exists
    if (dt->date) {
        free(dt->date);
    }
    //free time str if it exists
    if (dt->time) {
        free(dt->time);
    }
    //free txt string 
    if (dt->text) {
        free(dt->text);
    }
    free(dt);    //free the DateTime struct itself
}

//cmp two DateTime structs, ret0
int compareDates(const void* first, const void* second) {
    return 0;
}

//convert a DateTime struct to a string
char* dateToString(void* date) {
    //if date pointer is NULL, ret a default string
    if (!date) {
        char* tempStr = calloc(15, sizeof(char));
        if (tempStr) {
            strcpy(tempStr, "(null date)");
        }
        else {
             //err handling
            return NULL;
        }
        return tempStr;
    }
     
    DateTime* dt = (DateTime*)date; //cast input pointer to a DateTime pointer
    char buf[256] = ""; //create a buffer for the string
    //build str with field labels and values
    strcat(buf, "DateTime(");
    strcat(buf, "isText=");
    if (dt->isText)
        strcat(buf, "true");
    else
        strcat(buf, "false");
    strcat(buf, ", UTC=");
    if (dt->UTC)
        strcat(buf, "true");
    else
        strcat(buf, "false");
    strcat(buf, ", date=\"");
    if (dt->date) {
        strcat(buf, dt->date);
    }
    strcat(buf, "\", time=\"");
    if (dt->time) {
        strcat(buf, dt->time);
    }
    strcat(buf, "\", text=\"");
    if (dt->text) {
        strcat(buf, dt->text);
    }
    strcat(buf, "\")");
     //alloc mem for the resulting string and return it
    char* out = calloc(strlen(buf) + 1, sizeof(char));    
    if (out) {
        strcpy(out, buf);
    }
    else {
        //err handling
        return NULL;
    }
    return out;
}



//trim() remove whitespace before and after str
void trimString(char* str) 
{
    if (str == NULL) return;
    int startIndex = 0;
    while (str[startIndex] != '\0' && isspace(str[startIndex])) {
        startIndex++;
    }
    if (startIndex > 0) {
        int shiftIndex = 0;
        while (str[startIndex + shiftIndex] != '\0') {
            str[shiftIndex] = str[startIndex + shiftIndex];
            shiftIndex++;
        }
        str[shiftIndex] = '\0';
    }
    int strLength = (int)strlen(str);
    while (strLength > 0 && isspace(str[strLength - 1])) {
        str[strLength - 1] = '\0';
        strLength--;
    }
}

//parse date/time string into a DateTime structure.
DateTime* parseDateTime(const char* rawStr) 
{
    //input validation: if input str null ret null
    if (rawStr == NULL) {
        return NULL;
    }
    //alloc and init datetime obj.
    DateTime* dateTimeObject = calloc(1, sizeof(DateTime));
    if (!dateTimeObject) {
        return NULL;
    }
    dateTimeObject->UTC = false;
    dateTimeObject->isText = false;
    dateTimeObject->date = NULL;
    dateTimeObject->time = NULL;
    dateTimeObject->text = NULL;

    //case1 special date format with -- ---: treat the entire string as the date.
    if (strlen(rawStr) >= 2 && rawStr[0] == '-' && rawStr[1] == '-') 
    {
        dateTimeObject->date = calloc(strlen(rawStr) + 1, sizeof(char));
        //if problem allocating ret NULL
        if (!dateTimeObject->date) {
            free(dateTimeObject);
            return NULL;
        }
        strcpy(dateTimeObject->date, rawStr);
        //alloc empty strings for time and text.
        dateTimeObject->time = calloc(1, sizeof(char));
        dateTimeObject->text = calloc(1, sizeof(char));
        return dateTimeObject;
    }

    //case2: Date/Time String with 'T' that separates date and time parts.
    const char* tPointer = strchr(rawStr, 'T');
    if (tPointer != NULL) 
    {
        //calc length of date portion before T
        int datePortionLength = (int)(tPointer - rawStr);
        dateTimeObject->date = calloc(datePortionLength + 1, sizeof(char));
        //problem alloc
        if (!dateTimeObject->date) {
            free(dateTimeObject);
            return NULL;
        }
        //copy date portion from rawSTR
        for (int i = 0; i < datePortionLength; i++) {
            dateTimeObject->date[i] = rawStr[i];
        }
        dateTimeObject->date[datePortionLength] = '\0';
        //alloc and copy the time portion after 'T'.
        const char* afterT = tPointer + 1;
        dateTimeObject->time = calloc(strlen(afterT) + 1, sizeof(char));
        //problem alloc
        if (!dateTimeObject->time) {
            free(dateTimeObject->date); //free prev allocs
            free(dateTimeObject);
            return NULL;
        }
        strcpy(dateTimeObject->time, afterT);
        //if  time ends with Z set UTC flag and remove Z
        int timePortionLength = (int)strlen(dateTimeObject->time);
        if (timePortionLength > 0 && dateTimeObject->time[timePortionLength - 1] == 'Z') {
            dateTimeObject->UTC = true;
            dateTimeObject->time[timePortionLength - 1] = '\0';
        }
        dateTimeObject->text = calloc(1, sizeof(char));   //alloc  empty text field.
        if (!dateTimeObject->text) {
            free(dateTimeObject->date);  //free prev alloc
            free(dateTimeObject->time);     //free prev alloc
            free(dateTimeObject);
            return NULL;
        }
        return dateTimeObject;
    }

    //case3: 8-Digit Date Format:if exactly 8 char long, check if all are digits.
    if (strlen(rawStr) == 8) 
    {
        bool allDigits = true;
        for (int i = 0; i < 8; i++) {
            if (!isdigit(rawStr[i])) {
                allDigits = false;
                break;
            }
        }
        //if all char are digits --> treat the string as a date.
        if (allDigits) 
        {
            dateTimeObject->date = calloc(9, sizeof(char));
            //problem with alloc
            if (!dateTimeObject->date) {
                free(dateTimeObject);
                return NULL;
            }
            strcpy(dateTimeObject->date, rawStr);
            //alloc empty str for time
            dateTimeObject->time = calloc(1, sizeof(char));
            //problem with alloc
            if (!dateTimeObject->time) {
                free(dateTimeObject->date);
                free(dateTimeObject);
                return NULL;
            }
            //alloc empty str for txt
            dateTimeObject->text = calloc(1, sizeof(char));
            //problem with alloc
            if (!dateTimeObject->text) {
                free(dateTimeObject->date);
                free(dateTimeObject->time);
                free(dateTimeObject);
                return NULL;
            }
            return dateTimeObject;
        }
    }

    //case: mark the object as text.
    dateTimeObject->isText = true;
    dateTimeObject->text = calloc(strlen(rawStr) + 1, sizeof(char));
    if (!dateTimeObject->text) {
        free(dateTimeObject);
        return NULL;
    }
    strcpy(dateTimeObject->text, rawStr);
    //alloc empty strings for date and time.
    dateTimeObject->date = calloc(1, sizeof(char));
    dateTimeObject->time = calloc(1, sizeof(char));
    return dateTimeObject;
}

//split unfolded line into group, property name, parameters and value. Then adds to card object
//prof recommended to use strpbrk and strstr
void parseLine(Card* cardObj, char* lineStr, bool* foundBegin, bool* foundEnd, bool* foundVersion, bool* foundFn, VCardErrorCode* lastError, Card** newCardObj) 
{
    //if previous error occurred, do nothing.
    if (*lastError != OK) {
        return;
    }
    trimString(lineStr);    //remove extra spaces from line
    //if the line is empty after trimming, clean and set error
    if (lineStr[0] == '\0') 
    {
        deleteCard(cardObj);
        *lastError = INV_PROP;
        *newCardObj = NULL;
        return;
    }
    //create tolower copy of the line for case-insensitive comparisons
    char lowerCopy[1024];
    int lowerCharIndex = 0;
    while (lineStr[lowerCharIndex] != '\0' && lowerCharIndex < 1023) 
    {
        char currentChar = lineStr[lowerCharIndex];
        if (currentChar >= 'A' && currentChar <= 'Z') {
            currentChar = (char)(currentChar + ('a' - 'A'));
        }
        lowerCopy[lowerCharIndex] = currentChar;
        lowerCharIndex++;
    }
    lowerCopy[lowerCharIndex] = '\0';
    //checks if the line marks begin:vcard
    {
        char* beginMarker = strstr(lowerCopy, "begin:vcard");
        if (beginMarker != NULL && beginMarker == lowerCopy) {
            *foundBegin = true;
            return;
        }
    }
    //checks if the line marks end:vcard
    char* endMarker = strstr(lowerCopy, "end:vcard");
    if (endMarker != NULL && endMarker == lowerCopy) {
        *foundEnd = true;
        return;
    }
     
    //checks if the line contains VERSION property and verify it is 4.0 (VALID CARD)
    
    char* versionMarker = strstr(lowerCopy, "version:");
    if (versionMarker != NULL && versionMarker == lowerCopy) 
    {
        int versionOffset = 8;
        while (lowerCopy[versionOffset] == ' ' || lowerCopy[versionOffset] == '\t') {
            versionOffset++;
        }
        if (strcmp(&lowerCopy[versionOffset], "4.0") == 0) 
        {
            *foundVersion = true;
            return;
        } 
        //delete if not 4.0 version
        else 
        {
            deleteCard(cardObj);
            *lastError = INV_CARD; //invalid
            *newCardObj = NULL;
            return;
        }
    }


    //split line at the first colon to sep header from value.
    char* colonPosition = strpbrk(lineStr, ":");
    if (!colonPosition) 
    {
        deleteCard(cardObj);
        *lastError = INV_PROP;
        *newCardObj = NULL;
        return;
    }
    *colonPosition = '\0';
    colonPosition++;
    trimString(colonPosition);
    //if no value after the colon error out.
    if (strlen(colonPosition) == 0) 
    {
        deleteCard(cardObj);
        *lastError = INV_PROP;
        *newCardObj = NULL;
        return;
    }
    //copy and trim header part (before the colon)
    char headerBuffer[1024];
    strcpy(headerBuffer, lineStr);
    trimString(headerBuffer);

    //extract group (before dot) from header if present
    char groupNameBuffer[256] = "";
    char* dotPosition = strpbrk(headerBuffer, ".");
    if (dotPosition != NULL) 
    {
        int groupLength = (int)(dotPosition - headerBuffer);
        if (groupLength >= 255) 
        {
            deleteCard(cardObj);
            *lastError = INV_PROP;
            *newCardObj = NULL;
            return;
        }
        for (int i = 0; i < groupLength; i++) {
            groupNameBuffer[i] = headerBuffer[i];
        }
        groupNameBuffer[groupLength] = '\0';
        trimString(groupNameBuffer);
        //remove group part (up to and including the dot) from the header
        int afterDotIndex = groupLength + 1;
        int writeIndex = 0;
        while (headerBuffer[afterDotIndex] != '\0') {
            headerBuffer[writeIndex++] = headerBuffer[afterDotIndex++];
        }
        headerBuffer[writeIndex] = '\0';
        trimString(headerBuffer);
    }

    //separate property name from its parameters (if) using semicolon as delim
    char propertyNameBuffer[256] = "";
    char* parameterStringBuffer = NULL;
    
    char* semicolonPosition = strpbrk(headerBuffer, ";");
    if (semicolonPosition) 
    {
        int nameLength = (int)(semicolonPosition - headerBuffer);
        if (nameLength <= 0) {
            deleteCard(cardObj);
            *lastError = INV_PROP;
            *newCardObj = NULL;
            return;
        }
        if (nameLength > 255) nameLength = 255;
        for (int i = 0; i < nameLength; i++) {
            propertyNameBuffer[i] = headerBuffer[i];
        }
        propertyNameBuffer[nameLength] = '\0';
        trimString(propertyNameBuffer);
        parameterStringBuffer = semicolonPosition + 1;
        trimString(parameterStringBuffer);
    } 
    else {
        strcpy(propertyNameBuffer, headerBuffer);
        trimString(propertyNameBuffer);
    }
    
    //if property name is empty --> error
    if (strlen(propertyNameBuffer) == 0) 
    {
        deleteCard(cardObj);
        *lastError = INV_PROP;
        *newCardObj = NULL;
        return;
    }

    //see if the property type: FN, N, BDAY, ANNIVERSARY, TEL, ADR, by changing property name to lowercase.
    bool isFn = false, isN = false, isBday = false, isAnn = false, isTel = false, isAdr = false;
    char propertyNameLower[256];
    int i = 0;
    while (propertyNameBuffer[i] != '\0' && i < 255) {
        char ch = propertyNameBuffer[i];
        if (ch >= 'A' && ch <= 'Z') {
            ch = (char)(ch + ('a' - 'A'));
        }
        propertyNameLower[i] = ch;
        i++;
    }
    propertyNameLower[i] = '\0';
    if (!(*foundFn) && strcmp(propertyNameLower, "fn") == 0) {
        isFn = true;
    } 
    else if (strcmp(propertyNameLower, "n") == 0) {
        isN = true;
    } 
    else if (strcmp(propertyNameLower, "bday") == 0) {
        isBday = true;
    } 
    else if (strcmp(propertyNameLower, "anniversary") == 0) {
        isAnn = true;
    } 
    else if (strcmp(propertyNameLower, "tel") == 0) {
        isTel = true;
    } 
    else if (strcmp(propertyNameLower, "adr") == 0) {
        isAdr = true;
    }
    

    //process FN property: alloc memory and set name, group, parameters, and value.
    //name
    if (isFn) 
    {
        cardObj->fn->name = calloc(3, sizeof(char));
        //fail alloc
        if (!cardObj->fn->name) { 
            deleteCard(cardObj); 
            *lastError = OTHER_ERROR; 
            *newCardObj = NULL; 
            return; 
        }
        strcpy(cardObj->fn->name, "FN"); //string copy

        //group
        cardObj->fn->group = calloc(strlen(groupNameBuffer) + 1, sizeof(char));
        //fail alloc
        if (!cardObj->fn->group) 
        { 
            deleteCard(cardObj); 
            *lastError = OTHER_ERROR; 
            *newCardObj = NULL; 
            return; 
        }
        strcpy(cardObj->fn->group, groupNameBuffer); //stringcopy

        cardObj->fn->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
        cardObj->fn->values = initializeList(valueToString, deleteValue, compareValues);
        //fail alloc for either
        if (!cardObj->fn->parameters || !cardObj->fn->values) 
        {
            deleteCard(cardObj);
            *lastError = OTHER_ERROR;
            *newCardObj = NULL;
            return;
        }

        //process parameters for the FN property (IF PRESENT)
        if (parameterStringBuffer && strlen(parameterStringBuffer) > 0) 
        {
            char parameterCopyBuffer[1024];
            strcpy(parameterCopyBuffer, parameterStringBuffer);
            trimString(parameterCopyBuffer);
            char* token = parameterCopyBuffer;
            while (*token != '\0') 
            {
                char* nextSemicolon = strpbrk(token, ";");
                if (nextSemicolon) {
                    *nextSemicolon = '\0';
                }
                trimString(token);
                if (strlen(token) == 0) {
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                char* equalPosition = strpbrk(token, "=");
                //for not equal positions
                if (!equalPosition) {
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                *equalPosition = '\0';
                char* parameterName = token;
                char* parameterValue = equalPosition + 1;
                trimString(parameterName);
                trimString(parameterValue);
                if (strlen(parameterName) == 0 || strlen(parameterValue) == 0) {
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                Parameter* newParameter = calloc(1, sizeof(Parameter));
                //fail alloc
                if (!newParameter) 
                { 
                    deleteCard(cardObj); 
                    *lastError = OTHER_ERROR; 
                    *newCardObj = NULL; 
                    return; 
                }
                newParameter->name = calloc(strlen(parameterName) + 1, sizeof(char));
                newParameter->value = calloc(strlen(parameterValue) + 1, sizeof(char));
                //fail alloc
                if (!newParameter->name || !newParameter->value) 
                {
                    free(newParameter->name);
                    free(newParameter->value);
                    free(newParameter);
                    deleteCard(cardObj);
                    *lastError = OTHER_ERROR;
                    *newCardObj = NULL;
                    return;
                }
                strcpy(newParameter->name, parameterName);
                strcpy(newParameter->value, parameterValue);
                insertBack(cardObj->fn->parameters, newParameter); //add to list
                //if no more break.
                if (!nextSemicolon) {
                    break;
                }
                token = nextSemicolon + 1;
            }
        }

        //alloc and add the FN value
        char* fnValueBuffer = calloc(strlen(colonPosition) + 1, sizeof(char));
        //alloc fail
        if (!fnValueBuffer) 
        { 
            deleteCard(cardObj); 
            *lastError = OTHER_ERROR; 
            *newCardObj = NULL; 
            return; 
        }
        strcpy(fnValueBuffer, colonPosition);
        trimString(fnValueBuffer);
        insertBack(cardObj->fn->values, fnValueBuffer); //add to list
        *foundFn = true; //we found fn. SET flag to TRUE
        return;
    }

    //process N, TEL, ADR properties (Optional properties)
    if (isN || isTel || isAdr) 
    {
        Property* newProperty = calloc(1, sizeof(Property));
        if (!newProperty) { 
            deleteCard(cardObj); 
            *lastError = OTHER_ERROR; 
            *newCardObj = NULL; 
            return; 
        }
        //cpy property and group names locally.
        char propertyNameBufferLocal[256];
        strcpy(propertyNameBufferLocal, propertyNameBuffer);
        char groupNameBufferLocal[256];
        strcpy(groupNameBufferLocal, groupNameBuffer);
        newProperty->name = calloc(strlen(propertyNameBufferLocal) + 1, sizeof(char));
        newProperty->group = calloc(strlen(groupNameBufferLocal) + 1, sizeof(char));
        //if either alloc fail
        if (!newProperty->name || !newProperty->group) 
        {
            deleteProperty(newProperty);
            deleteCard(cardObj);
            *lastError = OTHER_ERROR;
            *newCardObj = NULL;
            return;
        }
        strcpy(newProperty->name, propertyNameBufferLocal);
        strcpy(newProperty->group, groupNameBufferLocal);

        newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
        newProperty->values = initializeList(valueToString, deleteValue, compareValues);
        //fail alloc
        if (!newProperty->parameters || !newProperty->values) 
        {
            deleteProperty(newProperty);
            deleteCard(cardObj);
            *lastError = OTHER_ERROR;
            *newCardObj = NULL;
            return;
        }

        //process param for the optional property (if any)
        if (parameterStringBuffer && strlen(parameterStringBuffer) > 0) 
        {
            char parameterCopyBuffer[1024];
            strcpy(parameterCopyBuffer, parameterStringBuffer);
            trimString(parameterCopyBuffer); //clear whitespace
            char* token = parameterCopyBuffer;
            while (*token != '\0') 
            {
                char* nextSemicolon = strpbrk(token, ";");
                if (nextSemicolon) {
                    *nextSemicolon = '\0';
                }
                trimString(token); 
                if (strlen(token) == 0) {
                    deleteProperty(newProperty);
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                char* equalPos = strpbrk(token, "=");
                if (!equalPos) {
                    deleteProperty(newProperty);
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                *equalPos = '\0';
                char* parameterName = token;
                char* parameterValue = equalPos + 1;
                trimString(parameterName);
                trimString(parameterValue);
                //if empty delete
                if (strlen(parameterName) == 0 || strlen(parameterValue) == 0) 
                {
                    deleteProperty(newProperty);
                    deleteCard(cardObj);
                    *lastError = INV_PROP;
                    *newCardObj = NULL;
                    return;
                }
                Parameter* newParameter = calloc(1, sizeof(Parameter)); //alloc for new parameter
                //fail alloc
                if (!newParameter) 
                {
                    deleteProperty(newProperty);
                    deleteCard(cardObj);
                    *lastError = OTHER_ERROR;
                    *newCardObj = NULL;
                    return;
                }
                newParameter->name = calloc(strlen(parameterName) + 1, sizeof(char)); //give memory to param name and value 
                newParameter->value = calloc(strlen(parameterValue) + 1, sizeof(char));
                //fail alloc
                if (!newParameter->name || !newParameter->value) 
                {
                    free(newParameter->name);
                    free(newParameter->value);
                    free(newParameter);
                    deleteProperty(newProperty);
                    deleteCard(cardObj);
                    *lastError = OTHER_ERROR;
                    *newCardObj = NULL;
                    return;
                }
                strcpy(newParameter->name, parameterName);
                strcpy(newParameter->value, parameterValue);
                insertBack(newProperty->parameters, newParameter); //add to list
                if (!nextSemicolon) {
                    break;
                }
                token = nextSemicolon + 1;
            }
        }

        //decided to maybe split the value based on the property type.
        bool splitValue = false;
        if (isN || isAdr) {
            splitValue = true;
        } 
        else if (isTel) {
            if (strchr(colonPosition, ';') != NULL) {
                splitValue = true;
            }
        }

        //split the value (required)
        if (splitValue) {
            char temporaryBuffer[1024];
            temporaryBuffer[0] = '\0';
            int index = 0, outputIndex = 0;
            int colonValueLength = (int)strlen(colonPosition);
            while (index <= colonValueLength) 
            {
                char currentChar = colonPosition[index];
                if (currentChar == ';' || currentChar == '\0') 
                {
                    temporaryBuffer[outputIndex] = '\0';
                    trimString(temporaryBuffer); //cut whitespace 
                    char* pieceValue = calloc(strlen(temporaryBuffer) + 1, sizeof(char));
                    //fail alloc
                    if (!pieceValue) {
                        deleteProperty(newProperty);
                        deleteCard(cardObj);
                        *lastError = OTHER_ERROR;
                        *newCardObj = NULL;
                        return;
                    }
                    strcpy(pieceValue, temporaryBuffer);
                    insertBack(newProperty->values, pieceValue); //add to list
                    temporaryBuffer[0] = '\0';
                    outputIndex = 0;
                    if (currentChar == '\0') {
                        break;
                    }
                } 
                else {
                    if (outputIndex < 1023) {
                        temporaryBuffer[outputIndex++] = currentChar;
                    }
                }
                index++;
            }
        } 
         //else just store the single value.
        else 
        {
            char* singleValue = calloc(strlen(colonPosition) + 1, sizeof(char));
            if (!singleValue) {
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = OTHER_ERROR;
                *newCardObj = NULL;
                return;
            }
            strcpy(singleValue, colonPosition);
            trimString(singleValue);
            insertBack(newProperty->values, singleValue);
        }
        insertBack(cardObj->optionalProperties, newProperty);        //insert new property to  list of optional properties.
        return;
    }

    //process BDAY or ANNIVERSARY properties by parsing the date/time.
    if (isBday || isAnn) 
    {
        DateTime* dateTimeObj = parseDateTime(colonPosition);
        if (!dateTimeObj) {
            deleteCard(cardObj);
            *lastError = OTHER_ERROR;
            *newCardObj = NULL;
            return;
        }
        if (isBday) {
            cardObj->birthday = dateTimeObj;
        } 
        else {
            cardObj->anniversary = dateTimeObj;
        }
        return;
    }

    Property* newProperty = calloc(1, sizeof(Property));    //alloc new property
    //alloc fail
    if (!newProperty)
    {
        deleteCard(cardObj);
        *lastError = OTHER_ERROR;
        *newCardObj = NULL;
        return;
    }
    //alloc property components
    newProperty->name = calloc(strlen(propertyNameBuffer) + 1, sizeof(char));
    newProperty->group = calloc(strlen(groupNameBuffer) + 1, sizeof(char));
    //failed alloc
    if (!newProperty->name || !newProperty->group) 
    {
        deleteProperty(newProperty);
        deleteCard(cardObj);
        *lastError = OTHER_ERROR;
        *newCardObj = NULL;
        return;
    }
    strcpy(newProperty->name, propertyNameBuffer);
    strcpy(newProperty->group, groupNameBuffer);
    newProperty->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    newProperty->values = initializeList(valueToString, deleteValue, compareValues);
    //failed inits
    if (!newProperty->parameters || !newProperty->values) 
    {
        deleteProperty(newProperty);
        deleteCard(cardObj);
        *lastError = OTHER_ERROR;
        *newCardObj = NULL;
        return;
    }
   //if param string and it is not empty --> process the parameters
    if (parameterStringBuffer && strlen(parameterStringBuffer) > 0) 
    {
        //copy of param string to work on and trim extra spaces
        char parameterCopyBuffer[1024];
        strcpy(parameterCopyBuffer, parameterStringBuffer);
        trimString(parameterCopyBuffer); //cut whitespace
        char* token = parameterCopyBuffer;        //set token pointer to iterate over the parameter copy

        //loop through each param separated by semicolons
        while (*token != '\0') 
        {
            //find next semicolon in token
            char* nextSemicolon = strpbrk(token, ";");
            if (nextSemicolon) {
                *nextSemicolon = '\0'; //term the current token at semicolon
            }
            trimString(token);            //cut extra spaces from the current token
            //if token is empty, clean and set error
            if (strlen(token) == 0) 
            {
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = INV_PROP;
                *newCardObj = NULL;
                return;
            }
            
            //find the '=' character to sep parameter name and value
            char* equalPos = strpbrk(token, "=");
            if (!equalPos) 
            {
                //if '=' is not found, clean and set error
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = INV_PROP;
                *newCardObj = NULL;
                return;
            }
            *equalPos = '\0';  //term param name string
            
            //set pointers for param name and value
            char* parameterName = token;
            char* parameterValue = equalPos + 1;
            //trim extra spaces from parameter name and value
            trimString(parameterName);
            trimString(parameterValue);
            //if either param name or value is empty, clean and set  error
            if (strlen(parameterName) == 0 || strlen(parameterValue) == 0) 
            {
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = INV_PROP;
                *newCardObj = NULL;
                return;
            }
            
            //alloc for a new Parameter structure
            Parameter* newParameter = calloc(1, sizeof(Parameter));
            if (!newParameter) 
            {
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = OTHER_ERROR;
                *newCardObj = NULL;
                return;
            }
            //alloc for the parameter name and value strings
            newParameter->name = calloc(strlen(parameterName) + 1, sizeof(char));
            newParameter->value = calloc(strlen(parameterValue) + 1, sizeof(char));
            if (!newParameter->name || !newParameter->value) 
            {
                free(newParameter->name);
                free(newParameter->value);
                free(newParameter);
                deleteProperty(newProperty);
                deleteCard(cardObj);
                *lastError = OTHER_ERROR;
                *newCardObj = NULL;
                return;
            }
            //copy param name and value into the new Parameter structure
            strcpy(newParameter->name, parameterName);
            strcpy(newParameter->value, parameterValue);
            
            //add new parameter to the property parameter list
            insertBack(newProperty->parameters, newParameter);
            
            //if no more semicolons --> exit the loop ELSE move to the next token
            if (!nextSemicolon) {
                break;
            }
            token = nextSemicolon + 1;
        }
    }

    //alloc memory for a copy of the generic value from the colon position
    char* genericValueCopy = calloc(strlen(colonPosition) + 1, sizeof(char));
    if (!genericValueCopy) 
    {
        //if alloc fail clean set error
        deleteProperty(newProperty);
        deleteCard(cardObj);
        *lastError = OTHER_ERROR;
        *newCardObj = NULL;
        return;
    }

    //copy, trim the generic value, THEN add it to the property values list
    strcpy(genericValueCopy, colonPosition);
    trimString(genericValueCopy);
    insertBack(newProperty->values, genericValueCopy);
    insertBack(cardObj->optionalProperties, newProperty); //add the new property to the card object list of optional properties

}


