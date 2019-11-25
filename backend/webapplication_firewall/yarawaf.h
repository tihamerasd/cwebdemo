#ifndef YARAWAF_H
#define YARAWAF_H
#include <yara.h>
#include "threads.h"
extern thread_local int match;

int yarafunction(char*, int);

#endif //YARAWAF_H
