#include <stdio.h>
#include "wUtils.h"

void test_resize(){
    wDict * shared = create_dictionary(2);
wDict * container = create_dictionary(4);
insert_value(container, "s", new_value(shared));   // shared.ref_counting = 1

insert_value(shared, "a", new_value(1));
insert_value(shared, "b", new_value(2));   // shared.capacity(2) raggiunta -> resize_dict(shared) con ref_counting=1

destroy(&container);   // deve liberare correttamente shared, incluso il rehash
}

void test_shared_ownership(){
    wDict * shared = create_dictionary(3);
    insert_value(shared, "x", new_value(1));

    wDict * container_a = create_dictionary(3);
    wDict * container_b = create_dictionary(3);
    insert_value(container_a, "s", new_value(shared));
    insert_value(container_b, "s", new_value(shared));

    destroy(&container_a);
    int * v = (int*)get_value(shared, "x");
    printf("shared ancora vivo, valore=%d\n", v ? *v : -1);

    destroy(&container_b);
}

void test_update_overwrite_nested(){
    wDict * dict = create_dictionary(3);
    wDict * old_nested = create_dictionary(3);
    insert_value(dict, "k", new_value(old_nested));
    insert_value(dict, "k", new_value(99));  // sovrascrive: old_nested deve essere distrutto qui
    destroy(&dict);
}

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
    
    //resize_dict(dict,10);
    insert_value(dict,"beth2",new_value(list));
    insert_value(dict,"beth3",new_value(list));
    insert_value(dict,"beth4",new_value(list));
    list=NULL; //ownership list

    destroy(&dict);
    printf("pointer list %p\n",dict);
    destroy(&dict);
    destroy(&list);
    destroy(&list);
    test_shared_ownership();
    test_update_overwrite_nested();
    test_resize();
    return 0;
}