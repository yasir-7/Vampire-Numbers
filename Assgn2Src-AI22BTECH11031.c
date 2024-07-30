#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

int count = 0;

// Structure to hold information for each thread
struct ThreadInfo {
    int id;
    int start;
    int end;
    FILE *logFile;
};

// Function to compare two characters
int compare(const void *a, const void *b) {
    return *(char *)a - *(char *)b;
}

// Function to calculate the number of digits in a given number
int countDigits(int num) {
    int count = 0;
    while (num != 0) {
        num /= 10;
        count++;
    }
    return count;
}

// Function to check if a given number is a vampire number
bool isVampireNumber(int num, int a, int b) {
    if(a*b == num && (a%10 != 0 || b%10 != 0)){
        return 1;
    }
    else
    {
        return 0;
    }
    
}

// Recursive function to generate permutations of digits
int generatePermutations(int num, int *digits, int pos, bool *used, int *a, int *b, int id, FILE *logFile) {
    if (pos == countDigits(num)) {
        // Check if the current permutation forms a vampire number
        if (*a <= *b && isVampireNumber(num, *a, *b)) {
            fprintf(logFile, "%d: Found by Thread %d\n", num, id);
            count++;
            return 1;
        }
    } else {
        for (int i = 0; i < countDigits(num); i++) {
            if (!used[i]) {
                used[i] = true;
                if (pos < countDigits(num) / 2) {
                    *a = *a * 10 + digits[i];
                } else {
                    *b = *b * 10 + digits[i];
                }

                if (generatePermutations(num, digits, pos + 1, used, a, b, id, logFile)){
                    break;
                    return 1;
                };

                if (pos < countDigits(num) / 2) {
                    *a = (*a - digits[i]) / 10;
                } else {
                    *b = (*b - digits[i]) / 10;
                }
                used[i] = false;
            }
        }
    }
    return 0;
}

// Function to check if a number is a vampire number
void checkVampireNumber(int num, FILE *logFile, int id) {
    int numDigits = countDigits(num);
    if (numDigits % 2 == 1) {
        return;
    }

    int *digits = (int *)malloc(numDigits * sizeof(int));
    bool *used = (bool *)malloc(numDigits * sizeof(bool));
    int a = 0, b = 0;

    // Extract individual digits
    int temp = num;
    for (int i = numDigits - 1; i >= 0; i--) {
        digits[i] = temp % 10;
        temp /= 10;
        used[i] = false;
    }
    generatePermutations(num, digits, 0, used, &a, &b, id, logFile);

    free(digits);
    free(used);
}

// Function to be executed by each thread
void *threadFunction(void *arg) {
    struct ThreadInfo *threadInfo = (struct ThreadInfo *)arg;

    // Check each number in the assigned range
    for (int num = threadInfo->start; num <= threadInfo->end; ++num) {
        checkVampireNumber(num, threadInfo->logFile, threadInfo->id);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    clock_t start = clock();

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N> <M>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]); // Range of numbers
    int M = atoi(argv[2]); // Number of threads

    // Open the log file
    FILE *logFile = fopen("OutFile.txt", "w");

    if (logFile == NULL) {
        perror("Error opening log file");
        return EXIT_FAILURE;
    }

    pthread_t threads[M];
    struct ThreadInfo threadInfo[M];

    // Divide the range of numbers among threads
    int numbersPerThread = N / M;

    // Create threads and assign ranges
    for (int i = 0; i < M; ++i) {
        threadInfo[i].id = i + 1;
        threadInfo[i].start = i * numbersPerThread + 1;
        threadInfo[i].end = (i + 1) * numbersPerThread;
        threadInfo[i].logFile = logFile; // Pass the file pointer to each thread

        pthread_create(&threads[i], NULL, threadFunction, (void *)&threadInfo[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < M; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Inform the main thread about the count
    fprintf(logFile, "Total vampire numbers: %d\n", count);

    // Close the log file
    fclose(logFile);

    clock_t end = clock();

    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    FILE *NTime = fopen("Time_vs_N2.csv", "a+");
    fprintf(NTime, "%d,%f\n", N, cpu_time_used);
    fclose(NTime);

    return EXIT_SUCCESS;
}
