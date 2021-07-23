#ifndef UTIL_H
#define UTIL_H

// Message used when printing fields that are currently Null
#define NULL_MESSAGE "campo com valor nulo"

// Maximum field size for each struct.
#define MAX_STRING_SIZE 100

// Logically removed
#define REMOVED '0'
// Not logically removed
#define NOT_REMOVED '1'

// Inconsistent file
#define STATUS_INCONSISTENT '0'
// Consistent file
#define STATUS_CONSISTENT '1'

// When you don't want a specific offset while updating
#define NO_OFFSET -1

// Possible file sources.
typedef enum _Source
{
    CLI,
    BIN
} Source;

// Possible results from functions.
// Can be: Errors, Incomplete Results, OK, etc...
typedef enum _FuncStatus
{
    UNKNOWN_ERR,
    OK,
    EOF_OR_MALFORMED,
    NO_FILE_FOUND,
    LOGICALLY_REMOVED
} FuncStatus;

// Logic for opening both files used on most routines
void openFiles(FILE **carBin, char *carFileName, char* carOpenOptions, FILE **lineBin, char *lineFileName, char* lineOpenOptions);

// Logic for opening the indexFile, used on the indexed join strategy
int openIndexFile(char* indexFileName, FILE** indexFile);

// Get the month, given its number
// e.g. 1 = January, 12 = December...
void getMonthName(char *monthName, int month);

// Transform a date of format "YYYY-MM-DD" to "_DAY de _MONTH_NAME_ de _YEAR_".
// Example: "2010-05-21" -> "21 de maio de 2021".
void tranformDate(char *date);

// Remove "Quotation Marks" from a string.
void removeQuotations(char *str);

// Shifts a string to left by 1 byte.
void leftShift(char *string, int len);

// Checks if string is equal to "NULO"
int isNULO(char *string);

// Fills string with '@' characters up to len
void fillWithGarbage(char *string, int len);

// Function given by the professor to convert prefixo into a hash that can be used by B-tree.
int convertePrefixo(char *str);

// Function given by the professor that calculates a value based on files bytes. Used by run.codes.
void binarioNaTela(char *nomeArquivoBinario);

// Function given by the professor that scans a "quoted string" like this.
// Removes the quotes from the string. Also checks for NULO
void scan_quote_string(char *str);

#endif