#include "structs.h"
#include "x.h"

void readFromBlocks(FILE* fileInDesc, int choice, int meta) {
	int i, j;
	BlockStats* BS = malloc(sizeof(BlockStats));
	void *blockPtr1 = malloc(sizeof(int));							//to read all the ints
	void *blockPtr2 = malloc(49*sizeof(char));						//to read the filenames
	void *blockPtr3 = malloc(10*sizeof(char));						//to read the access rights
	void* blockPtr4 = malloc(BLOCK_SIZE*sizeof(char));				//to read blocksize from file

	//BLOCK STATS//
	fread(&blockPtr1, 1, sizeof(int), fileInDesc);
	BS->numOfFiles = (int)blockPtr1;
	fread(&blockPtr1, 1, sizeof(int), fileInDesc);
	BS->numOfDirs = (int)blockPtr1;
	fread(&blockPtr1, 1, sizeof(int), fileInDesc);
	BS->numOfBlocks = (int)blockPtr1;
	//END OF BLOCK STATS//

	Dirs* retrievedDirectories = malloc(sizeof(Dirs));
	Files* retrievedFiles = malloc(sizeof(Files));
	retrievedDirectories = NULL;
	retrievedFiles = NULL;
	int offset = 0;
	offset += sizeof(BlockStats);

	//RESTORE DIRS//
	int dirs = BS->numOfDirs;
	for (i = 0; i < dirs; ++i) {	
		Dirs* temp = malloc(sizeof(Dirs));
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->size = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->level = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->parent = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->TreeID = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->numOfBlocks = (int)blockPtr1;
		fread(blockPtr2, 1, 49*sizeof(char), fileInDesc);
		strcpy(temp->dirname, (char*)blockPtr2);
		fread(blockPtr3, 1, 10*sizeof(char), fileInDesc);
		strcpy(temp->accessRights, (char*)blockPtr3);
		if (retrievedDirectories == NULL) {
			retrievedDirectories = temp;
			retrievedDirectories->next = NULL;
		}
		else {
			Dirs* seek = retrievedDirectories;
			while (seek->next != NULL) {
				seek = seek->next;
			}
			seek->next = temp;
		}
		rewind(fileInDesc);
		offset += sizeof(Dirs);
		fseek(fileInDesc, offset, SEEK_CUR);
	}
	//END OF DIRS//

	//RESTORE FILES//
	int files = BS->numOfFiles;
	for (i = 0; i < files; ++i) {
		Files* temp = malloc(sizeof(Files));
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->size = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->level = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->parent = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->TreeID = (int)blockPtr1;
		fread(&blockPtr1, 1, sizeof(int), fileInDesc);
		temp->numOfBlocks = (int)blockPtr1;
		fread(blockPtr2, 1, 49*sizeof(char), fileInDesc);
		strcpy(temp->filename, (char*)blockPtr2);
		fread(blockPtr3, 1, 10*sizeof(char), fileInDesc);
		strcpy(temp->accessRights, (char*)blockPtr3);
		if (retrievedFiles == NULL) {
			retrievedFiles = temp;
			retrievedFiles->next = NULL;
		}
		else {
			Files* seek = retrievedFiles;
			while (seek->next != NULL) {
				seek = seek->next;
			}
			seek->next = temp;
		}
		rewind(fileInDesc);
		offset += sizeof(Files);
		fseek(fileInDesc, offset, SEEK_CUR);
	}
	//END OF FILES//
	
	if (choice)
		remake(retrievedDirectories, retrievedFiles, fileInDesc);
	
	if (meta == 1) {
		struct stat statbuf;
		struct group  *grp;
		struct passwd *pwd;
		printf("Metadata printing...\n");

		Dirs* seekDirs = retrievedDirectories;
		while (seekDirs->next != NULL) {
			printf("Directory name: %s\n", seekDirs->dirname);
			printf("--Size: %d\n", seekDirs->size);
			printf("--Access Rights: %s\n", seekDirs->accessRights);
			if (stat(seekDirs->dirname, &statbuf) != -1) {
				if ((grp = getgrgid(statbuf.st_gid)) != NULL)
					printf("--Group: %-20.20s\n", grp->gr_name);
				else
					printf("--Group: %-20d\n", statbuf.st_uid);
				if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
					printf("--User: %-20.20s\n", pwd->pw_name);
				else 
					printf("--User: %-20d\n", statbuf.st_uid);
			}
			seekDirs = seekDirs->next;
		}
		Files* seekFiles = retrievedFiles;
		while (seekFiles->next != NULL) {
			printf("Filename: %s\n", seekFiles->filename);
			printf("--Size: %d\n", seekFiles->size);
			printf("--Access Rights: %s\n", seekFiles->accessRights);
			seekFiles = seekFiles->next;
			if (stat(seekFiles->filename, &statbuf) != -1) {
				if ((grp = getgrgid(statbuf.st_gid)) != NULL)
				printf("--Group: %-20.20s\n", grp->gr_name);
			else
				printf("--Group: %-20d\n", statbuf.st_uid);
			if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
				printf("--User: %-20.20s\n", pwd->pw_name);
			else 
				printf("--User: %-20d\n", statbuf.st_uid);
			}
		}
	}

	if (meta == 2) {
		printf("Hierarchy printing...\n\n");
		Dirs* seekDirs = retrievedDirectories;
		printf("--DIRECTORIES--\n");
		while (seekDirs != NULL) {
			printf("Directory: %s\n", seekDirs->dirname);
			seekDirs = seekDirs->next;
		}
		printf("\n");
		printf("--FILES--\n");
		Files* seekFiles = retrievedFiles;
		while (seekFiles != NULL) {
			printf("Directory: %s\n", seekFiles->filename);
			seekFiles = seekFiles->next;
		}
	}

	return;
}

void remake(Dirs* retrievedDirectories, Files* retrievedFiles, FILE* fileInDesc) {
	Dirs* remakeDirs = retrievedDirectories;
	void* blockPtr4 = malloc(BLOCK_SIZE*sizeof(char));
	char dirname[49];
	while (remakeDirs != NULL) {
		strcpy(dirname, remakeDirs->dirname);
		printf("Directory %s\n", dirname);
		if ((mkdir(dirname, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1) {
			perror("Directory");
		}
		remakeDirs = remakeDirs->next;
	}
	printf("Directories done\n");
	//END OF REMAKE DIRECTORIES//

	Files* remakeFiles = retrievedFiles;
	char filename[49];
	FILE* create;
	while (remakeFiles != NULL) {
		strcpy(filename, remakeFiles->filename);
		printf("File %s is writen on %d blocks\n", filename, remakeFiles->numOfBlocks);
		create = fopen(filename, "a");
		int offset = 0;
		int blocksRead = 0;
		while (offset < remakeFiles->size && blocksRead < remakeFiles->numOfBlocks) {
			fread(blockPtr4, 1, BLOCK_SIZE, fileInDesc);
			fwrite(blockPtr4, 1, BLOCK_SIZE, create);
			blocksRead++;
			offset += BLOCK_SIZE;
		}
		int sizeleft = remakeFiles->size - ftell(create);
		if (sizeleft > 0) {
			void* blockPtr5 = malloc(sizeleft*sizeof(char));
			fread(blockPtr5, 1, sizeleft, fileInDesc);
			fwrite(blockPtr5, 1, sizeleft, create);
			//free(blockPtr5);
		} 
		fclose(create);
		//free(blockPtr4);
		remakeFiles = remakeFiles->next;
	}
	printf("Files done\n");
}

