#include "car.h"
#include "utils.h"


struct _CarHeader
{
    char status;
    int64_t byteProxReg;
    int32_t nroRegistros;
    int32_t nroRegistrosRemovidos;
    char descrevePrefixo[18];
    char descreveData[35];
    char descreveLugares[42];
    char descreveLinha[26];
    char descreveModelo[17];
    char descreveCategoria[20];
};

typedef enum _CarField
{
    PREFIXO,
    DATA,
    QTD_LUGARES,
    COD_LINHA_CAR,
    MODELO,
    CATEGORIA
} CarField;

struct _Car
{
    char removido;
    int32_t tamanhoRegistro;
    char prefixo[5];
    char data[10];
    int32_t quantidadeLugares;
    int32_t codLinha;
    int32_t tamanhoModelo;
    char modelo[MAX_STRING_SIZE];
    int32_t tamanhoCategoria;
    char categoria[MAX_STRING_SIZE];
};

/* ## Functions to deal with Car headers ## */

int getCarNRegisters(CarHeader *ch)
{
    return ch->nroRegistros;
}

int getCarNRemovedRegisters(CarHeader *ch)
{
    return ch->nroRegistrosRemovidos;
}

CarHeader *newCarHeader()
{
    CarHeader *carHeader = calloc(1, sizeof(CarHeader));
    carHeader->byteProxReg = STRUCT_CAR_HEADER_SIZE;
    return carHeader;
}

void updateCarHeaderRegisterData(CarHeader *ch, int64_t byteProxReg, int32_t nroRegistros, int32_t nroRegistrosRemovidos) {
    ch->byteProxReg = byteProxReg;
    ch->nroRegistros = nroRegistros;
    ch->nroRegistrosRemovidos = nroRegistrosRemovidos;
}

// Generates a CarHeader from a valid binary file.
CarHeader *_getCarHeaderFromBin(CarHeader *carHeader, FILE *file)
{
    // Set the file pointer to the start of the file
    fseek(file, 0, SEEK_SET);

    // Read the data
    fread(&carHeader->status, sizeof(carHeader->status), 1, file);
    fread(&carHeader->byteProxReg, sizeof(carHeader->byteProxReg), 1, file);
    fread(&carHeader->nroRegistros, sizeof(carHeader->nroRegistros), 1, file);
    fread(&carHeader->nroRegistrosRemovidos, sizeof(carHeader->nroRegistrosRemovidos), 1, file);
    fread(&carHeader->descrevePrefixo, sizeof(carHeader->descrevePrefixo), 1, file);
    fread(&carHeader->descreveData, sizeof(carHeader->descreveData), 1, file);
    fread(&carHeader->descreveLugares, sizeof(carHeader->descreveLugares), 1, file);
    fread(&carHeader->descreveLinha, sizeof(carHeader->descreveLinha), 1, file);
    fread(&carHeader->descreveModelo, sizeof(carHeader->descreveModelo), 1, file);
    fread(&carHeader->descreveCategoria, sizeof(carHeader->descreveCategoria), 1, file);
    return carHeader;
}

CarHeader *getCarHeader(CarHeader *carHeader, FILE *file, Source from)
{
    switch (from)
    {
    case BIN:
        return _getCarHeaderFromBin(carHeader, file);

    default:
        break;
    }
    return NULL;
}

