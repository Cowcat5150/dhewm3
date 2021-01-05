/*
** This file contains glue linked into the "shared" object
*/

#include "dll.h"

extern void *GetGameAPI(void *);


dll_tExportSymbol DLL_ExportSymbols[] =
{
    {(void *)GetGameAPI, "GetGameAPI"},
    {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
    {0,0,0,0}
};

int DLL_Init(void)
{
    return 1;
}

void DLL_DeInit(void)
{
}
