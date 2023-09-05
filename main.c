/*******************************************************************************************
 * Programmer: Julian Lacey
 * Class: CptS 122, Fall 2023; Lab Section ?
 * Programming Assignment: PA1 - Analyzing Fitbit Data
 * Date: September 4rd, 2023
 * Description: This program parses a csv file of dailey fitbit data and outputs the daily totals
 *               of the data.
 * Note: No error handling is done in this program cuz i was way too lazy to do that shit
 *******************************************************************************************/

#include "header.h"

/*******************************************************************************************
 * Function: summary_update
 * Programmer: Julian Lacey
 * Class: CptS 122, Fall 2023; Lab Section ?
 * Date: September 3rd, 2023
 * Inputs:
 *   FitbitDataSummary* s - Pointer to FitbitDataSummary
 *   double cals - Current Entry Calories
 *   double dist - Current Entry Distance
 *   uint flrs   - Current Entry Floors
 *   uint hr     - Current Entry Heart Rate
 *   uint steps  - Current Entry Steps
 * Outputs: returns pointer to summer data
 * Description: Updates the summary data with the current entry data
 *******************************************************************************************/
FitbitDataSummary* summary_update(FitbitDataSummary* s, double cals, double dist, uint flrs, uint hr, uint steps) {
    s->total_calories += cals;
    s->total_distance += dist;
    s->total_floors += flrs;
    s->total_heartRate += hr;
    s->total_steps += steps;
    s->max_steps = MAX(s->max_steps, steps);
    return s;
}

/*******************************************************************************************
 * Function: fprintData
 * Programmer: Julian Lacey
 * Class: CptS 122, Fall 2023; Lab Section ?
 * Date: September 4rd, 2023
 * Inputs:
 *  FitbitDataSummary* s - Pointer to FitbitDataSummary
 *  FILE* results        - Pointer to Results File
 *  char** invalid_entries - Pointer to invalid entries string
 * Outputs: Returns pointer to summer data
 * Description: Prints the summary data to the results file
 *******************************************************************************************/
FitbitDataSummary* fprintData(FitbitDataSummary* s, FILE* results, char** invalid_entries) {
    fprintf(results, "Total Calories,Total Distance,Total Floors,Total Steps,Avg Heartrate,Max Steps,Sleep\n");
    fprintf(results, "%lf,%lf,%u,%u,%lf,%u,%s,%s\n", s->total_calories, s->total_distance, s->total_floors, s->total_steps, (double)s->total_heartRate / s->totalEntries, s->max_steps, s->bestSleep->beg, s->bestSleep->end);
    fprintf(results, "INVALID ENTRIES:\n%s\n", *invalid_entries);
    return s;
}

/*******************************************************************************************
 * Function: getData
 * Programmer: Julian Lacey
 * Class: CptS 122, Fall 2023; Lab Section ?
 * Date: September 4rd, 2023
 * Inputs:
 *     char** invEntries - Pointer to invalid entries string
 *     FitbitData** fitDataArr - Pointer to array of FitbitData pointers
 *     FitbitDataSummary* totalsSummary - Pointer to FitbitDataSummary
 *     FILE* file - Pointer to InputData
 * Outputs: Returns pointer to summer data
 * Description: Calculates the totals while parsing the data
 *******************************************************************************************/
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
    getline(&line, &len, file);             // Throw Away Headers

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

int main(void) {
    char* invalidEntries;
    FitbitData** fitData = malloc(sizeof(FitbitData*) * MAXENTRIES);
    FILE* file = fopen("FitbitData.csv", "r");
    FILE* results = fopen("Results.csv", "w");

    FitbitDataSummary summary = (FitbitDataSummary){0, 0, 0, 0, 0, 0, 0, &(BestSleep){" ", " ", 0}};
    summary.sUpdate = &summary_update;
    getData(&invalidEntries, fitData, &summary, file);

    fprintData(&summary, results, &invalidEntries);
    fprintData(&summary, stdout, &invalidEntries);

    free(invalidEntries);
    free(fitData);
    fclose(results);
    fclose(file);

    return 0;
}
