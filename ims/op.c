#include "ims.h"

int server_sendmsg(impMsg* a, int fd);

impError_t reg(client* c, char* name)
{
    if (c->user) {
        return IMP_ERROR_CLIENT_BOUND;
    }
    user* u = udbase_findUser(name);
    if (u) {
        return IMP_ERROR_USER_EXISTS;
    }
    int l = strlen(name);
    if (l > IMP_NAME_MAXLEN) {
        return IMP_ERROR_BAD_COMMAND;
    }
    udbase_addUser(name);
    return IMP_ERROR_UNKNOWN;
}

impError_t login(client* c, char* name)
{
    if (c->user) {
        return IMP_ERROR_CLIENT_BOUND;
    }
    user* u = udbase_findUser(name);
    if (!u) {
        return IMP_ERROR_USER_DOES_NOT_EXIST;
    }
    if (u->fd != -1){
        return IMP_ERROR_USER_ALREADY_ACTIVE;
    }
    lock(u);
    u->fd = c->fd;
    unlock(u);
    lock(c);
    c->user = u;
    unlock(c);
    return IMP_ERROR_UNKNOWN;
}

impError_t logout(client* c)
{
    if (!(c->user)) {
        return IMP_ERROR_CLIENT_NOT_BOUND;
    }
    lock(c->user);
    ((c->user)->fd) = -1;
    unlock(c->user);
    lock(c);
    c->user = NULL;
    unlock(c);
    return IMP_ERROR_UNKNOWN;
}

impError_t im(client* c, char* name, char* words)
{
    if (!(c->user)) {
        return IMP_ERROR_CLIENT_NOT_BOUND;
    }
    user* u = udbase_findUser(name);
    if (!u) {
        return IMP_ERROR_USER_DOES_NOT_EXIST;
    }
    friends* f = udbase_findFriend(c->user, name);
    if (!f) {
        return IMP_ERROR_NOT_FRIEND;
    }
    if (f->status != IMP_FRIEND_YES) {
        return IMP_ERROR_NOT_FRIEND;
    }
    if (u->fd == -1) {
        return IMP_ERROR_USER_NOT_ACTIVE;
    }
    int l = strlen(words);
    if (l > IMP_IM_MAXLEN || l < 1) {
        return IMP_ERROR_BAD_COMMAND;
    }
    return IMP_ERROR_UNKNOWN;
}

impError_t request(client* c, char* name)
{
    if (!(c->user)) {
        return IMP_ERROR_CLIENT_NOT_BOUND;
    }
    if (strcmp(name, c->user->name) == 0) {
        return IMP_ERROR_BAD_COMMAND;
    }
    user* u = udbase_findUser(name);
    if (!u) {
        return IMP_ERROR_USER_DOES_NOT_EXIST;
    }
    friends* f = udbase_findFriend(c->user, name);
    if (f) {
        if (f->status == IMP_FRIEND_YES) {
            return IMP_ERROR_FRIEND_ALREADY;
        } else if (f->status == IMP_FRIEND_REQUESTED) {
            return IMP_ERROR_REQUESTED_ALREADY;
        } else if (f->status == IMP_FRIEND_TOANSWER) {
            udbase_changeStatus(c->user, name, IMP_FRIEND_YES);
            udbase_changeStatus(f->user, c->user->name, IMP_FRIEND_YES);
            if (u->fd != -1) {
                    impMsg* p = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, c->user->name, IMP_FRIEND_YES, IMP_ACTIVE_YES, IMP_END);
                    server_sendmsg(p, u->fd);
            }
            impMsg* p = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, name, IMP_FRIEND_YES, IMP_ACTIVE_YES, IMP_END);
            server_sendmsg(p, c->fd);
            return IMP_ERROR_UNKNOWN;
        }
    }
    udbase_addFriend(c->user, name, IMP_FRIEND_REQUESTED);
    udbase_addFriend(u, c->user->name, IMP_FRIEND_TOANSWER);
    if (u->fd != -1) {
        impMsg* q = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, c->user->name, IMP_FRIEND_TOANSWER, IMP_ACTIVE_NOT, IMP_END);
        server_sendmsg(q, u->fd);
    }
    impMsg* p = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, name, IMP_FRIEND_REQUESTED, IMP_ACTIVE_NOT, IMP_END);
    server_sendmsg(p, c->fd);
    return IMP_ERROR_UNKNOWN;
}

impError_t remv(client* c, char* name)
{
    if (!(c->user)) {
        return IMP_ERROR_CLIENT_NOT_BOUND;
    }
    user* u = udbase_findUser(name);
    if (!u) {
        return IMP_ERROR_USER_DOES_NOT_EXIST;
    }
    if (strcmp(name, c->user->name) == 0) {
        return IMP_ERROR_BAD_COMMAND;
    }
    friends* f = udbase_findFriend(c->user, name);
    if (f != NULL && f->status == IMP_FRIEND_YES) {
        if (u->fd != -1) {
            impMsg* q = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, c->user->name, IMP_FRIEND_NOT, IMP_ACTIVE_NOT, IMP_END);
            server_sendmsg(q, u->fd);
        }
        udbase_removeFriend(c->user, name);
        udbase_removeFriend(f->user, c->user->name);
        return IMP_ERROR_UNKNOWN;
    } else if (f != NULL && f->status == IMP_FRIEND_REQUESTED) {
        if (u->fd != -1) {
            impMsg* q = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, c->user->name, IMP_FRIEND_NOT, IMP_ACTIVE_NOT, IMP_END);
            server_sendmsg(q, u->fd);
        }
        udbase_removeFriend(c->user, name);
        udbase_removeFriend(f->user, c->user->name);
        return IMP_ERROR_UNKNOWN;
    } else if (f != NULL && f->status == IMP_FRIEND_TOANSWER) {
        if (u->fd != -1) {
            impMsg* q = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, c->user->name, IMP_FRIEND_NOT, IMP_ACTIVE_NOT, IMP_END);
            server_sendmsg(q, u->fd);
        }
        udbase_removeFriend(c->user, name);
        udbase_removeFriend(f->user, c->user->name);
        return IMP_ERROR_UNKNOWN;
    }
    return IMP_ERROR_NOT_FRIEND;
}