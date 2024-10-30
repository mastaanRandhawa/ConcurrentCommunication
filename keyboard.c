#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "list.h"
#include "read.h"
#include "keyboard.h"
#include "sharedMutex.h"
#include "recv.h"
#include "send.h"

#define MAXLEN 10000

// Declare static variables for managing keyboard input thread, synchronization primitives, shared list, and text buffer
static pthread_t threadKeyboard;
static pthread_mutex_t mutexKeyboard = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t conditionKeyboard = PTHREAD_COND_INITIALIZER;
static List* list;
static char* text;

// Declaration of an infinite loop function for the keyboard input thread
static void* infinteWhileLoop(void* unused);

// Function to terminate the program safely, displaying an error message if provided
static void safeExit(const char* errorMsg, int errorCode) {
    if (errorMsg != NULL) {
        printf("%s\n", errorMsg);
    }
    exit(errorCode); // Terminate the entire process
}

// Function to signal the keyboard input thread to process incoming messages
void Signal_Keyboard()
{
    pthread_mutex_lock(&mutexKeyboard); // Acquire the mutex lock
    {
        pthread_cond_signal(&conditionKeyboard); // Signal the condition variable
    }
    pthread_mutex_unlock(&mutexKeyboard); // Release the mutex lock
}

// Function to initialize the keyboard input handling component
void Initalize_Keyboard(List* l){

    list = l; // Assign the shared list to the global variable

    // Create the keyboard input thread
    int writingThread =  pthread_create(&threadKeyboard, NULL, infinteWhileLoop, NULL);
    if(writingThread != 0){
        safeExit("Error: Thread create unsuccessful!", -1); // Exit if thread creation fails
    }

}

// Function to cancel the keyboard input thread
void Cancel_Keyboard()
{
    pthread_cancel(threadKeyboard);
}

// Function to shut down the keyboard input handling component
void Shutdown_Keyboard()
{
    free(text); // Free any remaining text buffer
    pthread_mutex_unlock(&globalMutex); // Unlock the global mutex (assuming it's locked)
    text = NULL;

    pthread_join(threadKeyboard, NULL); // Wait for the keyboard input thread to terminate
}

// The infinite loop function for the keyboard input thread
static void* infinteWhileLoop(void* unused){

    while(1) // Infinite loop
    {
        pthread_mutex_lock(&mutexKeyboard); // Acquire the mutex lock
        {
            pthread_cond_wait(&conditionKeyboard, &mutexKeyboard); // Wait for a signal to proceed
        }
        pthread_mutex_unlock(&mutexKeyboard); // Release the mutex lock
       
        int i = 0;
        int listNotEmpty = List_count(list) != 0; // Check if the shared list is not empty

        while (listNotEmpty) // Process each message in the shared list
        {
            i++;

            pthread_mutex_lock(&globalMutex); // Acquire the global mutex lock
            {
                text = List_trim(list); // Remove the last item from the list
            }
            pthread_mutex_unlock(&globalMutex); // Release the global mutex lock

            if(text == NULL) // Check if the text buffer is NULL
            {
                printf("Error: Trim unsuccessful!\n");
                break; // Exit the loop if the list is empty
            }

            int writeVar = write(1, text, strlen(text)); // Write the text to the standard output
            if(writeVar == -1)
            {
                safeExit("Error: Write unsuccessful!", -1); // Exit if writing to stdout fails
            }

            if(!strcmp(text, "!\n") && i == 1) // Check for a special exit command
            {
                free(text);
                text = NULL;
                return NULL; // Exit the thread
            }

            free(text); // Free the dynamically allocated text buffer
            text = NULL;

            listNotEmpty = List_count(list) != 0; // Update the loop condition
        }
    }
    return NULL; // Return NULL when the loop exits (though it's designed not to)
}
