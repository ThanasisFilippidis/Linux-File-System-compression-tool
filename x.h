#ifndef X_H
#define X_H

#include "structs.h"

void readFromBlocks(FILE* fileInDesc, int choice, int meta);
void remake(Dirs* retrievedDirectories, Files* retrievedFiles, FILE* fileInDesc);
Dirs* getDirs(FILE* fileInDesc, int* files);
Files* getFiles(FILE* fileInDesc, int* files);

#endif
