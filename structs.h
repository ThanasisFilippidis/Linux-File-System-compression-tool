#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#define BLOCK_SIZE 512

typedef struct BlockStats {
	int numOfFiles;
	int numOfDirs;
	int numOfBlocks;
} BlockStats;

typedef struct Files {
	int size;
	int level;
	int parent;
	int TreeID;
	int numOfBlocks;
	char filename[49];
	char accessRights[10];
	struct Files* next;
} Files;

typedef struct Dirs {
	int size;
	int level;
	int parent;
	int TreeID;
	int numOfBlocks;
	char dirname[49];
	char accessRights[10];
	struct Dirs* next;
} Dirs;

typedef struct Node {
	char name[49];
	int level;
	int TreeID;
	int dirID;
	int isDir;
	int parent;
	struct Node* next;
	struct Node* prev;
	struct Dirs* dirs;
	struct Files* files;
} Node;

#endif
