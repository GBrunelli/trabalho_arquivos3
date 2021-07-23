#include "project.h"
#include "utils.h"
#include "line.h"
#include "car.h"

void carStrategy(FILE* carUnsortedFile, FILE* carSortedFile) {
    // Checking total amount of lines
    CarHeader *ch = newCarHeader();
    getCarHeader(ch, carUnsortedFile, BIN);
    int carN = getCarNRegisters(ch);

    // Getting all cars from file
    Car **cars = getCars(carUnsortedFile, ch);

    // Sorting cars
    qsort(cars, carN, sizeof(Car *), compareCars);

    // Writing cars to sorted file
    writeCars(carSortedFile, ch, cars);

    // Cleaning up data
    for (int i = 0; i < carN; i++)
        freeCar(cars[i]);
    free(cars);
}

void lineStrategy(FILE* lineUnsortedFile, FILE* lineSortedFile) {
    // Checking total amount of lines
    LineHeader *lh = newLineHeader();
    updateLineHeader(lh, lineUnsortedFile, BIN);
    int lineN = getNRegisters(lh);

    // Getting all lines from file
    Line **lines = getLines(lineUnsortedFile, lh);

    // Sorting lines
    qsort(lines, lineN, sizeof(Line *), compareLines);

    // Writing lines to sorted file
    writeLines(lineSortedFile, lh, lines);

    // Cleaning up data
    for (int i = 0; i < lineN; i++)
        freeLine(lines[i]);
    free(lines);
}

void baseSort(void (*strategy)(FILE* unsortedFile, FILE* sortedFile)) {
    // Opening files
    char unsortedFilename[MAX_STRING_SIZE], sortedFilename[MAX_STRING_SIZE];
    scanf("%s %s", unsortedFilename, sortedFilename);
    FILE *unsortedFile = NULL, *sortedFile = NULL;
    openFiles(&unsortedFile, unsortedFilename, "rb", &sortedFile, sortedFilename, "w+b");

    // Pointer function executes specific type of strategy, as provided above    
    strategy(unsortedFile, sortedFile);

    // Closing files
    fclose(unsortedFile);
    fclose(sortedFile);
    
    // Results for run.codes
    binarioNaTela(sortedFilename);
}