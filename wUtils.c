#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "wUtils.h"
#include "hash_fnv1a_64.h"


Value _type_value_int(int value){
    return (Value){
        .type = NUMBER,
        .data.number = value
    };

}
Value _type_value_double(double value){
    return (Value){
        .type = DOUBLE,
        .data.real = value
    };

}


Value _type_value_text(char* value){
    
    return (Value){
        .type = TEXT,
        .data.text = strdup(value)
    };

}


Value _type_value_dict(wDict* value){
    return (Value){
        .type = DICTIONARY,
        .data.dict = value
    };

}

Value _type_value_list(wList* value){
    return (Value){
        .type = LIST,
        .data.list = value
    };

}


void * get_value(wDict * dict,char * key){
    uint64_t index =  hashing_fnv1a(key,dict->capacity);
    if (dict->slots[index] == NULL) return NULL;
    printf("sono qui\n");
    Node * next_node = dict->slots[index];
    while(next_node != NULL){
        if (strcmp(next_node->key,key)==0){
            switch(next_node->value.type){
                case NUMBER: return &next_node->value.data.number;
                case DOUBLE: return &next_node->value.data.real;
                case TEXT: return next_node->value.data.text;
                case DICTIONARY: return next_node->value.data.dict;
                default : break;
            }
        }
        next_node = next_node->next;
    }
    printf("sono qui");
    return NULL;
}



void * get_element(wList * list,int index){
    if (index >= list->element_inserted ) return NULL;
    printf("sono qui list\n");
    
    switch(list->elements[index]->type){
        case NUMBER: {printf("number \n");return &list->elements[index]->data.number;}
        case DOUBLE: return &list->elements[index]->data.real;
        case TEXT: return list->elements[index]->data.text;
        case DICTIONARY: return list->elements[index]->data.dict;
        default : break;
    }
       
    printf("sono qui");
    return NULL;
}





void insert_value(wDict * dict, char * key, Value value){
    
    uint64_t index =  hashing_fnv1a(key,dict->capacity);

    Node * node = malloc(sizeof(Node));
    if(node == NULL) return;
    
    node->key = strdup(key);
    node->value = value;
    node->next = NULL;

    if(value.type == DICTIONARY) value.data.dict->ref_counting +=1;
    if(value.type == LIST) value.data.list->ref_counting +=1;
    
    if (dict->slots[index]==NULL) dict->slots[index]=node;
    else{
        Node * next_node =dict->slots[index];
        Node * tmp_node = next_node;
        while(next_node!=NULL){
            //if the key is equal then update the value;
            if(strcmp(next_node->key,key)==0){
                if(next_node->value.type == DICTIONARY) {
                    next_node->value.data.dict->ref_counting -=1;
                    if(next_node->value.data.dict->ref_counting ==0) free(next_node->value.data.dict);
                }
                if(next_node->value.type == LIST) {
                    next_node->value.data.list->ref_counting -=1;
                    if(next_node->value.data.list->ref_counting ==0) free(next_node->value.data.list);
                }
                if(next_node->value.type == TEXT) {
                    
                    free(next_node->value.data.text);
                }
                next_node->value=value;
                //node is not more useful
                free(node);
                return;
            }
            tmp_node=next_node;
            next_node=next_node->next;
        }
        
        tmp_node->next = node;
    }
    
}


void append_element(wList * list, Value value){
    
    int index = list->element_inserted;
    if (index<list->capacity){
        if(value.type == DICTIONARY) value.data.dict->ref_counting +=1;
        if(value.type == LIST) value.data.list->ref_counting +=1;
        
        Value * stored = malloc(sizeof(Value));
        if (stored == NULL) return; 

        *stored = value;


        list->elements[index] = stored;
        printf("insert eleemnt %i\n",index);

        list->element_inserted += 1;
    }

    
}


static void free_dict_list(wDict_destroy * dict_list,wList_destroy * lists_toDestroy){
 /*
 free elements into list type of wDict_destroy and in wList_destroy without destroy elements
 */
    wDict_destroy * tmp_dict;
    wList_destroy * tmp_list;
    
    while(dict_list!=NULL){
        tmp_dict=dict_list->next;
        free(dict_list);
        dict_list=tmp_dict;
    }
    
    while(lists_toDestroy!=NULL){
        tmp_list=lists_toDestroy->next;
        free(lists_toDestroy);
        lists_toDestroy=tmp_list;
    }

}

