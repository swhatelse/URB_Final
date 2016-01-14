#include<stdio.h>

#include"../common.h"
#include"../communication.h"

#define GROUP_SIZE 3

void create_group(){
    group = g_hash_table_new(g_int_hash, g_int_equal);
   int i; 
    for(i = 0; i < GROUP_SIZE ; i++){
        node_t* peer = node_create(NULL);
        peer->id = i;
        if(!g_hash_table_insert(group, &peer->id, peer)){
            perror("OUPS! Group creation failed");
            exit(EXIT_FAILURE);
        }
    }
}

bool check_group(){
    if(g_hash_table_size(group) != GROUP_SIZE)
        return false;
    int i;
    for(i = 0; i < GROUP_SIZE ; i++){
        node_t* peer = g_hash_table_lookup(group, &i);
        if(peer->id != i){
            return false;
        }
    }
    return true;
}

bool acks_create_test(){
    GHashTable* table = acks_create();
    if(g_hash_table_size(table) != GROUP_SIZE){
        perror("Size of group not correct\n");
        return false;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, group);
    while(g_hash_table_iter_next (&iter, &key, &value)){
        bool* acks = g_hash_table_lookup(group, key);
        if(*acks != false){
            printf("[%d] true\n", *(int*)key);
            return false;
        }
        else{
            printf("[%d] false\n", *(int*)key);
        }
    }

    return true;
}

bool add_ack_test(){
    message_element_t* msg = (message_element_t*) malloc(sizeof(message_element_t*));
    msg->acks = acks_create();
    if(g_hash_table_size(msg->acks) != GROUP_SIZE){
        perror("Size of group not correct\n");
        return false;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, group);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        int* i = (int*) key;
        if(*i%2 == 0){
            add_ack(msg,key);
        }
        bool* ack = (bool*)g_hash_table_lookup(msg->acks, key);
        if(*i%2 == 0){
            if(ack){
                printf("[%d] true\n", *i);
            }
            else{
                return false;
            }
        }
        else{
            if(!ack){
                return false;                
            }
            else{
                printf("[%d] false\n", *i);
            }
        }
    }
    return true;
}

bool insert_message_test(){
    already_received = NULL;
    message_t* m1 = (message_t*) malloc(sizeof(message_t*));
    m1->id = 2;
    m1->node_id = 3;
    insert_message(m1, &already_received);

    GList* l1 = get_msg_from_list(already_received, m1);
    message_element_t* m1b = (message_element_t*)l1->data;
    if(m1b->msg->id == m1->id && m1b->msg->node_id == m1->node_id){
        return true;
    }
    else{
        return false;
    }
}

int main(int argc, char **argv){

    init("host.txt", "127.0.0.1", 9001);

    if(acks_create_test()){
        printf("Acks creation ok\n");
    }
    else{
        printf("Acks creation Error\n");
    }
    
    if(add_ack_test()){
        printf("Acks ok\n");
    }
    else{
        printf("Acks Error\n");
    }

    if(insert_message_test()){
        printf("Insert ok\n");
    }
    else{
        printf("Insert Error\n");
    }

    
    return 0;
}
