#ifndef CS_FILES_H
#define CS_FILES_H

#if defined(__APPLE__)
// assumes exe lives in Appname.app/Contents/MacOS
#    define DOCS_FOLDER "../../../documents/"
#else
#    define DOCS_FOLDER "./documents/"
#endif

#define FILE_EXT ".txt"
#define FILE_EXT_LEN 4
#define MAX_FILE_CHARS (CHARS_PER_LINE - FILE_EXT_LEN)


typedef struct files_tag {
	char ** names;
	int len;
} files_t;


files_t*
Files_Populate();

void
Files_CheckDocDir();

int
Files_Exists(char * filename);

char *
Files_GetAbsPath(char * filename);

void
Files_Destroy(files_t * files);

#endif
