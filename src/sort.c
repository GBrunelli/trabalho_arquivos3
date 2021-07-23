#include "project.h"
#include "utils.h"
#include "line.h"
#include "car.h"

int _compareCars(const void *c1, const void *c2) {
    Car* elem1 = *(Car**)c1;    
    Car* elem2 = *(Car**)c2;
    int res = getCarCodLinha(elem1) - getCarCodLinha(elem2);
    printf("elem1: %d - elem2: %d = %d\n", getCarCodLinha(elem1), getCarCodLinha(elem2), res);
    return res;
}

int _compareLines(const void *l1, const void *l2) {
    Line* elem1 = *(Line**)l1;    
    Line* elem2 = *(Line**)l2;
    int res = getLineCodLinha(elem1) - getLineCodLinha(elem2);
    return res;
}

void sortCars(void) {
    // Opening files
    char carUnsortedFilename[MAX_STRING_SIZE], carSortedFilename[MAX_STRING_SIZE];
    scanf("%s %s", carUnsortedFilename, carSortedFilename);
    FILE *carUnsortedFile = NULL, *carSortedFile = NULL;
    openFiles(&carUnsortedFile, carUnsortedFilename, "rb", &carSortedFile, carSortedFilename, "w+b");
    

    // Checking total amount of lines
    CarHeader *ch = newCarHeader();
    getCarHeader(ch, carUnsortedFile, BIN);
    int carN = getCarNRegisters(ch);

    // Reading all cars and preparing array for sorting
    Car **cars = calloc(carN, sizeof(Car*));
    int carsRead = 0;
    while (carsRead < carN) {
        cars[carsRead] = newCar();
        readCar(cars[carsRead], carUnsortedFile, BIN, NO_OFFSET);

        if (!carLogicallyRemoved(cars[carsRead]))
            carsRead++;
        else
            freeCar(cars[carsRead]);
    }
    fclose(carUnsortedFile);

    // Sorting cars array
    qsort(cars, carN, sizeof(Car *), _compareCars);

    // Reinitializing File Header for new file
    updateCarHeaderRegisterData(ch, STRUCT_CAR_HEADER_SIZE, 0, 0);
    writeCarHeader(ch, carSortedFile, BIN);
    freeCarHeader(ch);
    setCarFileStatus(carSortedFile, '0');

    // Writing sorted data into sortedFile
    for (int i = 0; i < carN; i++) {
        writeCar(cars[i], carSortedFile, BIN);
    }

    // Overwriting File Header with new information then closing file
    setCarFileStatus(carSortedFile, '1');
    fclose(carSortedFile);

    // Cleaning up data
    for (int i = 0; i < carN; i++)
        freeCar(cars[i]);
    free(cars);
    
    binarioNaTela(carSortedFilename);
}

void sortLines(void) {
    // Opening files
    char lineUnsortedFilename[MAX_STRING_SIZE], lineSortedFilename[MAX_STRING_SIZE];
    scanf("%s %s", lineUnsortedFilename, lineSortedFilename);
    FILE *lineUnsortedFile = NULL, *lineSortedFile = NULL;
    openFiles(&lineUnsortedFile, lineUnsortedFilename, "rb", &lineSortedFile, lineSortedFilename, "w+b");
    

    // Checking total amount of lines
    LineHeader *lh = newLineHeader();
    updateLineHeader(lh, lineUnsortedFile, BIN);
    int lineN = getNRegisters(lh);

    // Reading all lines and preparing array for sorting
    Line **lines = calloc(lineN, sizeof(Line*));
    int linesRead = 0;
    while (linesRead < lineN) {
        lines[linesRead] = newLine();
        updateLine(lines[linesRead], lineUnsortedFile, BIN, NO_OFFSET);

        if (lineLogicallyRemoved(lines[linesRead])) 
            freeLine(lines[linesRead]);
        else 
            linesRead++;        
    }
    fclose(lineUnsortedFile);

    
    // Sorting lines array
    qsort(lines, lineN, sizeof(Line *), _compareLines);

    // Reinitializing File Header for new file
    updateLineHeaderRegisterData(lh, LINE_HEADER_OFFSET, 0, 0);
    overwriteLineHeader(lh, lineSortedFile, BIN);
    freeLineHeader(lh);
    setLineFileStatus(lineSortedFile, '0');

    // Writing sorted data into sortedFile
    for (int i = 0; i < lineN; i++)
        writeLine(lines[i], lineSortedFile, BIN);

    // Overwriting File Header with new information then closing file
    setLineFileStatus(lineSortedFile, '1');
    fclose(lineSortedFile);

    // Cleaning up data
    for (int i = 0; i < lineN; i++)
        freeLine(lines[i]);
    free(lines);
    
    binarioNaTela(lineSortedFilename);
}