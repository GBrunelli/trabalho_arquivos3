#include "line.h"
#include "utils.h"

// Base offset for each line
#define LINE_OFFSET 13

// Pre-determined size of each header string
#define DESCREVECODIGO_SIZE 15
#define DESCREVECARTAO_SIZE 13
#define DESCREVENOME_SIZE 13
#define DESCREVECOR_SIZE 24

struct _Line
{
    int8_t removido;
    int32_t tamanhoRegistro;
    int32_t codLinha;
    int8_t aceitaCartao;
    int32_t tamanhoNome;
    char nomeLinha[MAX_STRING_SIZE];
    int32_t tamanhoCor;
    char nomeCor[MAX_STRING_SIZE];
};

struct _LineHeader
{
    int8_t status;
    int64_t byteProxReg;
    int32_t nroRegistros;
    int32_t nroRegistrosRemovidos;
    char descreveCodigo[DESCREVECODIGO_SIZE + 1];
    char descreveCartao[DESCREVECARTAO_SIZE + 1];
    char descreveNome[DESCREVENOME_SIZE + 1];
    char descreveCor[DESCREVECOR_SIZE + 1];
};

LineHeader *newLineHeader()
{
    LineHeader *lh = calloc(1, sizeof(LineHeader));
    lh->status = '0';
    lh->nroRegistros = 0;
    lh->nroRegistrosRemovidos = 0;
    lh->byteProxReg = LINE_HEADER_OFFSET;

    lh->descreveCodigo[DESCREVECODIGO_SIZE] = '\0';
    lh->descreveCartao[DESCREVECARTAO_SIZE] = '\0';
    lh->descreveNome[DESCREVENOME_SIZE] = '\0';
    lh->descreveCor[DESCREVECOR_SIZE] = '\0';

    return lh;
}

// Updates a LineHeader using a valid binary file.
void _updateLineHeaderFromBin(LineHeader *lh, FILE *file)
{
    fseek(file, 0, SEEK_SET);

    fread(&lh->status, sizeof(lh->status), 1, file);
    fread(&lh->byteProxReg, sizeof(lh->byteProxReg), 1, file);
    fread(&lh->nroRegistros, sizeof(lh->nroRegistros), 1, file);
    fread(&lh->nroRegistrosRemovidos, sizeof(lh->nroRegistrosRemovidos), 1, file);
    fread(&lh->descreveCodigo, DESCREVECODIGO_SIZE, 1, file);
    fread(&lh->descreveCartao, DESCREVECARTAO_SIZE, 1, file);
    fread(&lh->descreveNome, DESCREVENOME_SIZE, 1, file);
    fread(&lh->descreveCor, DESCREVECOR_SIZE, 1, file);
}

void updateLineHeader(LineHeader *LineHeader, FILE *file, Source from)
{
    switch (from)
    {
    case BIN:
        _updateLineHeaderFromBin(LineHeader, file);
        break;
    default:
        break;
    }
}

int getNRegisters(LineHeader *lh)
{
    return lh->nroRegistros;
}

int getNRemovedRegisters(LineHeader *lh)
{
    return lh->nroRegistrosRemovidos;
}

int getLineTotalRegisters(FILE *bin)
{
    LineHeader *lh = newLineHeader();
    updateLineHeader(lh, bin, BIN);
    int nreg = getNRegisters(lh) + getNRemovedRegisters(lh);
    freeLineHeader(lh);

    return nreg;
}

bool checkLineFileIntegrity(FILE *bin)
{
    LineHeader *lh = newLineHeader();
    updateLineHeader(lh, bin, BIN);
    bool integrity = !checkLineHeaderIntegrity(lh);
    freeLineHeader(lh);

    return integrity;
}

bool checkLineHeaderIntegrity(LineHeader *lh)
{
    switch (lh->status)
    {
    case '1':
        return true;
    default:
        return false;
    }
}

