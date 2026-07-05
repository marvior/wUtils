#include <stdio.h>
#include "wUtils.h"

int main(){
    wDict * dict = create_dictionary(3);
    wDict * dict2 = create_dictionary(3);
    wDict * dict3 = create_dictionary(3);

    
    insert_value(dict3,"pippo",new_value(3));
    insert_value(dict2,"pippo",new_value(3));
    insert_value(dict2,"pippo2",new_value(dict3));
    dict3=NULL;
    
    insert_value(dict,"hello",new_value(5));
    insert_value(dict,"ciao",new_value("walter"));
    insert_value(dict,"nada",new_value("cipro"));
    insert_value(dict,"beth",new_value(dict2));
    dict2=NULL;

    char * c = (char*) get_value(dict,"ciao");
    printf("ciao %s\n",c);

    wDict * d = (wDict *) get_value(dict,"beth");
    int n = *(int *)get_value(d,"pippo");
    printf("ciao 2 %i\n",n);

    

    wDict * dict5 = create_dictionary(3);
    insert_value(dict5,"pippo",new_value(3));

    wList * list = create_list(3);
    append_element(list,new_value(10));
    append_element(list,new_value(dict5));
    dict5=NULL; //ownership dict

    printf("list element 0 %i\n",*(int*)get_element(list,0));
    printf("list element 1 %i\n",*(int*)get_value((wDict*)get_element(list,1),"pippo"));
    
    insert_value(dict,"beth2",new_value(list));
    list=NULL; //ownership list

    destroy(&dict);
    printf("pointer list %p\n",dict);
    destroy(&dict);
    destroy(&list);
    destroy(&list);
    
    return 0;
}