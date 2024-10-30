#ifndef SEND_H
#define SEND_H

#include "list.h"

void Cancel_Sender();
void Shutdown_Sender();
void Initialize_Sender(char* hostIP, char* hostInfo, List* l);
void Signal_Sender();


#endif