#pragma once
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MATCHES 1
#define UPDATE(a, b, ...) a->b(a, __VA_ARGS__)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define SWAP(a, b, t) \
    do {              \
        t SWAP = a;   \
        a = b;        \
        b = SWAP;     \
    } while (0)
#define MAXENTRIES 1446

typedef unsigned int uint;  // Typedef for unsigned int cuz im way to lazy to type that out every time

typedef enum sleep {
    NONE,
    ASLEEP,
    AWAKE,
    REALLYAWAKE
} Sleep;

typedef struct fitbit {
    char patient[10];
    char minute[9];
    double calories;
    double distance;
    uint floors;
    uint heartRate;
    uint steps;
    Sleep sleepLevel;
    int flag;
} FitbitData;

typedef struct best {
    char beg[10];
    char end[10];
    uint length;
} BestSleep;

typedef struct dataSummary {
    uint totalEntries;
    double total_calories;
    double total_distance;
    uint total_floors;
    uint total_heartRate;
    uint max_steps;
    uint total_steps;
    BestSleep* bestSleep;
    void (*sUpdate)();
} FitbitDataSummary;