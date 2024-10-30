#ifndef RECV_H
#define RECV_H

#include "list.h"

void Shutdown_Receiver();
void Initalize_Receiver(char* localInfo, List* l);
void Cancel_Receiver();

#endif