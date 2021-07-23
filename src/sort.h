#ifndef SORT_H
#define SORT_H

void carStrategy(FILE* carUnsortedFile, FILE* carSortedFile);
void lineStrategy(FILE* lineUnsortedFile, FILE* lineSortedFile);
void baseSort(void (*strategy)(FILE* unsortedFile, FILE* sortedFile));

#endif