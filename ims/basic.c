/*
** cs154-2015 Project 5 ("p5ims") Instant Message Server
** basic.c: for functions that aren't called per-client-thread
*/

#include "ims.h"

#define BACKLOG 10

void serverAccept();
int serverStart(impEm *iem);
void* serve_client(void* vargp);
void reply_success(impOp_t opt, client* c, char* name);
void reply_error(impError_t e, client* c, char* name);
void notify(user* u, impActive_t act);
void list_friends(user* u);
int server_send(char* s, int fd);
int server_sendmsg(impMsg* a, int fd);
void serverStop(void);

int cquit;
int sockfd;
client* clients;
FILE* filer;
FILE* filew;
int casecount = 0;

int sendc = 0;

/* call serverStop() upon getting "q" or "quit" on stdin */
void* readQuitFromStdin(void* vargp) {
  impEm* em = (impEm*)vargp;
  char *line=NULL;
  size_t lsize=0;
  while (1) {
    ssize_t glret;
    if (verbose > 0) {
      printf("Type \"q\" or \"quit\" to cleanly quit the server\n");
    }
    glret = getline(&line, &lsize, stdin);
    /* Ctrl-D or EOF will also break out of this loop */
    if (glret <= 0 || !strcmp("quit\n", line) || !strcmp("q\n", line)) {
      /* tell things to quit gracefully */
      free(line);
      impEmFree(em);
      quitting = 1;
      serverStop();
      /* anything else to do here? */
      break;
    }

    /* ... else !strcmp("foo\n", line) to see if user typed "foo" and then
       return. You can use this to add your own additional commands here, like
       for querying globals or other aspects of server's internal state */

  }
  return NULL;
}


int serverStart(impEm *iem) {
  /* ================ thread 3 checking for quit ================ */
  pthread_t tid2;
  pthread_create(&tid2, NULL, readQuitFromStdin, (void*)iem);

  static const char me[]="serverStart";
  filer = fopen(udbaseFilename, "r");
  if (verbose > 1) {
    printf("%s: server starting\n", me);
  }
  if (udbaseRead(iem)) {
    impEmAdd(iem, "%s: failed to read database file \"%s\"",
             me, udbaseFilename);
    return 1;
  }
  fclose(filer);
  /* immediately try writing database, so that any errors here can be
     reported as a failure of server start-up. Whether and how you do
     error handling for subsequent calls to udbaseWrite() is up to you */
  if (udbaseWrite(iem)) {
    impEmAdd(iem, "%s: failed to write database file \"%s\"",
             me, udbaseFilename);
    return 1;
  }

  /* YOUR CODE HERE:
     -- create listening file descriptor for listenPort and listen() on it
     See http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
     and May 18 2015 class slides
     -- start a thread to periodically save database
     -- figure out whether looking for "quit" on stdin should be done
     in a thread that is started here, or in main.c
  */
    /* code here basically referenced from the sample stream server on Beej's guide */
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int yes = 1;
    char port[20];
    sprintf(port, "%u", listenPort);
    if (getaddrinfo(NULL, port, &hints, &servinfo) == -1) {
            server_error("getaddrinfo error");
            exit(1);
    }

    for (p = servinfo; p; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            server_error("socket failed");
            continue;
        }
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
            server_error("setsockopt error");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            server_error("binding failed");
            continue;
        }
        break;
    }

    if (!p) {
        server_error("Failed binding eventually");
        exit(2);
    }
    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) == -1) {
        server_error("failed listening");
        exit(1);
    }
  if (verbose) {
    printf("%s: server started on part %u from %s\n",
           me, listenPort, udbaseFilename);
  }
  if (verbose > 1) {
    printf("%s: bye\n", me);
  }

    serverAccept();
  return 0;
}

void serverAccept() {
    int new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    while (1) {
    sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            server_error("accept failed");
            continue;
        }
        pthread_t tid;
        client* c = (client*)malloc(sizeof(client));
        c->fd = new_fd;
        c->next = clients;
        clients = c;
        c->user = NULL;
        setLock(c);
        pthread_create(&tid, NULL, serve_client, (void*)c);
    }
    //printf("=============here=============\n");
}

