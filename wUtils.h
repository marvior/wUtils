/*
 * Copyright 2026 Molino Rosario Walter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --- IMPORTANT NOTICE ---
 * THIS SOFTWARE IS PROVIDED SOLELY FOR EDUCATIONAL AND ACADEMIC PURPOSES.
 * IT IS NOT INTENDED AND MUST NOT BE USED IN PRODUCTION ENVIRONMENTS.
 */

#include <stdint.h>

typedef enum {
    DICT_OK = 0,
    DICT_ERR_MEMORY = -1,
    DICT_ERR_INVALID = -2
} wDictStatus;

typedef struct wDict wDict;
typedef struct wList wList;

typedef enum {
    TEXT,
    DICTIONARY,
    NUMBER,
    DOUBLE,
    LIST,
    NONE
}typeObject;

typedef struct ListKeys{
    char * key;
    struct ListKeys * next;
}ListKeys;


typedef struct {
    typeObject type;
    union {
        char * text;
        wDict * dict;
        int number;
        double real;
        wList * list;
    } data;
}Value;

typedef struct Node{
    char * key;
    Value value;
    struct Node * next;
} Node;


struct wDict{
    Node ** slots;
    int capacity;
    int ref_counting;
    int element_inserted;

};


typedef struct wDict_destroy{
    wDict *dict;
    struct wDict_destroy * next;

} wDict_destroy;


typedef struct wList_destroy{
    wList *list;
    struct wList_destroy * next;

} wList_destroy;

struct wList{
    Value ** elements;
    int capacity;
    int element_inserted;
    int ref_counting;
};


Value _type_value_int(int value);
Value _type_value_double(double value);
Value _type_value_text(char* value);
Value _type_value_dict(wDict* value);
Value _type_value_list(wList* value);
void _destroy_list(wList **list_ptr);
void _destroy_dict(wDict **dict_ptr);



wDict * create_dictionary(int capacity);
void insert_value(wDict * dict, char * key, Value value);
void * get_value(wDict * dict,char * key);

wList * create_list(int capacity);
void append_element(wList * list, Value value);
void * get_element(wList * dict,int index);
void resize_dict(wDict * dict);
void resize_list(wList * list);


#define destroy(object) _Generic((object), \
wList **: _destroy_list, \
wDict **: _destroy_dict\
)(object)



#define new_value(value) _Generic((value), \
int:        _type_value_int,        \
double:     _type_value_double,     \
char *:     _type_value_text,       \
wDict *:    _type_value_dict,       \
wList *:    _type_value_list        \
)(value)