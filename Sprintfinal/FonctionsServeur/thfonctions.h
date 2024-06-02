#ifndef THFONCTIONS_H_
#define THFONCTIONS_H_


#include "global.h"
#include "traitmessage.h"
#include "commfonctions.h"
#include "traitsalons.h"
#include "traitclients.h"


void * uploadFile_th(void * fileNameParam);


void * downloadFile_th(void * fp);


void * broadcast(void * clientParam);

#endif