// Write the CarHeader in a bin file
void _writeCarHeaderToBin(CarHeader *carHeader, FILE *file)
{
    // set the file pointer to the correct position
    fseek(file, 0, SEEK_SET);

    // write the data on the file
    fwrite(&carHeader->status, sizeof(carHeader->status), 1, file);
    fwrite(&carHeader->byteProxReg, sizeof(carHeader->byteProxReg), 1, file);
    fwrite(&carHeader->nroRegistros, sizeof(carHeader->nroRegistros), 1, file);
    fwrite(&carHeader->nroRegistrosRemovidos, sizeof(carHeader->nroRegistrosRemovidos), 1, file);
    fwrite(&carHeader->descrevePrefixo, sizeof(carHeader->descrevePrefixo), 1, file);
    fwrite(&carHeader->descreveData, sizeof(carHeader->descreveData), 1, file);
    fwrite(&carHeader->descreveLugares, sizeof(carHeader->descreveLugares), 1, file);
    fwrite(&carHeader->descreveLinha, sizeof(carHeader->descreveLinha), 1, file);
    fwrite(&carHeader->descreveModelo, sizeof(carHeader->descreveModelo), 1, file);
    fwrite(&carHeader->descreveCategoria, sizeof(carHeader->descreveCategoria), 1, file);
}

void writeCarHeader(CarHeader *carHeader, FILE *file, Source from)
{
    switch (from)
    {
    case BIN:
        _writeCarHeaderToBin(carHeader, file);
        break;

    default:
        break;
    }
}

void setCarFileStatus(FILE *file, char c)
{
    // if the char is valid
    if (c == REMOVED || c == NOT_REMOVED)
    {
        CarHeader *header = newCarHeader();
        header = getCarHeader(header, file, BIN);
        header->status = c;
        writeCarHeader(header, file, BIN);
        freeCarHeader(header);
    }
}

bool checkCarFileIntegrity(FILE *bin)
{
    CarHeader *ch = newCarHeader();
    getCarHeader(ch, bin, BIN);
    bool integrity = !checkCarHeaderIntegrity(ch);
    freeCarHeader(ch);

    return integrity;
}

int checkCarHeaderIntegrity(CarHeader *header)
{
    if (header->status == STATUS_CONSISTENT)
        return 1;
    return 0;
}

/* ## Basic Car functions ## */

Car *newCar()
{
    Car *car = malloc(sizeof(struct _Car));
    car->removido = NOT_REMOVED;
    car->tamanhoRegistro = STRUCT_BASE_CAR_SIZE;
    return car;
}

int getCarCodLinha(Car* c) {
    return c->codLinha;
}

// Reads the next car at the current file pointer
int64_t _readCarFromBIN(Car *car, FILE *file, int64_t customOffset)
{
    // if the pointer is pointing at the header, set the pointer for the first car in the file
    long long position = ftell(file);
    long long offset = position < STRUCT_CAR_HEADER_SIZE ? STRUCT_CAR_HEADER_SIZE : position;

    // Checking if offset was provided
    offset = customOffset != NO_OFFSET ? customOffset : offset;

    fseek(file, offset, SEEK_SET);

    // reads the field "removido" and "tamanhoRegistro" to analyse if car is valid
    fread(&car->removido, sizeof(car->removido), 1, file);
    fread(&car->tamanhoRegistro, sizeof(car->tamanhoRegistro), 1, file);
    // if the car is logically removed, point to the next car
    if (car->removido == REMOVED)
    {
        fseek(file, car->tamanhoRegistro, SEEK_CUR);
        return -1;
    }
    // read the data
    fread(&car->prefixo, sizeof(car->prefixo), 1, file);
    fread(&car->data, sizeof(car->data), 1, file);
    fread(&car->quantidadeLugares, sizeof(car->quantidadeLugares), 1, file);
    fread(&car->codLinha, sizeof(car->codLinha), 1, file);
    fread(&car->tamanhoModelo, sizeof(car->tamanhoModelo), 1, file);
    fread(&car->modelo, car->tamanhoModelo, 1, file);
    fread(&car->tamanhoCategoria, sizeof(car->tamanhoCategoria), 1, file);
    fread(&car->categoria, car->tamanhoCategoria, 1, file);
    return offset;
}

