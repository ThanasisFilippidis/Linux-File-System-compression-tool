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
#include "structs.h"
#include "c.h"
#include "x.h"

int dirID;
int fileID;

int main(int argc, char const *argv[]) {
	int i, flag = 0;
	char choice, choice2;

	if (argc < 3) {												//we need at least: mydiz, a flag and a filename
		printf("Arguments missing, correct format: ./mydiz {-c|-a|-x|-m|-d|-p|-j} <archive-file> <list-of-files/dirs>\n");
		printf("Please try again. Exiting...\n");
		exit(-1);
	}
	choice = argv[1][0];
	if (choice == '-') {
		choice = argv[1][1];
	}
	else {
		printf("No flags given. Exiting...\n");
		exit(-1);
	}

	switch (choice) {
		case 'c': {
			if (argc == 3) {									//for c, a, j and q we need a list of files or directories
				printf("No files detected.\n");
				printf("Correct format: ./mydiz {-c|-a|-x|-m|-d|-p|-j} <archive-file> <list-of-files/dirs>\n");
				exit(-1);
			}
			printf("Archive data process begins.\n");
			flag = 1;											//a and c can be run with -j too
			break;												//so we need to check if there is a second flag
		}
		case 'a': {
			if (argc == 3) {
				printf("No files detected.\n");
				printf("Correct format: ./mydiz {-c|-a|-x|-m|-d|-p|-j} <archive-file> <list-of-files/dirs>\n");
				exit(-1);
			}
			printf("Addition to directory process begins.\n");
			flag = 1;
			break;
		}
		case 'x': {
			printf("Data extraction process begins.\n");
			break;
		}
		case 'd': {
			printf("Data removal process begins.\n");
			break;
		}
		case 'm': {
			printf("Metadata printing process begins.\n");
			break;
		}
		case 'q': {
			if (argc == 3) {
				printf("No files detected.\n");
				exit(-1);
			}
			printf("Query process begins.\n");
			break;
		}
		case 'p': {
			printf("Ierarchy printing process begins.\n");
			break;
		}
		default: {
			printf("Unknown operation. Exiting...\n");
			exit(-1);
		}
	}

	if (flag == 1) {										//if -c or -a is given
		choice2 = argv[2][0];								//get the first char of the next argument
		if (choice2 == '-') {								//if it is -
			choice2 = argv[2][1];							//there is a second flag
			switch (choice2) {								//the only acceptable flag is -j
				case 'j': {
					printf("Archive data process will begin shortly.\n");
					break;
				}
				default: {
					printf("Unknown operation. Exiting...\n");
					exit(-1);
				}
			}
			flag = 2;
		}
		else 
			choice2 = '-';
	}

	char archiveFileName[49];								//copy the .di file 
	if (flag == 2) {										//in order to open or create it later
		strcpy(archiveFileName, argv[3]);
		flag = 3;
	}
	else 
		strcpy(archiveFileName, argv[2]);

	printf("Archive file name is: %s\n", archiveFileName);
	printf("Operations are: %c, %c\n", choice, choice2);

	if (choice == 'c' || choice == 'a' || choice == 'q') {

		if (choice == 'c') {
			printf("Creating hierarchy...\n");
			int numOfFiles;									//create an array with the files given at the command line
			char listOfFiles[argc - 3][49];					//each set of files is considered a single hierarchy or tree

			if (flag == 3) {
				numOfFiles = argc - 4;
				int offset = 0;
				for (i = 4; i < argc; ++i) {
					strcpy(listOfFiles[offset], argv[i]);
					offset++;
				}
			}
			else {
				numOfFiles = argc - 3;
				int offset = 0;
				for (i = 3; i < argc; ++i) {
					strcpy(listOfFiles[offset], argv[i]);
					offset++;
				}
			}

			int DirsCount[numOfFiles];							//count directories of each hierarchy
			int FilesCount[numOfFiles];							//count files of each hierarchy
			Node* Ierarchy = malloc(numOfFiles * sizeof(Node));
			BlockStats* BS = malloc(sizeof(BlockStats));		//Header structure
			FILE*  fileOutDesc, *fileInDesc;
			DIR *directory;
			int treeID = 0;
			
			for (i = 0; i < numOfFiles; ++i) {
				strcpy(Ierarchy[i].name, listOfFiles[i]);
				Ierarchy[i].TreeID = treeID;
				Ierarchy[i].dirs   = NULL;
				Ierarchy[i].files  = NULL;
				Ierarchy[i].level  = 0;
				Ierarchy[i].parent = -1;
				Ierarchy[i].prev   = NULL;
				Ierarchy[i].next   = NULL;
				if ((directory = opendir(Ierarchy->name)) != NULL) {
					Ierarchy[i].isDir = 1;
					Ierarchy[i].dirID = dirID;
					closedir(directory);
					list(&Ierarchy[i]);
				}
				else {
					Ierarchy[i].isDir = 0;
				}
				treeID++;
				DirsCount[i] = dirID + 1;
				FilesCount[i] = fileID;
				dirID = 0;
				fileID = 0;
			}
			printf("Ierarchy done.\n");

			//after creating the hierarchy, we create a list of the directories and a list of the files
			//included in the hierarchy, in order to create the metadata structure

			Dirs*  Directories = NULL;
			printf("Listing directories...\n");
			Directories = (Dirs*)listDirectories(DirsCount, numOfFiles, Ierarchy);
			if (Directories != NULL) printf("Directories done!\n");
			else printf("Directories failed.\n");
			
			Files* FilesList = NULL;
			printf("Listing files...\n");
			FilesList = (Files*)listFiles(FilesCount, DirsCount, numOfFiles, Ierarchy);
			if (FilesList != NULL) printf("Files done!\n");
			else printf("Files failed.\n");
			int zip = 0;
			if (choice2 == 'j') {
				zip = 1;
				Files* tempFile = FilesList;
				while (tempFile != NULL) {
					char filename[49];
					pid_t pid = fork();
					if (pid < 0) {
						perror("fork");
						return 1;
					} else if (pid == 0) {
						strcpy(filename, tempFile->filename);
						strcat(filename, ".tar");
						if (execlp("tar","tar","-cvf",filename, tempFile->filename, NULL) == -1) {
							perror("execlp failed");
							return 1;
						}
					}
					wait(NULL);
					tempFile = tempFile->next;
				}
				printf("Tar done.\n");
			}

			//count the total number of directories and files
			int fCount = 0, dCount = 0;
			for (i = 0; i < numOfFiles; ++i) {
				fCount += FilesCount[i];
				dCount += DirsCount[i];
			}

			//edit the header structure
			BS->numOfFiles = fCount;
			BS->numOfDirs = dCount;
			BS->numOfBlocks = 0;

			//get the metadata of each file and directory
			//for directories
			printf("Creating directories metadata...\n");
			createDirsMetadata(Directories);
			printf("Done!\n");
			//for files
			printf("Creating files metadata...\n");
			createFileMetadata(FilesList);
			printf("Done!\n");
			//write everything to the .di file
			printf("Writing everything to blocks...\n");
			strcat(archiveFileName, ".di");
			if ((fileOutDesc = fopen(archiveFileName, "w")) == NULL) {
				perror("Open archive-file");
			}
			writeMetadata(fileOutDesc, Directories, FilesList, BS, zip);
			fclose(fileOutDesc);
			printf("Done!\n");
			printf("Flag -c finished.\n");
		}
	}

	if (choice == 'x') {
		FILE* fileInDesc;
		fileInDesc = fopen(archiveFileName, "r");
		readFromBlocks(fileInDesc, 1, 0);
		fclose(fileInDesc);
	}

	if (choice == 'm') {
		FILE* fileInDesc;
		fileInDesc = fopen(archiveFileName, "r");
		readFromBlocks(fileInDesc, 0, 1);
		fclose(fileInDesc);
	}

	if (choice == 'p') {
		FILE* fileInDesc;
		fileInDesc = fopen(archiveFileName, "r");
		readFromBlocks(fileInDesc, 0, 2);
		fclose(fileInDesc);
	}

	if (choice == 'q') {
		FILE* fileInDesc;
		int numOfFiles = 0;
		char listOfFiles[argc - 3][49];
		if (flag == 3) {
			numOfFiles = argc - 4;
			int offset = 0;
			for (i = 4; i < argc; ++i) {
				strcpy(listOfFiles[offset], argv[i]);
				offset++;
			}
		}
		else {
			numOfFiles = argc - 3;
			int offset = 0;
			for (i = 3; i < argc; ++i) {
				strcpy(listOfFiles[offset], argv[i]);
				offset++;
			}
		}
		
		fileInDesc = fopen(archiveFileName, "r");
		int i, j;
		BlockStats* BS = malloc(sizeof(BlockStats));
		void *blockPtr1 = malloc(sizeof(int));	
		void *blockPtr2 = malloc(49*sizeof(char));
		void *blockPtr3 = malloc(10*sizeof(char));
		void* blockPtr4 = malloc(BLOCK_SIZE*sizeof(char));

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
		fclose(fileInDesc);

		int* queryResults = calloc(numOfFiles, sizeof(int));
		char queryArray[BS->numOfFiles + BS->numOfDirs][301];
		j = 0;
		Files* itterator1 = retrievedFiles;
		while (itterator1 != NULL) {
			strcpy(queryArray[j], itterator1->filename);
			j++;
			itterator1 = itterator1->next;
		}
		Dirs* itterator2 = retrievedDirectories;
		
		while (itterator2 != NULL) {
			strcpy(queryArray[j], itterator2->dirname);
			j++;
			itterator2 = itterator2->next;
		}
		int sum = BS->numOfFiles + BS->numOfDirs;
		
		for (i = 0; i < numOfFiles; ++i) {
			for (j = 0; j < sum; j++) {
				if (strcmp(queryArray[j], listOfFiles[i]) == 0) {
					queryResults[i] = 1;
					break;
				}
			}
		}

		for (i = 0; i < numOfFiles; ++i) {
			if (queryResults[i] == 1) {
				printf("%s exists.\n", listOfFiles[i]);
			}
			else {
				printf("%s does not exist.\n", listOfFiles[i]);
			}
		}
		free(queryResults);
	}
	return 0;
}
