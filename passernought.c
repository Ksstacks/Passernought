#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void leetSpeak(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case 'a': case 'A': str[i] = '4'; break;
            case 'e': case 'E': str[i] = '3'; break;
            case 'i': case 'I': str[i] = '1'; break;
            case 'o': case 'O': str[i] = '0'; break;
            case 's': case 'S': str[i] = '5'; break;
            case 't': case 'T': str[i] = '7'; break;
        }
    }
}

void generatePasswords(char *letters, char *numbers, char *symbols, char words[][256], int wordCount, int passwordCount, int passwordLength, int useLeet, int prefixOption, int suffixOption, FILE *outputFile) {
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
        int wordIndex = rand() % wordCount;
        char selectedWord[256];
        strcpy(selectedWord, words[wordIndex]);

        if (useLeet) {
            leetSpeak(selectedWord);
        }

        int selectedWordLen = strlen(selectedWord);
        int prefixLen = 0;
        int suffixLen = 0;

        if (prefixOption || suffixOption) {
            if (passwordLength < selectedWordLen) {
                printf("Password length is too short for the selected word. Please increase the password length.\n");
                return;
            }

            if (prefixOption) {
                prefixLen = rand() % (passwordLength - selectedWordLen + 1); // Random prefix length
            }
            if (suffixOption) {
                suffixLen = passwordLength - selectedWordLen - prefixLen; // Remaining length for suffix
            }
        }

        char password[passwordLength + 1];

        // Generate prefix
        for (int j = 0; j < prefixLen; j++) {
            password[j] = allChars[rand() % index];
        }

        // Insert the word
        strncpy(&password[prefixLen], selectedWord, selectedWordLen);

        // Generate suffix
        for (int j = prefixLen + selectedWordLen; j < passwordLength; j++) {
            password[j] = allChars[rand() % index];
        }

        password[passwordLength] = '\0';

        // Print and optionally save the password
        printf("%s\n", password);
        if (outputFile != NULL) {
            fprintf(outputFile, "%s\n", password);
        }
    }
}

int readWordListFromFile(char *filePath, char words[][256]) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filePath);
        return 0;
    }

    int wordCount = 0;
    while (fgets(words[wordCount], 256, file) && wordCount < 256) {
        // Remove newline character
        words[wordCount][strcspn(words[wordCount], "\n")] = '\0';
        wordCount++;
    }

    fclose(file);
    return wordCount;
}

int main() {
    int passwordLength;
    int passwordCount;
    char letters[27];   // Assuming the input is a series of letters (a-z)
    char numbers[11];   // Assuming the input is a series of numbers (0-9)
    char symbols[33];   // Assuming the input is a series of symbols
    char words[256][256];  // Words used in word generation
    int wordCount = 0;
    char useWordList;   // To decide whether to use the word list or not
    char filePath[256]; // Path to the word list file
    int useLeet;        // To decide whether to use leet speak or not
    char saveToFile;    // To decide whether to save the passwords to a file
    char outputFilePath[256]; // Path to the output file
    int prefixOption;   // Option to add prefix
    int suffixOption;   // Option to add suffix

    printf("Enter the length of the passwords (1-32): ");
    scanf("%d", &passwordLength);

    if (passwordLength < 1 || passwordLength > 32) {
        printf("Invalid length. Please enter a number between 1 and 32.\n");
        return 1;
    }

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
        scanf("%255s", filePath); // Limiting the input to 255 characters

        wordCount = readWordListFromFile(filePath, words);
        if (wordCount == 0) {
            printf("No words read from file. Exiting.\n");
            return 1;
        }
    } else {
        printf("Enter words to use (separated by comma): ");
        scanf(" %255[^\n]", words[0]); // Read words separated by commas into a single string

        // Split the words into the words array
        char *token = strtok(words[0], ",");
        while (token != NULL && wordCount < 256) {
            strncpy(words[wordCount], token, 255);
            words[wordCount][255] = '\0';  // Ensure null termination
            wordCount++;
            token = strtok(NULL, ",");
        }
    }

    printf("Enter the number of passwords to generate: ");
    scanf("%d", &passwordCount);

    printf("Do you want to use leet speak? (1 for yes, 0 for no): ");
    scanf("%d", &useLeet);

    printf("Do you want to add a prefix? (1 for yes, 0 for no): ");
    scanf("%d", &prefixOption);

    printf("Do you want to add a suffix? (1 for yes, 0 for no): ");
    scanf("%d", &suffixOption);

    printf("Do you want to save the passwords to a file? (y/n): ");
    scanf(" %c", &saveToFile);

    FILE *outputFile = NULL;
    if (saveToFile == 'y' || saveToFile == 'Y') {
        printf("Enter the path to the output file: ");
        scanf("%255s", outputFilePath);
        outputFile = fopen(outputFilePath, "w");
        if (outputFile == NULL) {
            printf("Error opening file: %s\n", outputFilePath);
            return 1;
        }
    }

    if (passwordCount > 0) {
        printf("Generated passwords:\n");
        generatePasswords(letters, numbers, symbols, words, wordCount, passwordCount, passwordLength, useLeet, prefixOption, suffixOption, outputFile);
    } else {
        printf("Invalid password count.\n");
    }

    if (outputFile != NULL) {
        fclose(outputFile);
    }

    return 0;
}