// Reads the next car from the standard input
int64_t _readCarFromCLI(Car *car)
{
    // Initializing zeroed char arrays and then reading from stdinput
    char prefixo[5] = {0};
    char data[10] = {0};
    char quantidadeLugares[MAX_STRING_SIZE] = {0};
    char codLinha[MAX_STRING_SIZE] = {0};
    char modelo[MAX_STRING_SIZE] = {0};
    char categoria[MAX_STRING_SIZE] = {0};

    // Scanning and removing possible quotations from each field
    scan_quote_string(prefixo);
    scan_quote_string(data);
    scan_quote_string(quantidadeLugares);
    scan_quote_string(codLinha);
    scan_quote_string(modelo);
    scan_quote_string(categoria);

    strncpy(car->prefixo, prefixo, sizeof(car->prefixo));

    // DATA
    if (data[0] == 0)
        fillWithGarbage(car->data, sizeof(car->data));
    else
        strncpy(car->data, data, sizeof(car->data));

    // QUANTIDADE DE LUGARES
    if (*quantidadeLugares == 0)
        car->quantidadeLugares = -1;
    else
        sscanf(quantidadeLugares, "%d", &car->quantidadeLugares);

    // COD LINHA
    if (*codLinha == 0)
        car->codLinha = -1;
    else
        sscanf(codLinha, "%d", &car->codLinha);

    // MODELO
    if (*modelo == 0)
        fillWithGarbage(car->modelo, sizeof(car->modelo));
    else
        strncpy(car->modelo, modelo, sizeof(car->modelo));

    // CATEGORIA
    if (*categoria == 0)
        fillWithGarbage(car->categoria, sizeof(car->categoria));
    else
        strncpy(car->categoria, categoria, sizeof(car->categoria));

    // verify if the register is removed
    if (car->prefixo[0] == '*')
    {
        car->removido = REMOVED;
        leftShift(car->prefixo, 5);
    }
    else
        car->removido = NOT_REMOVED;

    // calculates the lenght of car->modelo and car->categoria
    int lenghtModelo = strlen(car->modelo);
    int lenghtCategoria = strlen(car->categoria);
    car->tamanhoModelo = lenghtModelo;
    car->tamanhoCategoria = lenghtCategoria;

    // calculates the size of the register
    car->tamanhoRegistro = STRUCT_BASE_CAR_SIZE + lenghtModelo + lenghtCategoria;

    return -1;
}

int64_t readCar(Car *car, FILE *file, Source from, int64_t customOffset)
{
    switch (from)
    {
    case BIN:
        return _readCarFromBIN(car, file, customOffset);

    case CLI:
        return _readCarFromCLI(car);

    default:
        break;
    }
    return -1;
}



// Get a new string with value of the field of a Car.
// The string must be freed by the user.
char *_getCarContent(Car *car, CarField field)
{
    char *string = calloc(1, MAX_STRING_SIZE);
    switch (field)
    {
    case PREFIXO:
        strncpy(string, car->prefixo, sizeof(car->prefixo));
        break;

    case DATA:
        strncpy(string, car->data, sizeof(car->data));
        tranformDate(string);
        break;

    case QTD_LUGARES:
        snprintf(string, MAX_STRING_SIZE, "%d", car->quantidadeLugares);
        break;

    case COD_LINHA_CAR:
        snprintf(string, MAX_STRING_SIZE, "%d", car->codLinha);
        break;

    case MODELO:
        strncpy(string, car->modelo, car->tamanhoModelo);
        break;

    case CATEGORIA:
        strncpy(string, car->categoria, car->tamanhoCategoria);
        break;

    default:
        free(string);
        return NULL;
    }
    if (string[0] == 0 || (string[0] == '-' && string[1] == '1'))
    {
        strcpy(string, NULL_MESSAGE);
    }
    return string;
}