// Writing new data from LH into FILE. Overwrites old header
void _overwriteLineHeaderToBin(LineHeader *lh, FILE *file)
{
    // set the file pointer to the correct position
    fseek(file, 0, SEEK_SET);

    // write the data on the file
    fwrite(&lh->status, sizeof(lh->status), 1, file);
    fwrite(&lh->byteProxReg, sizeof(lh->byteProxReg), 1, file);
    fwrite(&lh->nroRegistros, sizeof(lh->nroRegistros), 1, file);
    fwrite(&lh->nroRegistrosRemovidos, sizeof(lh->nroRegistrosRemovidos), 1, file);
    fwrite(&lh->descreveCodigo, DESCREVECODIGO_SIZE, 1, file);
    fwrite(&lh->descreveCartao, DESCREVECARTAO_SIZE, 1, file);
    fwrite(&lh->descreveNome, DESCREVENOME_SIZE, 1, file);
    fwrite(&lh->descreveCor, DESCREVECOR_SIZE, 1, file);
}

void overwriteLineHeader(LineHeader *lh, FILE *file, Source source)
{
    switch (source)
    {
    case BIN:
        _overwriteLineHeaderToBin(lh, file);
        break;

    default:
        break;
    }
}

void updateLineHeaderRegisterData(LineHeader *lh, int64_t byteProxReg, int32_t nroRegistros, int32_t nroRegistrosRemovidos)
{
    lh->byteProxReg = byteProxReg;
    lh->nroRegistros = nroRegistros;
    lh->nroRegistrosRemovidos = nroRegistrosRemovidos;
}

void setLineFileStatus(FILE *file, char c)
{
    if (c == REMOVED || c == NOT_REMOVED)
    {
        LineHeader *lh = newLineHeader();
        updateLineHeader(lh, file, BIN);
        lh->status = c;
        overwriteLineHeader(lh, file, BIN);
        freeLineHeader(lh);
    }
}

/* ## Basic Line functions ## */

Line *newLine()
{
    Line *l = calloc(1, sizeof(Line));
    l->removido = '1';
    l->tamanhoRegistro = LINE_OFFSET;
    return l;
}

// Update Line from BIN file, uses current offset by default
int64_t _updateLineFromBin(Line *l, FILE *file, int64_t customOffset)
{
    if (l == NULL)
        return -1;
    if (file == NULL)
        return -1;

    // if the pointer is pointing at the header, set the pointer for the first car in the file
    long long position = ftell(file);
    long long offset = position < LINE_HEADER_OFFSET ? LINE_HEADER_OFFSET : position;

    // Checking if custom offset was provided
    offset = customOffset != NO_OFFSET ? customOffset : offset;

    fseek(file, offset, SEEK_SET);

    fread(&l->removido, sizeof(l->removido), 1, file);
    fread(&l->tamanhoRegistro, sizeof(l->tamanhoRegistro), 1, file);
    fread(&l->codLinha, sizeof(l->codLinha), 1, file);
    fread(&l->aceitaCartao, sizeof(l->aceitaCartao), 1, file);
    fread(&l->tamanhoNome, sizeof(l->tamanhoNome), 1, file);
    fread(&l->nomeLinha, l->tamanhoNome, 1, file);
    fread(&l->tamanhoCor, sizeof(l->tamanhoCor), 1, file);
    fread(&l->nomeCor, l->tamanhoCor, 1, file);

    l->nomeLinha[l->tamanhoNome] = '\0';
    l->nomeCor[l->tamanhoCor] = '\0';

    return offset;
}

