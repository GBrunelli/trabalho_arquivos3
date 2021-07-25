#include "join.h"
#include "b_tree.h"
#include "utils.h"
#include "sort.h"

void baseJoin(int (*joinStrategy)(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile)) {
    // Opening files
    char carFileName[MAX_STRING_SIZE], lineFileName[MAX_STRING_SIZE];
    scanf("%s %s", carFileName, lineFileName);
    FILE *carFile = NULL, *lineFile = NULL;
    openFiles(&carFile, carFileName, "r+b", &lineFile, lineFileName, "r+b");

    // Checking total amount of cars and lines
    LineHeader *lh = newLineHeader();
    CarHeader *ch = newCarHeader();
    updateLineHeader(lh, lineFile, BIN);
    getCarHeader(ch, carFile, BIN);
    int carN = getCarNRegisters(ch) + getCarNRemovedRegisters(ch);
    int lineN = getNRegisters(lh) + getNRemovedRegisters(lh);

    // Initializing joining containers
    int nFound = 0;
    Line *l = newLine();
    Car *c = newCar();

    // Executing join strategy
    nFound = joinStrategy(c, ch, carN, carFile, l, lh, lineN, lineFile);

    // Finishing program
    freeLine(l);
    freeCar(c);
    freeCarHeader(ch);
    freeLineHeader(lh);
    fclose(carFile);
    fclose(lineFile);

    if (!nFound)
        printf("Registro inexistente.\n");
}

int nestedStrategy(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile) {
    int nFound = 0;
    for (int i = 0; i < carN; i++) {
        readCar(c, carFile, BIN, NO_OFFSET);
        for (int j = 0; j < lineN; j++) {
            updateLine(l, lineFile, BIN, NO_OFFSET);
            if (!carLogicallyRemoved(c) && !lineLogicallyRemoved(l) && getCarCodLinha(c) == getLineCodLinha(l)) {
                printCar(c, ch);
                printLine(l, lh);
                nFound++;
                break;
            }
        }
        fseek(lineFile, 0, SEEK_SET);
    }
    return nFound;
}


int indexedStrategy(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile) {
    // Opening extra index file for this strategy
    char indexFileName[MAX_STRING_SIZE];
    scanf("%*s %*s %s", indexFileName);
    FILE* indexFile = NULL;
    if (!openIndexFile(indexFileName, &indexFile))
        return -1;

    Index* idx = openIndex(indexFile);
    Register* reg;

    int nFound = 0;
    for (int i = 0; i < carN; i++) {
        readCar(c, carFile, BIN, NO_OFFSET);
        if (searchRegister(idx, getCarCodLinha(c), &reg) == FOUND) {
            updateLine(l, lineFile, BIN, getPR(reg));
            if (!lineLogicallyRemoved(l) && !carLogicallyRemoved(c)) {
                printCar(c, ch);
                printLine(l, lh);
                nFound++;
            }
        }
    }
    closeIndex(idx);
    return nFound;
}

void ___preSortFiles(FILE* carFile, CarHeader* ch, int carN, FILE* lineFile, LineHeader* lh, int lineN) {
    // Getting all registers from files
    Line **lines = getLines(lineFile, lh);
    Car **cars = getCars(carFile, ch);

    // Sorting
    qsort(cars, carN, sizeof(Car *), compareCars);
    qsort(lines, lineN, sizeof(Line *), compareLines);

    // Writing registers
    writeCars(carFile, ch, cars);
    writeLines(lineFile, lh, lines);

    // Cleaning up data
    for (int i = 0; i < lineN; i++)
        freeLine(lines[i]);
    for (int i = 0; i < carN; i++)
        freeCar(cars[i]);
    free(cars);
    free(lines);
}

int sortedStrategy(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile) {
    // Sorting both files and then saving the result in-place 
    carStrategy(carFile, carFile);
    lineStrategy(lineFile, lineFile);

    // Correcting amount of registers in each file
    carN -= getCarNRemovedRegisters(ch);
    lineN -= getNRemovedRegisters(lh);

    // Initializing memory for each register container
    if (lineN > 0) {
        updateLine(l, lineFile, BIN, NO_OFFSET);
        lineN--;
    }
    if (carN > 0) {
        readCar(c, carFile, BIN, NO_OFFSET);
        carN--;
    }

    // Starting merge loop
    int nFound = 0;
    int curr_l = getLineCodLinha(l), curr_c = getCarCodLinha(c);
    do {
        // Checking wheter both sides match
        if (curr_l == curr_c) {
            // Printing results to stdin
            printCar(c, ch);
            printLine(l, lh);
            nFound++;
            
            // If there are still some cars, go to next one
            if (carN > 0) {
                readCar(c, carFile, BIN, NO_OFFSET);
                carN--;
            }
        }
        // Read a new line If: 
        // lineN still has valid entries and lines' side has a lower codLinha
        // OR if there are no more cars, while there are still some lines
        else if ((lineN > 0 && (curr_l < curr_c)) || (!carN && lineN)) {
            updateLine(l, lineFile, BIN, NO_OFFSET);
            lineN--;
        } 
        // If no other condition was met and cars are not empty, read a car
        else if (carN > 0) {
            readCar(c, carFile, BIN, NO_OFFSET);
            carN--;            
        }

        curr_l = getLineCodLinha(l);
        curr_c = getCarCodLinha(c);
    } while (carN > 0 || lineN > 0);

    // Checking last one on memory
    if (curr_l == curr_c) {
        printCar(c, ch);
        printLine(l, lh);
        nFound++;
    }

    return nFound;
}
