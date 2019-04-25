#ifndef C_H
#define C_H

#include "structs.h"

extern int dirID;
extern int fileID;

void list(Node* Ierarchy);
Dirs*  listDirectories(int* DirsCount, int numOfFiles, Node* Ierarchy);
Files* listFiles(int* FilesCount, int* DirsCount, int numOfFiles, Node* Ierarchy);
void createDirsMetadata(Dirs* Directories);
void createFileMetadata(Files* FilesList);
void writeMetadata(FILE* fileOutDesc, Dirs* Directories, Files* FilesList, BlockStats* BS, int zip);

#endif
