#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <pthread.h>

//mutex shared accross files to solve race conditions
extern pthread_mutex_t globalMutex;

#endif
