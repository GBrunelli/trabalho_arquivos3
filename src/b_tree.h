#ifndef B_TREE_H
#define B_TREE_H

#include "project.h"

#define ORDER 5
#define REGISTERS_PER_PAGE 4

typedef enum _Result
{
    PROMOTION = -100,
    NO_PROMOTION,
    FOUND,
    NOT_FOUND,
    ERROR
} Result;

typedef struct _Index Index;
typedef struct _Register Register;


/**
 * Opens an index file, and set its status to inconsistent
*/
Index *openIndex(FILE *indexFile);

/**
 * Closes the index file, and set its status to consistent
*/
void closeIndex(Index *index);

/**
 * Retuns an index register with key C and value Pr
*/
Register *createRegister(int32_t C, int64_t Pr);

/**
 * Retuns the Pr of an specific register
*/
int64_t getPR(Register *reg);

/**
 * Frees all data associated with a specific register
*/ 
void freeRegister(Register *reg);
/**
 * Searches for a key in the index file,
 * if a register is founds, returns it 
*/
Result searchRegister(Index *index, int32_t key, Register **foundReg);

#endif