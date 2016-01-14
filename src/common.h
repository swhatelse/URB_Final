#ifndef COMMON_H
#define COMMON_H

#include<stdio.h>
#include <string.h>
#include<stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<stdbool.h>
#include<pthread.h>
#include<assert.h>

#include"node.h"
#include"group.h"
#include"listener.h"
#include"communication.h"

/******************************************
 *
 *               Macros
 *
 *****************************************/
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define CYAN "\e[96m"
#define RESET "\e[m"

#define PRINT(string)                           \
    printf("%d: %s\n" , my_port, string)

#define DEBUG_ERR(fmt, ...)                                             \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, RED fmt RESET , ##__VA_ARGS__);                 \
    }while(0);

#define DEBUG_VALID(fmt, ...)                                           \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, GREEN fmt RESET , ##__VA_ARGS__);               \
    }while(0);

#define DEBUG_RECV(fmt, ...)                                            \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, CYAN fmt RESET , ##__VA_ARGS__);                \
    }while(0);

#define DEBUG_SEND(fmt, ...)                                            \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, YELLOW fmt RESET , ##__VA_ARGS__);                \
    }while(0);

#define DEBUG(fmt, ...)                                                 \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);                            \
    }while(0);

/******************************************
 *
 *               Types
 *
 *****************************************/

/******************************************
 *
 *               Global vars
 *
 *****************************************/

bool terminate;
int my_port;
struct sockaddr_in my_addr;

/******************************************
 *
 *               Functions
 *
 *****************************************/

int init(char *file, char *my_addr, int port);

// 
unsigned short get_my_port();
char* get_my_addr();
int get_my_id();

void set_my_port(unsigned short port);
void set_my_addr(char* addr);
void set_my_id(int id);

void program_halt();
#endif
