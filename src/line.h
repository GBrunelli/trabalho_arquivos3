#ifndef LINE_H
#define LINE_H

#include "project.h"
#include "utils.h"

typedef struct _LineHeader LineHeader;

typedef struct _Line Line;


// Offset to pass through header
#define LINE_HEADER_OFFSET 82

/* ## Functions to deal with line headers ## */

// Create a new line header
LineHeader *newLineHeader();

int compareLines(const void *l1, const void *l2);
Line** getLines(FILE* lineUnsortedFile, LineHeader *lh);
void writeLines(FILE* lineSortedFile, LineHeader* lh, Line** lines);

// Get all Header information from a specific source file.
// Currently supported sources: BIN, CSV
void updateLineHeader(LineHeader *lh, FILE *file, Source from);

// Update data associated with registes inside LineHeader
void updateLineHeaderRegisterData(LineHeader *ch, int64_t byteProxReg, int32_t nroRegistros, int32_t nroRegistrosRemovidos);

// Check if a binary line file is corrupted
bool checkLineFileIntegrity(FILE *bin);

// Check whether LineHeader status is '1' or '0'
bool checkLineHeaderIntegrity(LineHeader *lh);

// Overwrite old LineHeader from file with a newer, currently in-memory one.
void overwriteLineHeader(LineHeader *lh, FILE *file, Source source);

// Returns the total amount of registers. Logically Removed + Non Logically Removed
int getLineTotalRegisters(FILE *bin);

// Returns how many non logically removed registers there are.
int getNRegisters(LineHeader *lh);

// Returns how many logically removed registers there are
int getNRemovedRegisters(LineHeader *lh);

// Free all data associated with a line register
void freeLineHeader(LineHeader *lh);

// Set a binary line file as INCONSISTENT or CONSISTENT, depending on supplied C
void setLineFileStatus(FILE *file, char c);

/* ## Basic line functions ## */

// Creates a new reusable Line.
Line *newLine();

// Free all memory associated with a line
void freeLine(Line *l);

// Prints Line. Also checks if Line is logically removed and deals with nulls.
FuncStatus printLine(Line *l, LineHeader *lh);

// Updates a Line with data from a specific source.
// Currently supported sources: BIN, CLI, CSV
// If updating from CLI, file should be NULL.
int64_t updateLine(Line *l, FILE *file, Source from, int64_t pre_offset);

// Writes a Line to a specific source
// Currently only supports BIN files.
int64_t writeLine(Line *l, FILE *file, Source from);

// Returns the codLinha from current line
int32_t getLineCodLinha(Line *l);

// Checks whether a line is logically removed
bool lineLogicallyRemoved(Line *l);

// Based on an specific searched value, check whether the current line matches
bool checkIfLineMatches(Line *l, char *index);

#endif