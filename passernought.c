#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 100
#define MAX_WORDS 256

typedef struct {
    char *letters;
    char *numbers;
    char *symbols;
    char **words;
    int wordCount;
    int passwordCount;
    int minLength;
    int maxLength;
    int useLeet;
    int prefixOption;
    int suffixOption;
    FILE *outputFile;
    int threadId;
    int totalThreads;
} ThreadData;

pthread_mutex_t progressMutex = PTHREAD_MUTEX_INITIALIZER;
int globalProgress = 0;

void printProgressBar(int current, int total) {
    int barWidth = 70;
    float progress = (float)current / total;
    int pos = barWidth * progress;
    printf("\r[");
    for (int i = 0; i < barWidth; i++) {
        if (i < pos) printf("#");
        else printf(" ");
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

int readWordListFromFile(char *filePath, char **words, int maxWordLength) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filePath);
        return 0;
    }

    int wordCount = 0;
    char buffer[256];
    while (fgets(buffer, 256, file) && wordCount < MAX_WORDS) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline

        if (strlen(buffer) <= maxWordLength) { // Exclude words that are too long
            words[wordCount] = strdup(buffer);
            wordCount++;
        }
    }
    fclose(file);
    return wordCount;
}

void calculateFileSize(int passwordCount, int avgLength) {
    size_t totalSize = passwordCount * (avgLength + 1) * 2; // *2 for both leeted and non-leeted
    double totalSizeMB = totalSize / (1024.0 * 1024.0);
    printf("Estimated file size for the password file: %.2f MB\n", totalSizeMB);
}

void *threadedPasswordGeneration(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char allChars[256];
    int index = 0;

    // Add letters, numbers, and symbols to pool
    for (int i = 0; data->letters[i] != '\0'; i++) allChars[index++] = data->letters[i];
    for (int i = 0; data->numbers[i] != '\0'; i++) allChars[index++] = data->numbers[i];
    for (int i = 0; data->symbols[i] != '\0'; i++) allChars[index++] = data->symbols[i];
    allChars[index] = '\0';

    // Generate passwords for assigned range
    int start = (data->passwordCount / data->totalThreads) * data->threadId;
    int end = (data->passwordCount / data->totalThreads) * (data->threadId + 1);
    if (data->threadId == data->totalThreads - 1) end = data->passwordCount;

    for (int i = start; i < end; i++) {
        int wordIndex = rand() % data->wordCount;
        char selectedWord[256];
        char leetedWord[256];
        strcpy(selectedWord, data->words[wordIndex]);
        strcpy(leetedWord, selectedWord);

        // Generate random password length within range
        int currentLength = data->minLength + (rand() % (data->maxLength - data->minLength + 1));

        // Apply leet speak if enabled
        if (data->useLeet) {
            leetSpeak(leetedWord);
        }

        // Generate passwords for both original and leeted versions
        char *wordVersions[2] = {selectedWord, data->useLeet ? leetedWord : NULL};
        int versions = data->useLeet ? 2 : 1;

        for (int v = 0; v < versions; v++) {
            if (wordVersions[v] == NULL) continue;

            int wordLen = strlen(wordVersions[v]);
            
            // Skip if word is too long for the current password length
            if (wordLen > currentLength) {
                continue;
            }

            int prefixLen = (data->prefixOption) ? rand() % (currentLength - wordLen + 1) : 0;
            int suffixLen = currentLength - wordLen - prefixLen;

            // Allocate dynamic buffer for password
            char *password = malloc(currentLength + 1);
            if (!password) continue;

            for (int k = 0; k < prefixLen; k++) password[k] = allChars[rand() % index];
            strncpy(&password[prefixLen], wordVersions[v], wordLen);
            for (int k = prefixLen + wordLen; k < currentLength; k++) password[k] = allChars[rand() % index];
            password[currentLength] = '\0';

            if (data->outputFile) fprintf(data->outputFile, "%s\n", password);
            
            free(password);
        }

        pthread_mutex_lock(&progressMutex);
        globalProgress++;
        printProgressBar(globalProgress, data->passwordCount);
        pthread_mutex_unlock(&progressMutex);
    }
    return NULL;
}

int main() {
    int minLength, maxLength, passwordCount, useLeet, prefixOption, suffixOption, totalThreads;
    char letters[27], numbers[11], symbols[33], filePath[256], outputFilePath[256];
    char *words[MAX_WORDS];
    int wordCount = 0;
    char saveToFile;

    printf("Enter the minimum length of the passwords (1-32): ");
    scanf("%d", &minLength);

    printf("Enter the maximum length of the passwords (1-32): ");
    scanf("%d", &maxLength);

    if (maxLength < minLength) {
        printf("Error: Maximum length cannot be less than minimum length.\n");
        return 1;
    }

    printf("Enter letters (a-z): ");
    scanf("%26s", letters);

    printf("Enter numbers (0-9): ");
    scanf("%10s", numbers);

    printf("Enter symbols: ");
    scanf("%32s", symbols);

    printf("Enter the path to the word list file: ");
    scanf("%255s", filePath);
    wordCount = readWordListFromFile(filePath, words, maxLength);
    if (wordCount == 0) {
        printf("No words suitable for the password length found. Exiting.\n");
        return 1;
    }

    printf("Enter the number of passwords to generate: ");
    scanf("%d", &passwordCount);

    int avgLength = (minLength + maxLength) / 2;
    calculateFileSize(passwordCount, avgLength);

    printf("Do you want to use leet speak? (1 for yes, 0 for no): ");
    scanf("%d", &useLeet);

    printf("Do you want to add a prefix? (1 for yes, 0 for no): ");
    scanf("%d", &prefixOption);

    printf("Do you want to add a suffix? (1 for yes, 0 for no): ");
    scanf("%d", &suffixOption);

    printf("Do you want to save the passwords to a file? (y/n): ");
    scanf(" %c", &saveToFile);

    FILE *outputFile = NULL;
    if (saveToFile == 'y') {
        printf("Enter the path to the output file: ");
        scanf("%255s", outputFilePath);
        outputFile = fopen(outputFilePath, "w");
        if (!outputFile) {
            printf("Error opening file: %s\n", outputFilePath);
            return 1;
        }
    }

    printf("Enter the number of threads to use (default %d): ", MAX_THREADS);
    scanf("%d", &totalThreads);

    pthread_t threads[totalThreads];
    ThreadData threadData[totalThreads];
    for (int i = 0; i < totalThreads; i++) {
        threadData[i] = (ThreadData){letters, numbers, symbols, words, wordCount, passwordCount, minLength, maxLength,
                                     useLeet, prefixOption, suffixOption, outputFile, i, totalThreads};
        pthread_create(&threads[i], NULL, threadedPasswordGeneration, &threadData[i]);
    }

    for (int i = 0; i < totalThreads; i++) pthread_join(threads[i], NULL);
    if (outputFile) fclose(outputFile);

    for (int i = 0; i < wordCount; i++) free(words[i]);

    return 0;
}