void* serve_client(void* vargp) {
    pthread_detach(pthread_self());
    if (verbose) {
        printf("handling client request\n");
    }
    client* c = (client*)vargp;
    char pm[3000];
    reply_success(IMP_OP_CONNECT, c, NULL);
    char* start;
    while (1) {
        if (cquit) {
            //printf("==============1==============\n");
            break;
        }
        memset(pm, 0, 3000);
        if (recv(c->fd, pm, 3000, 0) <= 0) {
            //printf("==============2==============\n");
            break;
        }
        start = pm;
        char* p = strchr(start, '\n');
        do {
            *p = '\0';
            impEm* em = impEmNew();
            impMsg* msg = impStrToMsg(em, start);
            start = p + 1;
            if (!msg) {
                impMsg* a = impMsgNew(em, IMP_MSG_TYPE_ERROR, IMP_ERROR_BAD_COMMAND, IMP_END);
                server_sendmsg(a, c->fd);
                impEmFprint(stderr, em);
                impEmFree(em);
                continue;
            }
            if (msg->mt != IMP_MSG_TYPE_OP) {
                impMsg* a = impMsgNew(em, IMP_MSG_TYPE_ERROR, IMP_ERROR_BAD_COMMAND, IMP_END);
                server_sendmsg(a, c->fd);
                impEmFprint(stderr, em);
                impEmFree(em);
                continue;
            }

            impMsgOp* mop = (impMsgOp*)msg;
            impError_t e; /* feedback from op.c */

            switch (mop->op) {
            case IMP_OP_UNKNOWN:;
                impMsg* a = impMsgNew(em, IMP_MSG_TYPE_ERROR, IMP_ERROR_BAD_COMMAND, IMP_END);
                server_sendmsg(a, c->fd);
                impEmFprint(stderr, em);
                impEmFree(em);
                break;

            case IMP_OP_REGISTER:
                e = reg(c, msg->userName);
                if (e == IMP_ERROR_UNKNOWN) {
                    reply_success(IMP_OP_REGISTER, c, msg->userName);
                } else {
                    reply_error(e, c, msg->userName);
                }
                break;


            case IMP_OP_LOGIN:
                e = login(c, msg->userName);
                if (e == IMP_ERROR_UNKNOWN) {
                    reply_success(IMP_OP_LOGIN, c, msg->userName);
                    notify(c->user, IMP_ACTIVE_YES);
                    list_friends(c->user);
                } else {
                    reply_error(e, c, msg->userName);
                }
                break;

            case IMP_OP_LOGOUT:;
                user* u = c->user;
                e = logout(c);
                if (e == IMP_ERROR_UNKNOWN) {
                    reply_success(IMP_OP_LOGOUT, c, NULL);
                    notify(u, IMP_ACTIVE_NOT);
                } else {
                    reply_error(e, c, msg->userName);
                }
                break;


            case IMP_OP_FRIEND_REQUEST:
                e = request(c, msg->userName);
                if (e != IMP_ERROR_UNKNOWN) {
                    reply_error(e, c, msg->userName);
                }
                break;

            case IMP_OP_FRIEND_REMOVE:    
                e = remv(c, msg->userName);
                if (e == IMP_ERROR_UNKNOWN) {
                    if ((udbase_findUser(msg->userName))->fd == -1) {
                        impMsg* p = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, msg->userName, IMP_FRIEND_NOT, IMP_ACTIVE_NOT, IMP_END);
                        server_sendmsg(p, c->fd);
                    } else {
                        impMsg* p = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, msg->userName, IMP_FRIEND_NOT, IMP_ACTIVE_NOT, IMP_END);
                        server_sendmsg(p, c->fd);                    
                    }
                } else {
                    reply_error(e, c, msg->userName);
                }
                break;

            case IMP_OP_FRIEND_LIST:
                if (!(c->user)) {
                    reply_error(IMP_ERROR_CLIENT_NOT_BOUND, c, msg->userName);
                } else {
                    list_friends(c->user);
                }
                break;

            case IMP_OP_IM:
                e = im(c, msg->userName, ((impMsgOp*)msg)->IM);
                if (e == IMP_ERROR_UNKNOWN) {
                    reply_success(IMP_OP_IM, c, msg->userName);
                    user* uu = udbase_findUser(msg->userName);
                    impMsg* content = impMsgNew(NULL, IMP_MSG_TYPE_OP, IMP_OP_IM, c->user->name, ((impMsgOp*)msg)->IM, IMP_END);
                    server_sendmsg(content, uu->fd);
                } else {
                    reply_error(e, c, msg->userName);
                }
                break;
            default:;
            }
        } while ((p = strchr(start, '\n')));
    }
    //lock(clients);

    if (verbose) {
        printf("logging out client\n");
    }
    client* curc = clients;
    if (curc == c) {
        clients = c->next;
        free(c);
        //unlock(clients);
        return NULL;
    }
    while (curc->next && curc->next != c) {
        curc = curc->next;
    }
    curc->next = c->next;
    free(c);
    //unlock(clients);
    return NULL;
}

