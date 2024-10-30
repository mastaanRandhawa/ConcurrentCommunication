#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "send.h"
#include "sharedMutex.h"
#include "list.h"
#include "read.h"
#include "keyboard.h"
#include "recv.h"

#define MAXLEN 10000

static List* list; // Declare a pointer to a shared list for storing messages
static pthread_t threadRead; // Declare a variable for the thread that reads input

// Function to safely exit the program with an error message and error code
static void safeExit(const char* errorMsg, int errorCode) {
    if (errorMsg != NULL) {
        printf("%s\n", errorMsg);
    }
    exit(errorCode); // Terminate the process
}

// Declaration of an infinite loop function for the reading thread
static void* infiniteLoop(void* useless);

// Function to initialize the reading component
void Initialize_Read(List* l){
    list = l; // Assign the provided list to the global list variable
    // Attempt to create the reading thread
    if(pthread_create(&threadRead, NULL, infiniteLoop, NULL) !=0){
        safeExit("Error: Thread create unsuccessful!", -1); // Exit if thread creation fails
    }
}

// Function to cancel the reading thread
void Cancel_Read()
{
    pthread_cancel(threadRead);
}

// Function to shutdown the reading component
void Shutdown_Read()
{
    pthread_join(threadRead,NULL); // Wait for the reading thread to terminate
}

// The infinite loop function for the reading thread
static void* infiniteLoop(void* useless){
    while(1) // Enter an infinite loop
    {
        char* text; // Declare a pointer for dynamically allocated text
        char storeMessage[MAXLEN]; // Declare a buffer to store the input message
        ssize_t bytesRead; // Variable to store the number of bytes read
       
        int shouldContinue = 1; // Initialize loop condition
        int i = 0; // Initialize counter

        while(shouldContinue && i != 100){

            i++; // Increment the counter

            memset(&storeMessage, 0, MAXLEN); // Clear the input string buffer

            bytesRead = read(0, storeMessage, MAXLEN); // Read user input
            if(bytesRead == -1)
            {
                safeExit("Error: Read unsuccessful!", -1); // Exit if read fails
            }

            text = (char*)malloc(sizeof(char)*(bytesRead+1)); // Allocate memory for the text
            strncpy(text, storeMessage, bytesRead); // Copy the input to the allocated memory
            text[bytesRead] = '\0'; // Null-terminate the string

            int success;
            pthread_mutex_lock(&globalMutex); // Lock the global mutex before accessing the shared list
            {
                success = List_prepend(list, text); // Prepend the text to the list
            }
            pthread_mutex_unlock(&globalMutex); // Unlock the mutex after modifying the list
            if(success == -1)
            {
                printf("Error: List prepend unsuccessful!\n");
            }

            if (!strcmp(text, "!\n") && i == 1) // Check for a special exit command
            {
                Signal_Sender(); // Signal the sender component
                Cancel_Receiver(); // Cancel the receiver thread
                Cancel_Keyboard(); // Cancel the keyboard thread
                return NULL; // Exit the thread
            }

            shouldContinue = (storeMessage[bytesRead-1] != '\n'); // Update loop condition based on the last character read
        }

        Signal_Sender(); // Signal the sender component after processing input
    }
    return NULL; // Return NULL when the loop exits (though it's designed not to exit)
}