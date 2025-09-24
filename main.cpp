// main.cpp 
#include <cstdio>   // printf
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include "auth.h"   // int authenticateUser(void);

int main(void) {
    printf("Booting SimpleOS...\n");

    int ok = authenticateUser();  // prompts for username/password

    if (ok) {
        printf("Login successful. Welcome!\n");
        return EXIT_SUCCESS;
    } else {
        printf("Authentication failed.\n");
        return EXIT_FAILURE;
    }
}