void notify(user* u, impActive_t act) {
    if (verbose > 1) {
        printf("notifying friends for %s\n", u->name);
    }
    friends* f = u->friends;
    while (f) {
        if (f->status == IMP_FRIEND_YES && f->user->fd != -1) {
            impMsg* msg = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, u->name, IMP_FRIEND_YES, act, IMP_END);
            server_sendmsg(msg, f->user->fd);
        }
        f = f->next;
    }
}

void list_friends(user* u) {
    if (verbose > 1) {
        printf("listing friends for %s\n", u->name);
    }
    friends* f = u->friends;
    while (f) {
        impActive_t act;
        if (f->user->fd != -1 && f->status == IMP_FRIEND_YES) {
            act = IMP_ACTIVE_YES;
        } else {
            act = IMP_ACTIVE_NOT;
        }
        impMsg* msg = impMsgNew(NULL, IMP_MSG_TYPE_STATUS, f->name, f->status, act, IMP_END);
        server_sendmsg(msg, u->fd);
        f = f->next;
    }
}

int server_send(char* s, int fd) {
    if (send(fd, s, strlen(s), 0) == -1) {
        perror("send failed");
        return -1;
    }
    return 1;
}

extern int server_sendmsg(impMsg* a, int fd) {
    char* s = impMsgToStr(NULL, a);
    int ret = server_send(s, fd);
    free(s);
    impMsgFree(a);
    return ret;  
}

void reply_error(impError_t e, client* c, char* name) {
    impMsg* msg;
    if (e == IMP_ERROR_BAD_COMMAND ||
        e == IMP_ERROR_USER_DOES_NOT_EXIST ||
        e == IMP_ERROR_CLIENT_NOT_BOUND) {
        msg = impMsgNew(NULL, IMP_MSG_TYPE_ERROR, e, IMP_END);
    } else {
        msg = impMsgNew(NULL, IMP_MSG_TYPE_ERROR, e, name, IMP_END);
    }
    server_sendmsg(msg, c->fd);
}

void reply_success(impOp_t opt, client* c, char* name) {
    impMsg* msg;
    if (name) {
        msg = impMsgNew(NULL, IMP_MSG_TYPE_ACK, opt, name, IMP_END);
    } else {
        msg = impMsgNew(NULL, IMP_MSG_TYPE_ACK, opt, IMP_END);
    }
    server_sendmsg(msg, c->fd);
}

void serverStop(void) {
  static const char me[]="serverStop";
  cquit = 1;
  if (verbose > 0) {
    printf("%s: this server is to going to quit\n", me);
  }
  udbase_free();
  close(sockfd);
  if (verbose == 2) {
    printf("hope you enjoyed this ims service\n");
  }
  if (verbose > 0) {
    printf("%s: bye\n", me);
  }
  exit(0);
}

