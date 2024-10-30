#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "list.h"

void Signal_Keyboard();
void Cancel_Keyboard();
void Initalize_Keyboard(List* l);
void Shutdown_Keyboard();

#endif