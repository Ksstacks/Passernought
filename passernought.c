#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 8

typedef struct {
    char *letters;
    char *numbers;
    char *symbols;
    char **words;  // Pointer to array of strings
    int wordCount;
    int passwordCount;
    int passwordLength;
    int useLeet;
    int prefixOption;
    int suffixOption;
    FILE *outputFile;
    int threadId;
    int totalThreads;
} ThreadData;

void printProgressBar(int current, int total) {
    int barWidth = 70;
    float progress = (float)current / total;
    int pos = barWidth * progress;
    printf("\r[");
    for (int i = 0; i < barWidth; i++) {
        if (i < pos)
            printf("#");
        else
            printf(" ");
    }
    printf("] %.2f%%", progress * 100.0);
    fflush(stdout);
}

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

int readWordListFromFile(char *filePath, char **words) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filePath);
        return 0;
    }

    int wordCount = 0;
    char buffer[256];
    while (fgets(buffer, 256, file) && wordCount < 256) {
        // Remove newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        words[wordCount] = strdup(buffer); // Allocate and copy the word
        wordCount++;
    }

    fclose(file);
    return wordCount;
}

void *threadedPasswordGeneration(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char allChars[256];
    int index = 0;

    // Add letters to the pool
    for (int i = 0; data->letters[i] != '\0'; i++) {
        allChars[index++] = data->letters[i];
    }

    // Add numbers to the pool
    for (int i = 0; data->numbers[i] != '\0'; i++) {
        allChars[index++] = data->numbers[i];
    }

    // Add symbols to the pool
    for (int i = 0; data->symbols[i] != '\0'; i++) {
        allChars[index++] = data->symbols[i];
    }

    // Null-terminate the string of all characters
    allChars[index] = '\0';

    // Calculate range for this thread
    int start = (data->passwordCount / data->totalThreads) * data->threadId;
    int end = (data->passwordCount / data->totalThreads) * (data->threadId + 1);
    if (data->threadId == data->totalThreads - 1) {
        end = data->passwordCount;
    }

    // Generate the passwords
    for (int i = start; i < end; i++) {
        int wordIndex = rand() % data->wordCount;
        char selectedWord[256];
        strcpy(selectedWord, data->words[wordIndex]);

        // Generate leet and non-leet variations
        for (int j = 0; j < (data->useLeet ? 2 : 1); j++) {
            if (j == 1) {
                leetSpeak(selectedWord);
            }

            int selectedWordLen = strlen(selectedWord);
            int prefixLen = 0;
            int suffixLen = 0;

            if (data->prefixOption || data->suffixOption) {
                if (data->passwordLength < selectedWordLen) {
                    printf("Password length is too short for the selected word. Please increase the password length.\n");
                    return NULL;
                }

                if (data->prefixOption) {
                    prefixLen = rand() % (data->passwordLength - selectedWordLen + 1); // Random prefix length
                }
                if (data->suffixOption) {
                    suffixLen = data->passwordLength - selectedWordLen - prefixLen; // Remaining length for suffix
                }
            }

            char password[data->passwordLength + 1];

            // Generate prefix
            for (int k = 0; k < prefixLen; k++) {
                password[k] = allChars[rand() % index];
            }

            // Insert the word
            strncpy(&password[prefixLen], selectedWord, selectedWordLen);

            // Generate suffix
            for (int k = prefixLen + selectedWordLen; k < data->passwordLength; k++) {
                password[k] = allChars[rand() % index];
            }

            password[data->passwordLength] = '\0';

            // Optionally save the password
            if (data->outputFile != NULL) {
                fprintf(data->outputFile, "%s\n", password);
            }
        }

        // Print the progress bar
        printProgressBar(i + 1, data->passwordCount);
    }
    printf("\n"); // Move to the next line after the progress bar completes
    return NULL;
}

int main() {
    int passwordLength;
    int passwordCount;
    char letters[27];   // Assuming the input is a series of letters (a-z)
    char numbers[11];   // Assuming the input is a series of numbers (0-9)
    char symbols[33];   // Assuming the input is a series of symbols
    char *words[256];   // Pointer to array of strings
    int wordCount = 0;
    char useWordList;   // To decide whether to use the word list or not
    char filePath[256]; // Path to the word list file
    int useLeet;        // To decide whether to use leet speak or not
    char saveToFile;    // To decide whether to save the passwords to a file
    char outputFilePath[256]; // Path to the output file
    int prefixOption;   // Option to add prefix
    int suffixOption;   // Option to add suffix
    int totalThreads;   // Total number of threads to use

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
        char inputWords[1024];
        scanf(" %1023[^\n]", inputWords); // Read words separated by commas into a single string

        // Split the words into the words array
        char *token = strtok(inputWords, ",");
        while (token != NULL && wordCount < 256) {
            words[wordCount] = strdup(token); // Allocate and copy the word
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

    printf("Enter the number of threads to use (1-%d): ", MAX_THREADS);
    scanf("%d", &totalThreads);

    if (totalThreads < 1 || totalThreads > MAX_THREADS) {
        printf("Invalid number of threads. Please enter a number between 1 and %d.\n", MAX_THREADS);
        return 1;
    }

    pthread_t threads[totalThreads];
    ThreadData threadData[totalThreads];

    // Initialize thread data and create threads
    for (int i = 0; i < totalThreads; i++) {
        threadData[i].letters = letters;
        threadData[i].numbers = numbers;
        threadData[i].symbols = symbols;
        threadData[i].words = words;
        threadData[i].wordCount = wordCount;
        threadData[i].passwordCount = passwordCount;
        threadData[i].passwordLength = passwordLength;
        threadData[i].useLeet = useLeet;
        threadData[i].prefixOption = prefixOption;
        threadData[i].suffixOption = suffixOption;
        threadData[i].outputFile = outputFile;
        threadData[i].threadId = i;
        threadData[i].totalThreads = totalThreads;

        pthread_create(&threads[i], NULL, threadedPasswordGeneration, &threadData[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < totalThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    if (outputFile != NULL) {
        fclose(outputFile);
    }

    // Free allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }

    return 0;
}