static void destroy_object(wDict_destroy * dict_list,wList_destroy * lists_toDestroy){
    
    Node * slot;
    Node * switch_slot;
    
    //list of the dict nested to destroy
    
    wDict_destroy * dict_list_next= NULL;

    
    wList_destroy * lists_toDestroy_next= NULL;

    lists_toDestroy_next = NULL;

    
    
    
    while(dict_list!=NULL || lists_toDestroy!=NULL){
        
        if(dict_list!=NULL){
            
            if(dict_list->dict->ref_counting ==0){ 

                //Destroy all slots in the dictionary
                for(int i=0; i<dict_list->dict->capacity; i++){
                    switch_slot = slot = dict_list->dict->slots[i];
                    
                    //Destroy collision list of the slot
                    while (slot != NULL){
                        
                        switch(slot->value.type){
                            case NUMBER: printf("destroy int\n"); break;
                            case DOUBLE: printf("destroy int\n"); break;
                            case TEXT:{
                                free(slot->value.data.text);
                                break;
                            }
                            case DICTIONARY: {
                                //save the dictionary nested in the list
                                slot->value.data.dict->ref_counting -=1;
                                if(slot->value.data.dict->ref_counting ==0){
                                    dict_list_next = dict_list;
                                    while(dict_list->next!=NULL)
                                        dict_list = dict_list->next;

                                    dict_list->next = malloc(sizeof(wDict_destroy));
                                    if (dict_list->next == NULL) {
                                        free_dict_list(dict_list,lists_toDestroy);
                                        return;
                                    }
                                    dict_list->next->next = NULL;
                                    dict_list->next->dict = slot->value.data.dict;
                                    
                                    
                                    dict_list = dict_list_next;
                                }
                                break;
                            }
                            case LIST:{
                                slot->value.data.list->ref_counting -=1;
                                if(slot->value.data.list->ref_counting==0){
                                    if(lists_toDestroy == NULL){
                                        lists_toDestroy = malloc(sizeof(wList_destroy));
                                        if(lists_toDestroy == NULL) {
                                            free_dict_list(dict_list,lists_toDestroy);
                                            return;
                                        }

                                        lists_toDestroy->next = NULL;
                                        lists_toDestroy->list = slot->value.data.list;
                                    }else{
                                        lists_toDestroy_next = lists_toDestroy;
                                        while(lists_toDestroy->next != NULL)
                                            lists_toDestroy = lists_toDestroy->next;
                                        lists_toDestroy->next = malloc(sizeof(wList_destroy));
                                        if(lists_toDestroy->next == NULL) {
                                            free_dict_list(dict_list,lists_toDestroy);
                                            return;
                                        }

                                        lists_toDestroy->next->list = slot->value.data.list;
                                        lists_toDestroy->next->next = NULL;
                                        
                                        lists_toDestroy = lists_toDestroy_next;
                                    }
                                }
                                break;
                        
                            }
                            default : break;
                        }
                        switch_slot = slot->next;
                        free(slot);
                        printf("free slot\n");
                        slot = switch_slot;
                        
                    }
                }
                printf("Destroy Dict \n");
                dict_list_next = dict_list->next;

                //destroy the dict
                free(dict_list->dict);

                //destroy di element in list
                free(dict_list);

                dict_list = dict_list_next;
            }
        }else{
            printf("begin destroy list\n");
            if(lists_toDestroy != NULL){
                
                if(lists_toDestroy->list->ref_counting ==0) {
                    for(int i=0; i< lists_toDestroy->list->capacity; i++){
                        if(lists_toDestroy->list->elements[i]!=NULL){
                            printf("remove element %i \n",i);
                            switch(lists_toDestroy->list->elements[i]->type){
                                case NUMBER: printf("destroy int\n"); break;
                                case DOUBLE: printf("destroy int\n"); break;
                                case TEXT:{
                                    free(lists_toDestroy->list->elements[i]->data.text);
                                    break;
                                }
                                case DICTIONARY: {
                                    //save the dictionary nested in the list
                                    printf("remove dict nested\n");
                                    lists_toDestroy->list->elements[i]->data.dict->ref_counting -=1;
                                    if(lists_toDestroy->list->elements[i]->data.dict->ref_counting ==0){
                                        if(dict_list == NULL){
                                            dict_list = malloc(sizeof(wDict_destroy));
                                            if(dict_list == NULL) {
                                                free_dict_list(dict_list,lists_toDestroy);
                                                return;
                                            }

                                            dict_list->next = NULL;
                                            dict_list->dict = lists_toDestroy->list->elements[i]->data.dict;
                                            
                                        }else{
                                            dict_list_next = dict_list;
                                            while(dict_list->next!=NULL)
                                                dict_list = dict_list->next;

                                            dict_list->next = malloc(sizeof(wDict_destroy));
                                            if(dict_list->next == NULL) {
                                                free_dict_list(dict_list,lists_toDestroy);
                                                return;
                                            }

                                            dict_list->next->next = NULL;
                                            dict_list->next->dict = lists_toDestroy->list->elements[i]->data.dict;
                                            
                                            dict_list = dict_list_next;
                                        }
                                    }
                                    break;
                                }
                                case LIST:{
                                    lists_toDestroy->list->elements[i]->data.list->ref_counting -=1;
                                    if(lists_toDestroy->list->elements[i]->data.list->ref_counting ==0){
                                        
                                        lists_toDestroy_next = lists_toDestroy;
                                        while(lists_toDestroy->next != NULL)
                                            lists_toDestroy = lists_toDestroy->next;
                                        lists_toDestroy->next = malloc(sizeof(wList_destroy));
                                        if(lists_toDestroy->next == NULL) {
                                            free_dict_list(dict_list,lists_toDestroy);
                                            return;
                                        }

                                        lists_toDestroy->next->list = lists_toDestroy->list->elements[i]->data.list;
                                        lists_toDestroy->next->next = NULL;
                                        
                                        lists_toDestroy = lists_toDestroy_next;
                                    
                                    }
                                    break;
                            
                                }
                                default : break;

                            }
                        }
                        printf("ready remove element list\n");
                        free(lists_toDestroy->list->elements[i]);
                        printf("removed element list\n");
                    }

                    printf("Destroy Dict \n");
                    lists_toDestroy_next = lists_toDestroy->next;

                    //destroy the dict
                    free(lists_toDestroy->list);

                    //destroy di element in list
                    free(lists_toDestroy);

                    lists_toDestroy = lists_toDestroy_next;

                } 
            }
        }
    }

}




