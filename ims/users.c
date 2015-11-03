#include "ims.h"


udbase* udb;

void* udb_save(void* vargp)
{
    while (sleep(saveInterval) == 0) {
        filew = fopen(udbaseFilename, "w");
        udbase_print();
        fclose(filew);
    }
    return NULL;
}

void server_error(char* msg)
{
    fprintf(stderr, "SERVER_INIT_FAIL: %s\n", msg);
}

void udbase_init()
{
    udb = (udbase*)malloc(sizeof(udbase));
    udb->user_n = 0;
    udb->first = NULL;
    setLock(udb);
    user* cur = NULL;
    int user_num = 0;
    impFriend_t status;
    char buf[IMP_NAME_MAXLEN + 20];

    /* scan all usernames and friends */
    if(fgets(buf, IMP_NAME_MAXLEN + 20, filer)){
        char* k = strchr(buf, ' ');
        *k = '\0';
        user_num= atoi(buf);
    }
    char* end;
    cur = NULL;
    while (fgets(buf, IMP_NAME_MAXLEN + 20, filer)) {
        switch(buf[0]) {
        case '.':
            cur = NULL;
            break;
        case '-':
            if (strstr(buf, "requested")) {
                status = IMP_FRIEND_REQUESTED;
            } else if (strstr(buf, "toanswer")) {
                status = IMP_FRIEND_TOANSWER;
            } else {
                status = IMP_FRIEND_YES;
            }
            end = buf+2;
            while (*end != ' ' && *end != '\n') {
                end++;
            }
            *end = '\0';
            udbase_addFriend(cur, buf + 2, status);
            break;
        default:
            buf[strlen(buf) - 1] = '\0';
            cur = udbase_addUser(buf);
        }
    }
    if (user_num != udb->user_n) {
        perror("user number not correct");
        exit(1);
    }
    cur = udb->first;
    friends* cur_f;
    while (cur) {
        cur_f = cur->friends;
        while (cur_f) {
            if (!(cur_f->user)) {
                cur_f->user = udbase_findUser(cur_f->name);
            }
            cur_f = cur_f->next;
        }
        cur = cur->next;
    }
}

void udbase_print()
{
    lock(udb);
    if (udb->user_n == 0) {
        fprintf(filew, "%d users:\n", udb->user_n);
        unlock(udb);
        return;
    }
    user* cur = udb->first;
    friends* cur_f = cur->friends;
    fprintf(filew, "%d users:\n", udb->user_n);
    while (cur) {
        cur_f = cur->friends;
        fprintf(filew, "%s\n", cur->name);
        while (cur_f) {
            if (cur_f->status == IMP_FRIEND_REQUESTED) {
                fprintf(filew, "- %s requested\n", cur_f->name);    
            } else if (cur_f->status == IMP_FRIEND_TOANSWER) {
                fprintf(filew, "- %s toanswer\n", cur_f->name);    
            } else {
                fprintf(filew, "- %s\n", cur_f->name);
            }
            cur_f = cur_f->next;
        }
        fprintf(filew, ".\n");
        cur = cur->next;
    }
    unlock(udb);
}

int udbase_count()
{
    int n = 0;
    if (!udb) {
        perror("udbase is null");
        exit(1);
    }
    lock(udb);
    user* cur= udb->first;
    while (cur) {
        n++;
        cur = cur->next;
    }
    unlock(udb);
    return n;
}


user* udbase_addUser(char* name)
{
    if (udbase_findUser(name)) {
        return NULL;
    }
    user* u = malloc(sizeof(user));
    if (!u) {
        perror("malloc failed for user");
        exit(1);
    }
    u->friends = NULL;
    setLock(u);
    u->name = strdup(name);
    u->fd = -1;
    lock(udb);
    user* us = udb->first;
    if (!us) {
        udb->first = u;
        udb->user_n += 1;
        unlock(udb);
        return u;
    }
    while (us->next) {
        us = us->next;
    }
    us->next = u;
    udb->user_n += 1;
    unlock(udb);
    return u;
}

int udbase_addFriend(user* u, char* name, impFriend_t status)
{
    friends* f = (friends*)malloc(sizeof(friends));
    if (!f) {
        perror("malloc failed for friends");
        exit(1);
    }
    setLock(f);
    f->user = udbase_findUser(name);
    f->name = strdup(name);
    f->status = status;
    lock(u);
    friends* cur = u->friends;
    f->next = NULL;
    if (!cur) {
        u->friends = f;
        unlock(u);
        return 1;
    }
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = f;
    unlock(u);
    return 1; /* friends added successfully */
}


int udbase_removeFriend(user* user, char* name)
{
    friends* prev = NULL;
    friends* cur = user->friends;
    while (cur) {
        if (strcmp(cur->name,name) == 0) {
            if (!prev) {
                user->friends = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

user* udbase_findUser(char* name)
{
    if (!udb) {
        return NULL;
    }
    user* cur = udb->first;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


friends* udbase_findFriend(user* user, char* name)
{
    if (!user) {
        return NULL;
    }
    friends* cur = user->friends;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int udbase_changeStatus(user* user, char* fr, impFriend_t status)
{
    friends* f = udbase_findFriend(user, fr);
    if (!f) {
        return -1;
    }
    lock(f);
    f->status = status;
    unlock(f);
    return 1;
}

void clfree(client* c) {
    lock(c);
    lock(clients)
    close(c->fd);
    if (strcmp(c->user->name, clients->user->name) == 0) {
        clients->next = c->next;
    } else {
        client* prev = clients;
        while (prev->next != NULL && strcmp(prev->next->user->name, c->user->name) != 0) {
            prev = prev->next;
        }
        free(c);
        prev->next = c->next;
    }
    unlock(clients);
    unlock(c);
}

void udbase_free() {
    user* u = udb->first;
    while (u) {
        friends* f = u->friends;
        while (f) {
            friends* tempf = f;
            f = f->next;
            free(tempf->name);
            free(tempf);
        }
        user* tempu = u;
        u = u->next;
        free(tempu->name);
        free(tempu);
    }
    free(udb);
}