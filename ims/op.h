#ifndef __OP_H__
#define __OP_H__

#include "ims.h"

impError_t reg(client* c, char* name);
impError_t login(client* c, char* name);
impError_t logout(client* c);
impError_t im(client* c, char* name, char* words);
impError_t request(client* c, char* name);
impError_t remv(client* c, char* name);

#endif