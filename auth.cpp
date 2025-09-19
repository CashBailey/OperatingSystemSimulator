#include "auth.h"
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 128

// Prints a credential prompt and flushes stdout
static void printPrompt(const char *text) {
    printf("%s", text);
    fflush(stdout);
}

int authenticateUser(void) {
    const char *expectedUsername = "admin";
    const char *expectedPassword = "password123";

    char username[MAX_INPUT];
    char password[MAX_INPUT];

    // Prompt for username
    printPrompt("Username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("Access denied.\n");
        return 0;
    }
    username[strcspn(username, "\n")] = '\0';

    // Prompt for password
    printPrompt("Password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        printf("Access denied.\n");
        return 0;
    }
    password[strcspn(password, "\n")] = '\0';

    int usernameMatches = strcmp(username, expectedUsername) == 0;
    int passwordMatches = strcmp(password, expectedPassword) == 0;

    if (usernameMatches && passwordMatches) {
        printf("Access granted.\n");
        return 1;
    }

    printf("Access denied.\n");
    return 0;
}