void _destroy_dict(wDict **dict_ptr){
    if(dict_ptr==NULL || *dict_ptr==NULL) {
        printf("dictionary already destroyed \n");
        return;
    }
    wDict * dict = *dict_ptr;
    if(dict->ref_counting>0) return;

    wList_destroy * lists_toDestroy= NULL;
    wDict_destroy * dict_list= NULL;
    dict_list = malloc(sizeof(wDict_destroy));
    if(dict_list == NULL) return;

    dict_list->next = NULL;
    dict_list->dict = dict;
    destroy_object(dict_list,lists_toDestroy);
    *dict_ptr=NULL;
}


void _destroy_list(wList **list_ptr){
    if(list_ptr ==NULL || *list_ptr==NULL) {
        printf("list already destroyed \n");
        return;
    }
    wList * list = *list_ptr;
    if(list->ref_counting>0) return;

    wList_destroy * lists_toDestroy= NULL;
    wDict_destroy * dict_list= NULL;
    
    lists_toDestroy = malloc(sizeof(wList_destroy));
    if(lists_toDestroy == NULL) return;

    lists_toDestroy->next = NULL;
    lists_toDestroy->list = list;
    destroy_object(dict_list,lists_toDestroy);
    *list_ptr=NULL;

}



wDict * create_dictionary(int capacity){
    if (capacity == 0) return NULL;

    wDict * dict = malloc(sizeof(*dict));
    if(dict == NULL) return NULL;

    dict->capacity = capacity;
    dict->ref_counting = 0;
    
    dict->slots = (Node **) malloc(sizeof(Node*) * capacity) ;
    if(dict->slots == NULL){
        free(dict);
        return NULL;
    }

    for(int i=0;i<capacity; i++) dict->slots[i] = NULL;
    
    return dict;

}



wList * create_list(int capacity){
    if (capacity == 0) return NULL;

    wList * list = malloc(sizeof(wList));
    if(list == NULL) return NULL;

    list->capacity = capacity;
    list->ref_counting = 0;
    list->element_inserted = 0;
    
    list->elements = (Value **) malloc(sizeof(Value*) * capacity) ;
    if(list->elements == NULL){
        free(list);
        return NULL;
    }

    for(int i=0;i<capacity; i++) list->elements[i] = NULL;
    
    return list;

}



