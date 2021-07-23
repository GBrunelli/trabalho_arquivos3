#include "b_tree.h"

#define DISK_PAGE_SIZE 77

typedef struct _BTreeHeader IndexHeader;
typedef struct _BTreeNode DiskPage;

struct _Index
{
    FILE *indexFile;
    IndexHeader *header;
    DiskPage **savedPages;
    int nSavedPages;
};

struct _BTreeHeader
{
    bool status;
    int32_t noRaiz;
    int32_t RRNproxNo;
};

struct _BTreeNode
{
    // metadata
    bool folha;
    int32_t nroChavesIndexadas;
    int32_t RRNdoNo;

    // data
    int32_t P[ORDER];
    Register *regs[REGISTERS_PER_PAGE];
};

struct _Register
{
    int32_t C;
    int64_t Pr;
};

/**
 * Write "bytes * @" in the current file
*/
void _writeGarbage(FILE *file, int bytes)
{
    char garbage[1] = "@";
    for (int i = 0; i < bytes; i++)
        fwrite(&garbage, sizeof(char), 1, file);
}

/**
 * Write the header of the index in the file
*/
void _writeIndexHeader(Index *index)
{
    FILE *indexFile = index->indexFile;
    fseek(indexFile, 0, SEEK_SET);
    char status = index->header->status ? '1' : '0';
    fwrite(&status, 1, 1, indexFile);
    fwrite(&index->header->noRaiz, 4, 1, indexFile);
    fwrite(&index->header->RRNproxNo, 4, 1, indexFile);
    _writeGarbage(indexFile, 68);
}

/**
 * Opens an existing index file
*/
Index *openIndex(FILE *indexFile)
{
    Index *index = malloc(sizeof(Index));
    index->indexFile = indexFile;
    if (!index->indexFile)
    {
        free(index);
        return NULL;
    }
    index->header = malloc(sizeof(IndexHeader));
    index->savedPages = NULL;
    index->nSavedPages = 0;
    fseek(index->indexFile, 0, SEEK_SET);
    char status;
    fread(&status, 1, 1, index->indexFile);
    if (status == '0')
        return NULL;

    index->header->status = false;
    fread(&index->header->noRaiz, 4, 1, index->indexFile);
    fread(&index->header->RRNproxNo, 4, 1, index->indexFile);
    return index;
}
/**
 * Closes the index file
*/
void closeIndex(Index *index)
{
    index->header->status = true;
    _writeIndexHeader(index);
    fclose(index->indexFile);
    free(index->header);
    for (int i = 0; i < index->nSavedPages; i++)
    {
        for (int j = 0; j < index->savedPages[i]->nroChavesIndexadas; j++)
        {
            free(index->savedPages[i]->regs[j]);
        }
        free(index->savedPages[i]);
    }
    free(index->savedPages);
    free(index);
}
/**
 * Creates and returns a filled register
*/
Register *createRegister(int32_t C, int64_t Pr)
{
    Register *reg = malloc(sizeof(Register));
    reg->C = C;
    reg->Pr = Pr;
    return reg;
}

/**
 * Frees all data associated with an specific register
 */ 
void freeRegister(Register *reg) {
    free(reg);
}

/**
 * Creates a new disk page for a given index file, with the correct rnn
*/
DiskPage *_createDiskPage(Index *index, bool folha)
{
    DiskPage *page = malloc(sizeof(DiskPage));
    page->folha = folha;
    page->nroChavesIndexadas = 1;
    for (int i = 0; i < ORDER; i++)
        page->P[i] = -1;
    for (int i = 0; i < REGISTERS_PER_PAGE; i++)
        page->regs[i] = NULL;

    page->RRNdoNo = index->header->RRNproxNo++;
    return page;
}

/**
 * Saves a disk page in memory for fast access 
*/
void _saveDiskPageInMemory(Index *index, DiskPage *page)
{
    index->savedPages = realloc(index->savedPages, (++index->nSavedPages) * sizeof(DiskPage *));
    index->savedPages[(index->nSavedPages) - 1] = page;
}

/**
 * Gets a node from the index file given its rnn
*/
DiskPage *_getPage(Index *index, int32_t rnn)
{
    int nPages = index->nSavedPages;
    for (int i = 0; i < nPages; i++)
    {
        if (index->savedPages[i]->RRNdoNo == rnn)
            return index->savedPages[i];
    }

    // if the disk page is not on memory, recover it

    // allocates the new page
    DiskPage *page = malloc(sizeof(DiskPage));
    for (int i = 0; i < ORDER; i++)
        page->P[i] = -1;

    // read and saves the fields
    fseek(index->indexFile, (rnn + 1) * DISK_PAGE_SIZE, SEEK_SET);
    char folha;
    int32_t nroChavesIndexadas;
    int32_t RRNdoNo;
    fread(&folha, 1, 1, index->indexFile);
    fread(&nroChavesIndexadas, 4, 1, index->indexFile);
    fread(&RRNdoNo, 4, 1, index->indexFile);
    page->folha = folha == '1' ? true : false;
    page->nroChavesIndexadas = nroChavesIndexadas;
    page->RRNdoNo = RRNdoNo;
    for (int i = 0; i < REGISTERS_PER_PAGE; i++)
    {
        fread(&page->P[i], 4, 1, index->indexFile);
        int32_t C;
        int64_t Pr;
        fread(&C, 4, 1, index->indexFile);
        fread(&Pr, 8, 1, index->indexFile);
        bool CIsGarbage = (C == -1);
        bool PrIsGarbage = (Pr == -1);
        if (!(CIsGarbage || CIsGarbage))
            page->regs[i] = createRegister(CIsGarbage ? -1 : C,
                                           PrIsGarbage ? -1 : Pr);
        else
            break;
    }
    fread(&page->P[REGISTERS_PER_PAGE], 4, 1, index->indexFile);
    _saveDiskPageInMemory(index, page);
    return page;
}

/**
 * Makes a binary search in the node for the key, if found, 
 * returns FOUND, the pos of the key, and the found register
 * else returns NOT_FOUND
*/
Result _binaryNodeSearch(DiskPage *page,
                         int32_t key,
                         Register **foundReg,
                         int *pos)
{
    int begin = 0;
    int end = page->nroChavesIndexadas - 1;
    int middle = 0;
    while (begin <= end)
    {
        middle = (begin + end) / 2;
        if (page->regs[middle]->C == key)
        {
            *foundReg = page->regs[middle];
            return FOUND;
        }
        if (page->regs[middle]->C < key)
            begin = middle + 1;
        else
            end = middle - 1;
    }
    if (pos != NULL)
    {
        if (page->regs[middle]->C > key)
            *pos = middle;
        else
            *pos = middle + 1;
    }
    return NOT_FOUND;
}

/**
 * Recursive function for searching the btree
*/
Result _searchRegister(Index *index,
                       int32_t currentRNN,
                       int32_t key,
                       Register **foundReg)
{
    // the we reach a leaf, there is no register with the given key
    if (currentRNN == -1)
        return NOT_FOUND;

    else
    {
        // keep calling the functino for the child node
        int pos;
        DiskPage *node = _getPage(index, currentRNN);
        Result result = _binaryNodeSearch(node, key, foundReg, &pos);
        if (result == FOUND)
            return FOUND;
        else
            return _searchRegister(index, node->P[pos], key, foundReg);
    }
}

/**
 * Searches the btree for a key, if found, returns FOUND, 
 * and the found register, else returns NOT_FOUND
*/
Result searchRegister(Index *index, int32_t key, Register **foundReg)
{
    return _searchRegister(index, index->header->noRaiz, key, foundReg);
}
