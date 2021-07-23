#include "join.h"
#include "b_tree.h"


void _openFiles(FILE **carBin, char *carFileName, FILE **lineBin, char *lineFileName)
{
    *carBin = fopen(carFileName, "rb");
    if (*carBin == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }

    *lineBin = fopen(lineFileName, "rb");
    if (*index == NULL)
    {
        fclose(*carBin);
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }

    // Checking integrity of each file
    char ic1 = '0', ic2 = '0';
    fread(&ic1, 1, 1, *lineBin);
    fread(&ic2, 1, 1, *carBin);
    if (ic1 == '0' || ic2 == '0')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(*carBin);
        fclose(*lineBin);
        exit(0);
    }

    fseek(*lineBin, 0, SEEK_SET);
    fseek(*carBin, 0, SEEK_SET);
}

int _openIndexFile(char* indexFileName, FILE** indexFile) {
    *indexFile = fopen(indexFileName, "rb");
    if (*indexFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    // Checking whether file is valid
    char c = '0';
    fread(&c, 1, 1, *indexFile);
    if (c == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(*indexFile); 
        return 0;
    }

    fseek(*indexFile, 0, SEEK_SET);
    return 1;
}

void baseJoin(int (*joinStrategy)(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile)) {
    // Opening files
    char carFileName[MAX_STRING_SIZE], lineFileName[MAX_STRING_SIZE];
    scanf("%s %s", carFileName, lineFileName);
    FILE *carFile = NULL, *lineFile = NULL;
    _openFiles(&carFile, carFileName, &lineFile, lineFileName);

    // Checking total amount of cars and lines
    LineHeader *lh = newLineHeader();
    CarHeader *ch = newCarHeader();
    updateLineHeader(lh, lineFile, BIN);
    getCarHeader(ch, carFile, BIN);
    int carN = getCarNRegisters(ch);
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
    if (!_openIndexFile(indexFileName, &indexFile))
        return -1;

    Index* idx = openIndex(indexFile);
    int nFound = 0;
    Register* reg = createRegister(0, 0);
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

    freeRegister(reg);
    closeIndex(idx);
    return nFound;
}

int sortedStrategy(Car* c, CarHeader *ch, int carN, FILE* carFile, Line *l, LineHeader *lh, int lineN, FILE* lineFile) {
    return 1;
}