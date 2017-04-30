#ifndef _DIFFLINK_H_
#define _DIFFLINK_H_

#include <config.h>
#include <dirent.h>
#include <sys/stat.h>
#include <glib.h>

#define FILE_CHUNK_SIZE 100000
#define REALLOC_CHUNK_SIZE 1000

typedef struct {
	struct dirent dir;
	struct stat stat;
} direntry_t;

typedef struct {
	int length;
	direntry_t **list;
	int listsize;
	GHashTable *ht;
} dirlist_t;

#endif
