#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include "recv.h"
#include "keyboard.h"
#include "send.h"
#include "list.h"
#include "sharedMutex.h"
#include "read.h"

#define MAXLEN 10000

static int socketNUM;
static struct addrinfo *remoteInfo;
static char* localPort;
static List* list;
static pthread_t threadReciev;

// Declaration of an infinite loop function for the receiving thread
static void* infiniteWhileLoop (void* unused);

// Function to safely exit the program with an error message and error code
static void safeExit(const char* errorMsg, int errorCode) {
    if (errorMsg != NULL) {
        printf("%s\n", errorMsg);
    }
    exit(errorCode); // Terminate the process
}

// Function to initialize the receiver component
void Initalize_Receiver(char* localInfo, List* l)
{
    localPort = localInfo; // Store the local port information
    list = l; // Store the reference to the shared list

    // Attempt to create the receiving thread
    int rectVal = pthread_create(&threadReciev, NULL, infiniteWhileLoop, NULL);
    if(rectVal != 0) // Check for thread creation failure
    {
        safeExit("Error: Thread creation unsuccessful!", -1);
    }
}

// Function to cancel the receiving thread
void Cancel_Receiver()
{
    pthread_cancel(threadReciev);
}

// Function to shutdown the receiver component
void Shutdown_Receiver()
{
    // Free the address information obtained from getaddrinfo
    pthread_mutex_lock(&globalMutex);
    {
        freeaddrinfo(remoteInfo);
    }
    pthread_mutex_unlock(&globalMutex);

    // Close the socket connection
    close(socketNUM);
    // Wait for the receiving thread to finish
    pthread_join(threadReciev, NULL);
}

// The infinite loop function for the receiving thread
static void* infiniteWhileLoop (void* unused)
{
    struct addrinfo netData, *info;
    int addrsNUM;
    int bindingNUM;
    char storeMessage[MAXLEN]; // Buffer for storing incoming messages
    char* text; // Pointer for dynamically allocated message storage
    struct sockaddr_in remoteAddress; // Struct for storing the remote address
    socklen_t remoteAddressLen; // Variable to store the length of the remote address

    // Initialize the addrinfo struct for getaddrinfo
    memset(&netData, 0 ,sizeof (netData));
    netData.ai_family = AF_INET; // IPv4
    netData.ai_socktype = SOCK_DGRAM; // Datagram socket for UDP
    netData.ai_flags = AI_PASSIVE; // Use my IP

    // Get address information
    addrsNUM = getaddrinfo(NULL, localPort, &netData, &remoteInfo);
    if (addrsNUM != 0 ) // Check for getaddrinfo failure
    {
        safeExit("Error: Address fetching unsuccessful!",-1);
    }

    // Iterate through all the results and bind to the first we can
    for(info = remoteInfo; info!=NULL; info=info->ai_next)
    {
        socketNUM = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (socketNUM ==-1) continue; // Try next address if socket creation fails

        bindingNUM = bind(socketNUM, info->ai_addr, info->ai_addrlen);
        if(bindingNUM ==-1) // Check for bind failure
        {
            close(socketNUM); // Close the socket and try the next
            continue;
        }
        break; // Successfully bound to an address
    }
    if(info==NULL) // Check if binding was unsuccessful for all addresses
    {
        safeExit("Error: Bind check unsuccessful!",-1);
    }

    // Main loop for receiving messages
    while(1)
    {
        int i = 0;
        int shouldContinue = 1; // Initialize loop continuation condition

        while (shouldContinue && i < 100)
        {
            i++;
            memset(&storeMessage, 0, MAXLEN); // Clear the message buffer

            // Receive messages
            remoteAddressLen = sizeof(remoteAddress);
            int numbytes = recvfrom(socketNUM, storeMessage, MAXLEN, 0, (struct sockaddr *)&remoteAddress, &remoteAddressLen);
            if(numbytes == -1) // Check for receive failure
            {
                safeExit("Error: Receive unsuccessful!",-1);
            }

            // Copy the received message to dynamically allocated memory
            text = (char*)malloc(sizeof(char)*(numbytes+1));
            strncpy(text, storeMessage, numbytes);
            text[numbytes] = '\0';

            // Add the message to the shared list
            int success;
            pthread_mutex_lock(&globalMutex);
            {
                success = List_prepend(list, text);
            }
            pthread_mutex_unlock(&globalMutex);
            if(success == -1) // Check for list prepend failure
            {
                printf("Error: List prepend unsuccessful!\n");
                free(text); // Free the allocated memory if prepend fails
            }

            // Check for a special exit command
            if(!strcmp(text, "!\n") && i == 1)
            {
                Signal_Keyboard(); // Signal the keyboard component
                Cancel_Read(); // Cancel the reading thread
                Cancel_Sender(); // Cancel the sending thread
                return NULL; // Exit the receiving thread
            }

            // Update loop continuation condition based on the last received message
            shouldContinue = (storeMessage[numbytes-1] != '\n');
        }
        Signal_Keyboard(); // Signal the keyboard component after processing messages
    }
    return NULL; // Return NULL when the loop exits (though it's designed not to exit)
}