#include "structs.h"
#include "c.h"

int dirID = 0;
int fileID = 0;

void list(Node* Ierarchy) {										//recursive function that creates the hierachy
	DIR *directory, *tempDirectory;
	char newname[49];
	struct dirent *dir;
	if ((directory = opendir(Ierarchy->name)) == NULL) {		//tries to open the node's name
		return;
	}
	while ((dir = readdir(directory)) != NULL) {				//if the name is a directory, open the directory
		if (dir->d_ino == 0) continue;				
		if (strcmp(dir->d_name, ".") == 0) continue;			//ignore current directory
		if (strcmp(dir->d_name, "..") == 0) continue;			//ignore parent directory
	
		strcpy(newname, Ierarchy->name);						//create the full path of the new element found in the directory
		strcat(newname, "/");
		strcat(newname, dir->d_name);

		if ((tempDirectory = opendir(newname)) == NULL) {		//if by trying to open the new element you get NULL
			fileID++;
			Files* newFile = malloc(sizeof(Files));				//it is a file, create a file node
			strcpy(newFile->filename, newname);
			newFile->level = Ierarchy->level + 1;				//append it to the visited directory's list of files
			newFile->parent = Ierarchy->dirID;
			newFile->TreeID = Ierarchy->TreeID;
			newFile->next = NULL;
			
			if (Ierarchy->files == NULL) {
				Ierarchy->files = newFile;
			}
			else {
				Files* tempFile = Ierarchy->files;
				while (tempFile->next != NULL) {
					tempFile = tempFile->next;
				}
				tempFile->next = newFile;
			}
		}
		else {													//else if by trying to open the new element you find a directory
			Node* newNode = malloc(sizeof(Node));				//append it to the general list of directories
			dirID++;
			strcpy(newNode->name, newname);

			newNode->level = Ierarchy->level + 1;
			newNode->TreeID = Ierarchy->TreeID;
			newNode->dirID = dirID;
			newNode->isDir = 1;
			newNode->parent = Ierarchy->dirID;

			Node* temp = Ierarchy;
			while (temp->next != NULL) {
				temp = temp->next;
			}
			
			newNode->prev   = temp;
			newNode->next   = NULL;
			newNode->dirs   = NULL;
			newNode->files  = NULL;
			temp->next  = newNode;

			Dirs* newDir = malloc(sizeof(Dirs));				//and append it to the list of directories of the current 
			strcpy(newDir->dirname, newname);					//directory we are working on
			newDir->level = newNode->level;
			newDir->parent = Ierarchy->dirID;
			newDir->TreeID = Ierarchy->TreeID;
			newDir->next = NULL;
			if (Ierarchy->dirs == NULL) {
				Ierarchy->dirs = newDir;
			}
			else {
				Dirs* tempDir = Ierarchy->dirs;
				while (tempDir->next != NULL) {
					tempDir = tempDir->next;
				}
				tempDir->next = newDir;
			}
			list(newNode);										//and since you found a directory, visit the new directory
		}														//and do the same things, open, get its files and directories, append them etc.
	}
	closedir(directory);
}

Dirs* listDirectories(int* DirsCount, int numOfFiles, Node* Ierarchy) {
	//gets every directory from the hierarchy and creates a simply linked list
	Dirs* Directories = NULL;
	int i, j;
	for (i = 0; i < numOfFiles; ++i) {
		int currDirs = DirsCount[i];
		Node* temp = &Ierarchy[i];
		for (j = 0; j < currDirs; ++j) {
			Dirs* tempDir = malloc(sizeof(Dirs));
			strcpy(tempDir->dirname, temp->name);
			tempDir->level = temp->level;
			tempDir->parent = temp->parent;
			tempDir->TreeID = temp->TreeID;
			tempDir->next = NULL;
			if (Directories == NULL) {
				Directories = tempDir;
			}
			else {
				Dirs* tempDir2 = Directories;
				while (tempDir2->next != NULL) {
					tempDir2 = tempDir2->next;
				}
				tempDir2->next = tempDir;
			}
			temp = temp->next;
		}	
	}
	return Directories;
}

