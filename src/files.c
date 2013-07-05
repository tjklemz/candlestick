#include "files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#  include <dirent.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#elif defined(_WIN32)
#  include <windows.h>
#  include <io.h>
#endif

static
void
Files_Append(files_t * files, char * name)
{
	int len = strlen(name);
	char * filename = malloc(len + 1);
	strcpy(filename, name);

	files->names = (char**)realloc(files->names, (files->len + 1) * sizeof(char*));
	files->names[files->len] = filename;
	files->len += 1;
}


/**************************************************************************
 * PopulateFiles
 *
 * This only allows ".txt" extensions at the moment.
 * Creates the list of filenames (just the names, with extensions).
 *
 * Returns the list of files
 **************************************************************************/

files_t*
Files_Populate()
{
	files_t * files = (files_t*)malloc(sizeof(files_t));
	files->len = 0;
	files->names = 0;
	
	Files_CheckDocDir();
	
#if defined(__unix__) || defined(__APPLE__)
	{
		struct dirent * dp;
		DIR * dfd;
		
		dfd = opendir(DOCS_FOLDER);
		
		while((dp = readdir(dfd))) {
			struct stat st = {0};
			int file_len = strlen(dp->d_name);
			char * filename_full = malloc(strlen(dp->d_name) + strlen(DOCS_FOLDER) + 1);
			
			sprintf(filename_full, "%s%s", DOCS_FOLDER, dp->d_name);
			
			//only do something if the thing is an actual file and is .txt
			if(stat(filename_full, &st) != -1) {
				if(S_ISREG(st.st_mode & S_IFMT)) {

					int len = strlen(dp->d_name);

					if(len > 4 && !strcmp(&dp->d_name[len - 4], FILE_EXT)) {
						Files_Append(files, dp->d_name)
					}
				}
			}
		}
		
		closedir(dfd);
	}
#elif defined(_WIN32)
	{
		struct _finddata_t file_d;
		long hFile;

		char search[60];
		sprintf(search, "%s*%s", DOCS_FOLDER, FILE_EXT);

		hFile = _findfirst(search, &file_d);

		if(hFile != -1L) {
			do {
				Files_Append(files, file_d.name);
			} while(_findnext(hFile, &file_d) == 0);
		}

		_findclose(hFile);
	}
#endif
	
	return files;
}

// Doctor checkup. Ha ha.
void
Files_CheckDocDir()
{
#if defined(__unix__) || defined(__APPLE__)
	{
		struct stat st = {0};

		if(stat(DOCS_FOLDER, &st) == -1) {
			mkdir(DOCS_FOLDER, (S_IRWXU | S_IRWXG | S_IRWXO));
		}
	}
#elif defined(_WIN32)
	{
		CreateDirectory(DOCS_FOLDER, NULL);
	}
#endif
}


int
Files_Exists(char * filename)
{
	FILE * file;
	
    if((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}


/**************************************************************************
 * GetAbsPath
 *
 * Will return a newly allocated full_filename that has everything
 * needed to save or open (i.e. actual file access).
 *
 * Expects the current "filename" to be valid (and have an extension
 * if there is one).
 **************************************************************************/

char *
Files_GetAbsPath(char * filename)
{
	int filename_size;
	char * full_filename;

	filename_size = strlen(DOCS_FOLDER) + strlen(filename) + 1;
	full_filename = (char*)malloc(filename_size * sizeof(char));
	
	strcpy(full_filename, DOCS_FOLDER);
	strcat(full_filename, filename);
	
	return full_filename;
}


void
Files_Destroy(files_t * files)
{
	if(files) {
		int i;
		for(i = 0; i < files->len; ++i) {
			free(files->names[i]);
		}
		free(files->names);

		files->names = 0;
		files->len = 0;
	}

	files = 0;
}
