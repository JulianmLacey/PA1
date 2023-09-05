#include "header.h"

void summary_update(FitbitDataSummary* s, double cals, double dist, uint flrs, uint hr, uint steps) {
    s->total_calories += cals;
    s->total_distance += dist;
    s->total_floors += flrs;
    s->total_heartRate += hr;
    s->total_steps += steps;
    s->max_steps = MAX(s->max_steps, steps);
}

void fprintData(FitbitDataSummary* summary, FILE* results, char** invalid_entries) {
    fprintf(results, "Total Calories,Total Distance,Total Floors,Total Steps,Avg Heartrate,Max Steps,Sleep\n");
    fprintf(results, "%lf,%lf,%u,%u,%lf,%u,%s,%s\n", summary->total_calories, summary->total_distance, summary->total_floors, summary->total_steps, (double)summary->total_heartRate / summary->totalEntries, summary->max_steps, summary->bestSleep->beg, summary->bestSleep->end);
    fprintf(results, "INVALID ENTRIES:\n%s\n", *invalid_entries);
}
void printData(FitbitDataSummary* summary) {
    printf("FIT SUMMARY:\nTotal Entries: %u\nHeartRate: %u\nCalories: %lf\nDistance: %lf\nFloors: %u\nSteps: %u\nMAX: %u\n\n", summary->totalEntries, summary->total_heartRate, summary->total_calories, summary->total_distance, summary->total_floors, summary->total_steps, summary->max_steps);
    printf("Sleep Length: |%u|\nStart: %s \nEnd: %s \n", summary->bestSleep->length, summary->bestSleep->beg, summary->bestSleep->end);
}
void getData(char** invEntries, FitbitData** fitDataArr, FitbitDataSummary* totalsSummary, FILE* file) {
    // vars
    char* line = NULL;
    size_t len = 0;
    char target[100] = "";
    regex_t exp;
    int res;
    BestSleep* newSleep = &(BestSleep){" ", " ", 0};

    *invEntries = malloc(sizeof(char) * 3360);  // allocate memory for invalid entries
    strcpy(*invEntries, "");                    // initialize

    getline(&line, &len, file);             // get first line for target user
    sscanf(line, "Target: ,%5s,", target);  // get target user
    getline(line, &len, file);              // Throw Away Headers

    strcat(target, ",[0-9]?[0-9](:[0-9]{2}){2}(,([0-9]*[.])?[0-9]+){2}(,[0-9]+){3,4}");  // build regex expression
    res = regcomp(&exp, target, REG_EXTENDED);                                           // compile regex

    while (getline(&line, &len, file) != -1) {
        if (regexec(&exp, line, 0, NULL, 0) == 0) {                                                                                                          // If valid Entry
            totalsSummary->totalEntries++;                                                                                                                   // Increase Total Entries
            FitbitData* d = (FitbitData*)malloc(sizeof(FitbitData));                                                                                         // Allocate New Struct
            sscanf(line, "%5s,%8s,%lf,%lf,%u,%u,%u,%u", d->patient, d->minute, &d->calories, &d->distance, &d->floors, &d->heartRate, &d->steps, &d->flag);  // Parse Entry

            UPDATE(totalsSummary, sUpdate, d->calories, d->distance, d->floors, d->heartRate, d->steps);  // Update totals
            fitDataArr[totalsSummary->totalEntries] = d;                                                  // add new entry to array

            if ((newSleep->length && (d->flag > 1)) && (newSleep->length += d->flag)) {  // sleeping - newsleep.length & d.flag - increase sleep length
            } else if (!newSleep->length && (d->flag > 1)) {                             // start sleep - !newsleep.length & d.flag
                newSleep = &(BestSleep){" ", " ", d->flag};                              // Create new Best Sleep
                strcpy(newSleep->beg, d->minute);                                        // Set Start Time
            } else if (newSleep->length && (d->flag <= 1)) {                             // end sleep - newsleep.length & !d.flag
                strcpy(newSleep->end, d->minute);                                        // Set End Time
                if (newSleep->length >= totalsSummary->bestSleep->length) {              // If new sleep is better than previous best sleep
                    memcpy(totalsSummary->bestSleep, newSleep, sizeof(BestSleep));       // set totalsSummary.sleep to newsleep
                    newSleep->length = 0;                                                // reset newsleep.length
                }
            } else if (!newSleep->length && (d->flag <= 1)) {  // awake - !newsleep.length & !d.flag - do nothing
            } else {                                           // you fucked something up
                printf("ERROR: INVALID SLEEP STATE\n");
            }
            free(d);  // Free struct cuz we dont do anything wit it
        } else {      // If not valid entry add to list of invalid
            strcat(*invEntries, line);
        }
    }
}