// Update Line from the Command Line (stdin). Consumes current stdin buffer
int64_t _updateLineFromCLI(Line *l)
{
    // Initializing zeroed char arrays and then reading from stdinput
    char codLinha[5] = {0};
    char aceitaCartao[5] = {0};
    char nomeLinha[MAX_STRING_SIZE] = {0};
    char corLinha[MAX_STRING_SIZE] = {0};

    // Scanning and removing possible quotations from each field
    scan_quote_string(codLinha);
    scan_quote_string(aceitaCartao);
    scan_quote_string(nomeLinha);
    scan_quote_string(corLinha);

    // Checking whether the Line should be considered logically removed
    if (codLinha[0] == '*')
    {
        l->removido = REMOVED;
        l->codLinha = atoi(&codLinha[1]);
    }
    else
    {
        l->removido = NOT_REMOVED;
        l->codLinha = atoi(codLinha);
    }

    // Checking whether nomeLinha and corLinha are identified as NULO
    if (isNULO(nomeLinha))
    {
        nomeLinha[0] = '\0';
    }
    if (isNULO(corLinha))
    {
        corLinha[0] = '\0';
    }

    l->aceitaCartao = aceitaCartao[0];
    l->tamanhoNome = strlen(nomeLinha);
    l->tamanhoCor = strlen(corLinha);
    l->tamanhoRegistro = LINE_OFFSET + l->tamanhoCor + l->tamanhoNome;
    strcpy(l->nomeLinha, nomeLinha);
    strcpy(l->nomeCor, corLinha);

    return 1;
}

int64_t updateLine(Line *l, FILE *file, Source from, int64_t customOffset)
{
    switch (from)
    {
    case BIN:
        return _updateLineFromBin(l, file, customOffset);
    case CLI:
        return _updateLineFromCLI(l);
    }
    return -1;
}

// Writes Line to end of binary file.
int64_t _writeLineToBin(Line *l, FILE *file)
{
    // gets the information from the header and update it
    LineHeader *lh = newLineHeader();
    updateLineHeader(lh, file, BIN);

    int byteOffset = lh->byteProxReg;
    lh->byteProxReg += l->tamanhoRegistro + sizeof(l->removido) + sizeof(l->tamanhoRegistro);

    if (l->removido == NOT_REMOVED)
        lh->nroRegistros++;
    else
        lh->nroRegistrosRemovidos++;

    overwriteLineHeader(lh, file, BIN);
    freeLineHeader(lh);

    fseek(file, byteOffset, SEEK_SET);
    fwrite(&l->removido, sizeof(l->removido), 1, file);
    fwrite(&l->tamanhoRegistro, sizeof(l->tamanhoRegistro), 1, file);
    fwrite(&l->codLinha, sizeof(l->codLinha), 1, file);
    fwrite(&l->aceitaCartao, sizeof(l->aceitaCartao), 1, file);
    fwrite(&l->tamanhoNome, sizeof(l->tamanhoNome), 1, file);
    fwrite(&l->nomeLinha, l->tamanhoNome, 1, file);
    fwrite(&l->tamanhoCor, sizeof(l->tamanhoCor), 1, file);
    fwrite(&l->nomeCor, l->tamanhoCor, 1, file);

    return byteOffset;
}

int64_t writeLine(Line *l, FILE *file, Source from)
{
    switch (from)
    {
    case BIN:
        return _writeLineToBin(l, file);
        break;

    default:
        break;
    }

    return -1;
}

// Matches each cardType with its text counterpart
char *_checkCardType(char cardType)
{
    switch (cardType)
    {
    case 'S':
        return "PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR";
    case 'N':
        return "PAGAMENTO EM CARTAO E DINHEIRO";
    case 'F':
        return "PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA";
    default:
        return "ERRO";
    }
}

int32_t getLineCodLinha(Line *l)
{
    return l->codLinha;
}

bool lineLogicallyRemoved(Line *l)
{
    if (l->removido == REMOVED)
        return true;
    return false;
}

// Prints Car. Checks if Car is logically removed and also deals with nulls.
FuncStatus printLine(Line *l, LineHeader *lh)
{
    if (l->removido == REMOVED)
        return LOGICALLY_REMOVED;

    printf("%s: %d\n", lh->descreveCodigo, l->codLinha);
    printf("%s: %s\n", lh->descreveNome, l->tamanhoNome > 0 ? l->nomeLinha : NULL_MESSAGE);
    printf("%s: %s\n", lh->descreveCor, l->tamanhoCor > 0 ? l->nomeCor : NULL_MESSAGE);
    printf("%s: %s\n\n", lh->descreveCartao, _checkCardType(l->aceitaCartao));
    return OK;
}

void freeLine(Line *l)
{
    free(l);
}

void freeLineHeader(LineHeader *lh)
{
    free(lh);
}
