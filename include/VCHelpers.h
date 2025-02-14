/*
    VCHelpers.h
    Name: Muhammad Ali
    Student ID: 1115336
*/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "VCParser.h"
#include "LinkedListAPI.h"

//ORIGINAL Helpers:
//property
void deleteProperty(void* toBeDeleted);
int compareProperties(const void* first,const void* second);
char* propertyToString(void* prop);

//params
void deleteParameter(void* toBeDeleted);
int compareParameters(const void* first,const void* second);
char* parameterToString(void* param);

//values
void deleteValue(void* toBeDeleted);
int compareValues(const void* first,const void* second);
char* valueToString(void* val);

//date
void deleteDate(void* toBeDeleted);
int compareDates(const void* first,const void* second);
char* dateToString(void* date);

//new helpers
void trimString(char* str);
DateTime* parseDateTime(const char* rawStr);
void parseLine(Card* cardObj,char* lineStr,bool* foundBegin,bool* foundEnd,bool* foundVersion,bool* foundFn,VCardErrorCode* lastError,Card** newCardObj);