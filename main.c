#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "read.h"
#include "keyboard.h"
#include "recv.h"
#include "send.h"

void displayBanner(){
    printf("|-------------------------------------------------|\n");
    printf("|                                                 |\n");
    printf("|  Welcome to My Program                          |\n");
    printf("|                                                 |\n");
    printf("|  Developed by: Mastaan                          |\n");
    printf("|                                                 |\n");
    printf("|  s-talk syntax:                                 |\n");
    printf("|  ./s-talk (YOUR-PORT) (THEIR-IP) (THEIR-PORT)   |\n");
    printf("|                                                 |\n");
    printf("|-------------------------------------------------|\n");
    printf("                                                   \n");

}

void displayExitBanner(){
    printf("                                                   \n");
    printf("|-------------------------------------------------|\n");
    printf("|                                                 |\n");
    printf("|  Thank you for using our Program                |\n");
    printf("|                                                 |\n");
    printf("|  Developed by: Mastaan                          |\n");
    printf("|                                                 |\n");
    printf("|-------------------------------------------------|\n");

}

// Initialize all components required for the program to function
void initializeComponents(List* sharedList, char *localPort, char *remoteHost, char *remotePort){
    // Initialize reading, sending, receiving, and keyboard input components
    Initialize_Read(sharedList);
    Initialize_Sender(remoteHost, remotePort, sharedList);
    Initalize_Receiver(localPort, sharedList);
    Initalize_Keyboard(sharedList);
}

// Shutdown all program components cleanly
void shutdownComponents(){
    // Shutdown reading, sending, receiving, and keyboard input components
    Shutdown_Read();
    Shutdown_Sender();
    Shutdown_Receiver();
    Shutdown_Keyboard();
}

int main(int argc, char *argv[]){
    // Check for correct number of command-line arguments
    if (argc != 4) {
        printf("Error: wrong s-talk syntax.\n");
        printf("Correct syntax for s-talk is: ./s-talk |YOUR-PORT| |THEIR-IP| |THEIR-PORT|\n");
        return -1; // Return with error code if syntax is incorrect
    }
   
    displayBanner(); // Display welcome banner

    // Extract command-line arguments
    char *localPort = argv[1];
    char *remoteHost = argv[2];
    char *remotePort = argv[3];

    List* sharedList = List_create(); // Create a shared list for message handling
    initializeComponents(sharedList, localPort, remoteHost, remotePort); // Initialize program components

    shutdownComponents(); // Cleanly shutdown program components
    List_free(sharedList, free); // Free the shared list

    displayExitBanner(); // Display exit banner

    return 0;
}

