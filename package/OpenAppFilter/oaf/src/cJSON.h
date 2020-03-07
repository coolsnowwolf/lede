/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h
#include <linux/slab.h>

// cJSON Types:
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6

// The cJSON structure:
typedef struct cJSON {
	struct cJSON *next,*prev;	// next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem
	struct cJSON *child;		// An array or object item will have a child pointer pointing to a chain of the items in the array/object.

	int type;					// The type of the item, as above.

	char *valuestring;			// The item's string, if type==cJSON_String
	int valueint;				// The item's number, if type==cJSON_Number
	char *string;				// The item's name string, if this item is the child of, or is in the list of subitems of an object.
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void *(*realloc_fn)(void *ptr, size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

// Supply malloc, realloc and free functions to cJSON
extern void cJSON_InitHooks(cJSON_Hooks* hooks);


// Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished.
extern cJSON *cJSON_Parse(const char *value);
// Render a cJSON entity to text for transfer/storage. Free the char* when finished.
extern char  *cJSON_Print(cJSON *item);
// Delete a cJSON entity and all subentities.
extern void   cJSON_Delete(cJSON *c);

// Returns the number of items in an array (or object).
extern int	  cJSON_GetArraySize(cJSON *array);
// Retrieve item number "item" from array "array". Returns NULL if unsuccessful.
extern cJSON *cJSON_GetArrayItem(cJSON *array,int item);
// Get item "string" from object. Case insensitive.
extern cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);

// These calls create a cJSON item of the appropriate type.
extern cJSON *cJSON_CreateNull(void);
extern cJSON *cJSON_CreateTrue(void);
extern cJSON *cJSON_CreateFalse(void);
extern cJSON *cJSON_CreateNumber(int num);
extern cJSON *cJSON_CreateString(const char *string);
extern cJSON *cJSON_CreateArray(void);
extern cJSON *cJSON_CreateObject(void);
extern void cJSON_Minify(char *json);
// These utilities create an Array of count items.
extern cJSON *cJSON_CreateIntArray(int *numbers,int count);

// Append item to the specified array/object.
extern void   cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void	  cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);

#define cJSON_AddNullToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#endif
