#ifndef __USERS_H__
#define __USERS_H__

/* extern variables to used across users.c and main.c */

/* struct design instructed by Frank Luan, a cs154 student last year */
/* he basically told me what structs I would need and that */
/* in particular, the 'name' field in friends will benefit me a lot */
typedef struct user_ user;
typedef struct udbase_ udbase;
typedef struct friends_ friends;
typedef struct client_ client;

struct user_{
    char* name;
    int fd;
    pthread_mutex_t lock;
    friends* friends;
    user* next;
};

struct udbase_{
    int user_n;
    user* first;
    pthread_mutex_t lock;
};

struct friends_{
    char* name;
    friends* next;
    user* user;
    impFriend_t status;
    pthread_mutex_t lock;
};

struct client_ {
    int fd;
    user* user;
    pthread_mutex_t lock;
    client* next;
};


extern udbase* udb;
void* udb_save(void* vargp);
void server_error(char* msg);
void udbase_init();
void udbase_print();
int udbase_count();
user* udbase_addUser(char* name);
int udbase_addFriend(user* user, char* name, impFriend_t status);
int udbase_removeFriend(user* user, char* name);
user* udbase_findUser(char* name);
friends* udbase_findFriend(user* user, char* name);
int udbase_changeStatus(user* user, char* fr, impFriend_t status);
void clfree(client* c);
void udbase_free();

#endif