Files* listFiles(int* FilesCount, int* DirsCount, int numOfFiles, Node* Ierarchy) {
	//gets every file from the hierarchy and creates a simply linked list
	Files* FilesList = NULL;
	int i, j;
	for (i = 0; i < numOfFiles; ++i) {
		int currFiles = FilesCount[i];
		int currDirs = DirsCount[i];
		Node* temp = &Ierarchy[i];
		for (j = 0; j < currDirs; ++j) {
			if (temp->files != NULL) {
				Files* tempFile = temp->files;
				while (tempFile != NULL) {
					Files* toADD = malloc(sizeof(Files));
					strcpy(toADD->filename, tempFile->filename);
					toADD->level = tempFile->level;
					toADD->parent = tempFile->parent;
					toADD->TreeID = tempFile->TreeID;
					toADD->next = NULL;
					if (FilesList == NULL) {
						FilesList = toADD;
					}
					else {
						Files* tempFile2 = FilesList;
						while (tempFile2->next != NULL) {
							tempFile2 = tempFile2->next;
						}
						tempFile2->next = toADD;
					}
					tempFile = tempFile->next;
				}
			}
			temp = temp->next;
		}
	}
	return FilesList;
}

void createDirsMetadata(Dirs* Directories) {
	//strcpy to a buffer the access rights of each directory and pass it to the list
	struct stat statbuf;
	struct group  *grp;
	struct passwd *pwd;
	Dirs* tempDirs = Directories;
	while (tempDirs != NULL) {
		if (stat(tempDirs->dirname, &statbuf) != -1) {
			char accessRights[10];
			if (S_ISDIR(statbuf.st_mode))  accessRights[0] = 'd'; else accessRights[0] = '-';
			if (statbuf.st_mode & S_IRUSR) accessRights[1] = 'r'; else accessRights[1] = '-';
			if (statbuf.st_mode & S_IWUSR) accessRights[2] = 'w'; else accessRights[2] = '-';
			if (statbuf.st_mode & S_IXUSR) accessRights[3] = 'x'; else accessRights[3] = '-';
			if (statbuf.st_mode & S_IRGRP) accessRights[4] = 'r'; else accessRights[4] = '-';
			if (statbuf.st_mode & S_IWGRP) accessRights[5] = 'w'; else accessRights[5] = '-';
			if (statbuf.st_mode & S_IXGRP) accessRights[6] = 'x'; else accessRights[6] = '-';
			if (statbuf.st_mode & S_IROTH) accessRights[7] = 'r'; else accessRights[7] = '-';
			if (statbuf.st_mode & S_IWOTH) accessRights[8] = 'w'; else accessRights[8] = '-';
			if (statbuf.st_mode & S_IXOTH) accessRights[9] = 'x'; else accessRights[9] = '-';
			strcpy(tempDirs->accessRights, accessRights);
			tempDirs->size = statbuf.st_size;
		}
		tempDirs = tempDirs->next;
	}
	return;
}

void createFileMetadata(Files* FilesList) {
	//strcpy to a buffer the access rights of each file and pass it to the list
	struct stat statbuf;
	struct group  *grp;
	struct passwd *pwd;
	Files* tempFiles = FilesList;
	while (tempFiles != NULL) {
		if (stat(tempFiles->filename, &statbuf) != -1) {
			char accessRights[10];
			if (S_ISDIR(statbuf.st_mode))  accessRights[0] = 'd'; else accessRights[0] = '-';
			if (statbuf.st_mode & S_IRUSR) accessRights[1] = 'r'; else accessRights[1] = '-';
			if (statbuf.st_mode & S_IWUSR) accessRights[2] = 'w'; else accessRights[2] = '-';
			if (statbuf.st_mode & S_IXUSR) accessRights[3] = 'x'; else accessRights[3] = '-';
			if (statbuf.st_mode & S_IRGRP) accessRights[4] = 'r'; else accessRights[4] = '-';
			if (statbuf.st_mode & S_IWGRP) accessRights[5] = 'w'; else accessRights[5] = '-';
			if (statbuf.st_mode & S_IXGRP) accessRights[6] = 'x'; else accessRights[6] = '-';
			if (statbuf.st_mode & S_IROTH) accessRights[7] = 'r'; else accessRights[7] = '-';
			if (statbuf.st_mode & S_IWOTH) accessRights[8] = 'w'; else accessRights[8] = '-';
			if (statbuf.st_mode & S_IXOTH) accessRights[9] = 'x'; else accessRights[9] = '-';
			strcpy(tempFiles->accessRights, accessRights);
			tempFiles->size = statbuf.st_size;
		}
		tempFiles = tempFiles->next;
	}
	return;
}