// Get a new string with the field description of a determined field of a CarHeader.
// The string must be freed by the user.
char *_getHeaderDescription(CarHeader *header, CarField field)
{
    char *string = calloc(1, MAX_STRING_SIZE);
    switch (field)
    {
    case PREFIXO:
        strncpy(string, header->descrevePrefixo, sizeof(header->descrevePrefixo));
        break;

    case DATA:
        strncpy(string, header->descreveData, sizeof(header->descreveData));
        break;

    case QTD_LUGARES:
        strncpy(string, header->descreveLugares, sizeof(header->descreveLugares));
        break;

    case COD_LINHA_CAR:
        strncpy(string, header->descreveLinha, sizeof(header->descreveLinha));
        break;

    case MODELO:
        strncpy(string, header->descreveModelo, sizeof(header->descreveModelo));
        break;

    case CATEGORIA:
        strncpy(string, header->descreveCategoria, sizeof(header->descreveCategoria));
        break;

    default:
        free(string);
        return NULL;
    }
    if (string[0] == 0 || (string[0] == '-' && string[1] == '1'))
    {
        strcpy(string, NULL_MESSAGE);
    }
    return string;
}

// Prints a single field from a Car
void _printField(CarHeader *header, Car *car, CarField field)
{
    char *fieldContent = _getCarContent(car, field);        // get the value of the field
    char *fieldName = _getHeaderDescription(header, field); // get the description of the field
    printf("%s: %s\n", fieldName, fieldContent);           // prints the field
    free(fieldContent);
    free(fieldName);
}

int printCar(Car *car, CarHeader *header)
{
    // verify id the car is removed
    if (car->removido == REMOVED)
        return 0;

    // print the fields required by the specification
    _printField(header, car, PREFIXO);
    _printField(header, car, MODELO);
    _printField(header, car, CATEGORIA);
    _printField(header, car, DATA);
    _printField(header, car, QTD_LUGARES);

    return 1;
}

int32_t getCarIndex(Car *c)
{
    return convertePrefixo(c->prefixo);
}

bool carLogicallyRemoved(Car *c)
{
    if (c->removido == REMOVED)
        return true;
    return false;
}

void freeCar(Car *c)
{
    free(c);
}

void freeCarHeader(CarHeader *carHeader)
{
    free(carHeader);
}

/* ## Functions related to writing Cars to different sources ## */

// Writes a Car to end of binary file.
int64_t _writeCarToBin(Car *car, FILE *file)
{
    // gets the information from the header and update it
    CarHeader *header = newCarHeader();
    getCarHeader(header, file, BIN);
    header->status = '0';
    int byteOffset = header->byteProxReg;
    header->byteProxReg += car->tamanhoRegistro + sizeof(car->removido) + sizeof(car->tamanhoRegistro);

    // verifica se o registro é removido ou não
    if (car->removido == NOT_REMOVED)
    {
        header->nroRegistros++;
    }
    else
    {
        header->nroRegistrosRemovidos++;
    }

    // writes the header with the updated information
    writeCarHeader(header, file, BIN);
    freeCarHeader(header);

    // set the file pointer to the correct position
    fseek(file, byteOffset, SEEK_SET);

    // write the data in the file
    fwrite(&car->removido, sizeof(car->removido), 1, file);
    fwrite(&car->tamanhoRegistro, sizeof(car->tamanhoRegistro), 1, file);
    fwrite(&car->prefixo, sizeof(car->prefixo), 1, file);
    fwrite(&car->data, sizeof(car->data), 1, file);
    fwrite(&car->quantidadeLugares, sizeof(car->quantidadeLugares), 1, file);
    fwrite(&car->codLinha, sizeof(car->codLinha), 1, file);
    fwrite(&car->tamanhoModelo, sizeof(car->tamanhoModelo), 1, file);
    fwrite(&car->modelo, car->tamanhoModelo, 1, file);
    fwrite(&car->tamanhoCategoria, sizeof(car->tamanhoCategoria), 1, file);
    fwrite(&car->categoria, car->tamanhoCategoria, 1, file);

    return byteOffset;
}

int64_t writeCar(Car *car, FILE *file, Source from)
{
    switch (from)
    {
    case BIN:
        return _writeCarToBin(car, file);
        break;

    default:
        break;
    }

    return -1;
}

/* ## Functions related to searching using a specific struct field ## */
