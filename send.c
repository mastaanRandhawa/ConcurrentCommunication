#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "send.h"
#include "list.h"
#include "sharedMutex.h"

// Global variables for network communication and threading
static int socketNUM = -1; // Socket descriptor
static char* remoteIP = NULL; // Remote IP address
static char* remotePort = NULL; // Remote port number
static List* list = NULL; // Shared list for storing messages to be sent
static pthread_t threadSend; // Thread for sending messages
static struct addrinfo *servinfo = NULL; // Address info for the remote server
static pthread_mutex_t mutexSend = PTHREAD_MUTEX_INITIALIZER; // Mutex for send operation synchronization
static pthread_cond_t conditionSend = PTHREAD_COND_INITIALIZER; // Condition variable for signaling the send operation

// Declaration of the sender thread's main function
static void* infiniteWhileLoop(void* unused);

// Function to safely exit the program with an error message and an error code
static void safeExit(const char* errorMsg, int errorCode) {
    if (errorMsg != NULL) {
        printf("%s\n", errorMsg);
    }
    exit(errorCode); // Terminate the process
}

// Function to signal the sender thread that messages are available for sending
void Signal_Sender(){
    pthread_mutex_lock(&mutexSend);
    {
        pthread_cond_signal(&conditionSend); // Signal the condition variable
    }
    pthread_mutex_unlock(&mutexSend);
}

// Function to cancel the sender thread
void Cancel_Sender()
{
    pthread_cancel(threadSend);
}

// Function to initialize the sender component
void Initialize_Sender(char* hostIP, char* hostInfo, List* l)
{
    remoteIP = hostIP; // Set the remote IP
    remotePort = hostInfo; // Set the remote port
    list = l; // Set the shared list

    // Create the sender thread
    if(pthread_create(&threadSend, NULL, infiniteWhileLoop, NULL) != 0)
    {
        safeExit("Error: Thread create unsuccessful!", -1);
    }
}

// Function to shutdown the sender component
void Shutdown_Sender()
{
    // Cancel and join the sender thread
    pthread_cancel(threadSend);
    pthread_join(threadSend, NULL);

    // Free resources
    pthread_mutex_lock(&globalMutex);
    if (servinfo != NULL) {
        freeaddrinfo(servinfo); // Free the address info
        servinfo = NULL;
    }
    pthread_mutex_unlock(&globalMutex);

    if (socketNUM != -1) {
        close(socketNUM); // Close the socket
    }
}

// Main function of the sender thread
static void* infiniteWhileLoop(void* unused) {
    struct addrinfo netData, *info;
    memset(&netData, 0, sizeof(netData));
    netData.ai_family = AF_INET; // IPv4
    netData.ai_socktype = SOCK_DGRAM; // Datagram socket for UDP

    // Retrieve address information for the remote server
    pthread_mutex_lock(&globalMutex);
    int addressNum = getaddrinfo(remoteIP, remotePort, &netData, &servinfo);
    pthread_mutex_unlock(&globalMutex);

    if (addressNum != 0) {
        safeExit("Error: Address fetching unsuccessful!", -1);
    }

    // Create a socket for the first valid address found
    for (info = servinfo; info != NULL; info = info->ai_next) {
        socketNUM = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (socketNUM == -1) continue; // Try the next address if socket creation fails

        break; // Exit the loop if socket creation is successful
    }

    if (info == NULL) { // Check if socket creation was unsuccessful for all addresses
        safeExit("Error: Socket unsuccessful!", -1);
    }

    // Main loop for sending messages
    while (1) {
        pthread_mutex_lock(&mutexSend);
        {
            // Wait for a signal indicating that messages are available to send
            pthread_cond_wait(&conditionSend, &mutexSend);
        }
        pthread_mutex_unlock(&mutexSend);

        char* message = NULL;
        // Send all available messages
        while ((message = List_trim(list)) != NULL) {
            int numbytes = sendto(socketNUM, message, strlen(message), 0, info->ai_addr, info->ai_addrlen);
            free(message); // Free the memory allocated for the message

            if (numbytes == -1) {
                printf("Error: Sending unsuccessful!\n");
                continue; // Attempt to send the next message if the current one fails
            }
        }
    }
    return NULL; // To satisfy the compiler, though the loop is designed to be infinite
}