void writeMetadata(FILE* fileOutDesc, Dirs* Directories, Files* FilesList, BlockStats* BS, int zip) {
	int sizeOfFiles = sizeof(Files);
	int sizeOfDirs = sizeof(Dirs);
	int sizeofBS = sizeof(BlockStats);
	int totalSize = sizeofBS + sizeOfFiles*(BS->numOfFiles) + sizeOfDirs*(BS->numOfDirs);
	int blocksToAlloc = totalSize/BLOCK_SIZE;
	if (totalSize%BLOCK_SIZE) {
		blocksToAlloc++;
	}
	BS->numOfBlocks = blocksToAlloc;
	Dirs*  tempDirsB = Directories;
	Files* tempFilesB = FilesList;
	int counter = sizeofBS;
	int i, endFlag = 0, endFlag2 = 0;

	void *blockPtr = calloc(BLOCK_SIZE, sizeof(char));
	memcpy(blockPtr, BS, sizeof(BlockStats));
	fwrite(blockPtr, sizeof(BlockStats), 1, fileOutDesc);
	blockPtr += sizeof(BlockStats);

	for (i = 0; i < blocksToAlloc; ++i) {
		while ((counter + sizeOfDirs) <= BLOCK_SIZE) {
			if (endFlag && endFlag2) break;
			if (endFlag != 1) {
				tempDirsB->numOfBlocks = 1;
				memcpy(blockPtr, tempDirsB, sizeof(Dirs));
				fwrite(blockPtr, 1, sizeof(Dirs), fileOutDesc);
				blockPtr += sizeof(Dirs);
				tempDirsB = tempDirsB->next;
				if (tempDirsB == NULL) {
					endFlag = 1;
				}
				counter += sizeOfDirs;
			}
			else {
				int blocksNum = (tempFilesB->size)/BLOCK_SIZE;
				if ((tempFilesB->size)%BLOCK_SIZE) blocksNum++;
				tempFilesB->numOfBlocks = blocksNum;
				//if (zip) {
				//	if (strstr(tempFilesB->filename, ".tar") != NULL) {
				//		memcpy(blockPtr, tempFilesB, sizeof(Files));
				//		fwrite(blockPtr, 1, sizeof(Files), fileOutDesc);
				//		blockPtr += sizeof(Files);
				//	}
				//}
				//else {
					memcpy(blockPtr, tempFilesB, sizeof(Files));
					fwrite(blockPtr, 1, sizeof(Files), fileOutDesc);
					blockPtr += sizeof(Files);
				//}
				tempFilesB = tempFilesB->next;
				if (tempFilesB == NULL) {
					endFlag2 = 1;
				}
				counter += sizeOfFiles;
			}
		}
		blockPtr -= counter;
		counter = 0;
	}
	printf("Done with the metadata.\n");

	tempFilesB = FilesList;
	char fname[49];
	FILE* fileInDesc;

	while (tempFilesB != NULL) {
		strcpy(fname, tempFilesB->filename);
		if ((fileInDesc = fopen(fname, "r")) == NULL) {
			perror("Open original file");
		}

		fseek(fileInDesc, 0, SEEK_END);
		int numBytes = ftell(fileInDesc);
		rewind(fileInDesc);

		int blocksNum = numBytes / BLOCK_SIZE;
		if (numBytes%BLOCK_SIZE) {
			blocksNum++;
		}
		void *blockPtr = malloc(BLOCK_SIZE);
		for (i = 0; i < (blocksNum - 1); ++i) {
			int size = fread(blockPtr, 1, BLOCK_SIZE, fileInDesc);
			fwrite(blockPtr, 1, size, fileOutDesc);
		}

		char ch = '0';
		if (numBytes%BLOCK_SIZE) {
			int size = fread(blockPtr, 1, (numBytes%BLOCK_SIZE) + 1, fileInDesc);
			int zero_bytes = BLOCK_SIZE - size;
			fwrite(blockPtr, 1, size, fileOutDesc);
			for (i = 0; i < zero_bytes; ++i) {
				size = fwrite(&ch, sizeof(char), 1, fileOutDesc);
				if (size == -1) perror("write");
			}
		}

		fclose(fileInDesc);
		tempFilesB = tempFilesB->next;
	}
	printf("Done with the files.\n");
}
