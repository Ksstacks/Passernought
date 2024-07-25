#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void generatePasswords(char *letters, char *numbers, char *symbols, int passwordCount, int passwordLength) {
    char allChars[256];
    int index = 0;

    // Add letters to the pool
    for (int i = 0; letters[i] != '\0'; i++) {
        allChars[index++] = letters[i];
    }

    // Add numbers to the pool
    for (int i = 0; numbers[i] != '\0'; i++) {
        allChars[index++] = numbers[i];
    }

    // Add symbols to the pool
    for (int i = 0; symbols[i] != '\0'; i++) {
        allChars[index++] = symbols[i];
    }

    // Null-terminate the string of all characters
    allChars[index] = '\0';

    // Seed the random number generator
    srand(time(NULL));

    // Generate the passwords
    for (int i = 0; i < passwordCount; i++) {
        char password[passwordLength + 1];
        for (int j = 0; j < passwordLength; j++) {
            password[j] = allChars[rand() % index];
        }
        password[passwordLength] = '\0';
        printf("Generated password %d: %s\n", i + 1, password);
    }
}

void readWordListFromFile(char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filePath);
        return;
    }

    char line[256];
    printf("Word list from file:\n");
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        printf("%s\n", line);
    }

    fclose(file);
}

int main() {
    int passwordLength;
    int passwordCount;
    char letters[27];   // Assuming the input is a series of letters (a-z)
    char numbers[11];   // Assuming the input is a series of numbers (0-9)
    char symbols[33];   // Assuming the input is a series of symbols
    char words[200];    // words used in word generation
    char useWordList;   // To decide whether to use the word list or not
    char filePath[256]; // Path to the word list file

    printf("Enter the length of the passwords(1-32): ");
    scanf("%d", &passwordLength);

    if (passwordLength < 1 || passwordLength > 32) {
        printf("Invalid length. Please enter a number between 1 and 32.\n");
        return 1;
    }
    printf("Enter words to use(seperated by comma): ");
    scanf("%d", &passwordLength);
    
    printf("Enter letters (a-z): ");
    scanf("%26s", letters);  // Limiting the input to 26 characters (a-z)

    printf("Enter numbers (0-9): ");
    scanf("%10s", numbers);  // Limiting the input to 10 characters (0-9)

    printf("Enter symbols: ");
    scanf("%32s", symbols);  // Limiting the input to 32 characters

    printf("Do you want to use a list of words? (y/n): ");
    scanf(" %c", &useWordList);

    if (useWordList == 'y' || useWordList == 'Y') {
        printf("Enter the path to the word list file: ");
        scanf("%s", filePath);

        readWordListFromFile(filePath);

        printf("Enter the number of new passwords to generate: ");
        scanf("%d", &passwordCount);

        if (passwordCount > 0) {
            printf("Generated passwords:\n");
            generatePasswords(letters, numbers, symbols, passwordCount, passwordLength);
        } else {
            printf("Invalid password count.\n");
        }
    } else {
        printf("Enter the number of passwords to generate: ");
        scanf("%d", &passwordCount);

        if (passwordCount > 0) {
            printf("Generated passwords:\n");
            generatePasswords(letters, numbers, symbols, passwordCount, passwordLength);
        } else {
            printf("Invalid password count.\n");
        }
    }

    printf("Password length: %d\n", passwordLength);
    printf("Letters: %s\n", letters);

    return 0;
}
