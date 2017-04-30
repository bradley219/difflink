#ifndef _DIFFLINK_SOURCE_
#define _DIFFLINK_SOURCE_

#include "difflink.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <assert.h>

//#define ENABLE_GLIB_HASH_TABLES

// Create a file list
dirlist_t *init_list(void)
{
	dirlist_t *list = malloc( sizeof(dirlist_t) );
	list->length = 0;
	list->list = NULL;
	list->listsize = 0;

#ifdef ENABLE_GLIB_HASH_TABLES
	list->ht = g_hash_table_new( g_str_hash, g_str_equal );
#endif
	return list;
}
// Free a file list
void free_list( dirlist_t *list )
{
#ifdef ENABLE_GLIB_HASH_TABLES
	g_hash_table_remove_all(list->ht);
#endif
	direntry_t **lp = list->list;
	for( int i = 0; i < list->length; i++ )
	{
		free( *lp++ );
	}

	free( list->list );
	free( list );
	return;
}
// Add a file's dirent and stat structs to a list
int add_to_list( dirlist_t *list, struct dirent *dir, struct stat *stat )
{
	(list->length)++;
	if( list->listsize < list->length )
	{
		list->listsize = list->listsize + REALLOC_CHUNK_SIZE;
		list->list = realloc( list->list, list->listsize * (sizeof(direntry_t*)) );
	}

	direntry_t *direntry = malloc(sizeof(direntry_t));
	memcpy( &direntry->dir, dir, sizeof(struct dirent) );
	memcpy( &direntry->stat, stat, sizeof(struct stat) );
	
	int index = list->length - 1;
	list->list[index] = direntry;

	// Add the direntry_t pointer to the hash table
#ifdef ENABLE_GLIB_HASH_TABLES
	g_hash_table_insert( list->ht, dir->d_name, direntry );
#endif

	return index;
}
// Check if a file is in the file list
direntry_t *is_in_list( dirlist_t *list, char *filename ) 
{
	direntry_t *retval = NULL;

#ifdef ENABLE_GLIB_HASH_TABLES
	retval = g_hash_table_lookup( list->ht, filename );
#else
	direntry_t **lp = list->list;
	for( int i = 0; i < list->length; i++ )
	{
		direntry_t *dp = *lp++;
		if( strcmp( filename, dp->dir.d_name ) == 0 )
		{
			retval = dp;
			break;
		}
	}
#endif

	return retval;
}


int filediff( char *file1, char *file2 )
{
	int retval = 0;

	char filebuf1[FILE_CHUNK_SIZE];
	char filebuf2[FILE_CHUNK_SIZE];

	FILE *fh1 = NULL;
	FILE *fh2 = NULL;

	if( (fh1=fopen(file1,"r")) && (fh2=fopen(file2,"r")) )
	{
		// Get file sizes
		fseek( fh1, 0L, SEEK_END );
		fseek( fh2, 0L, SEEK_END );
		int filesize1 = ftell(fh1);
		int filesize2 = ftell(fh2);

		// Rewind
		fseek(fh1, 0L, SEEK_SET );
		fseek(fh2, 0L, SEEK_SET );

		int offset = 0;
		assert(filesize1==filesize2);
		if( filesize1 == filesize2 )
		{
			while( offset < filesize1 )
			{

				size_t s1, s2;
				s1 = fread( filebuf1, 1, FILE_CHUNK_SIZE, fh1 );
				s2 = fread( filebuf2, 1, FILE_CHUNK_SIZE, fh2 );
				assert( s1 == s2 );
				int diff = memcmp( filebuf1, filebuf2, s1 );
				if( diff != 0 )
				{
					retval = diff;
					break;
				}
				offset += s1;
			}
		}
		else
		{
			retval = -2;
		}

		fclose(fh1);
		fclose(fh2);
	}
	else
	{
		fprintf( stderr, "Failed opening files\n" );
		retval = -1;
	}

	return retval;
}

int parse_dirs( char *dirname1, char *dirname2 )
{
	DIR *d1,*d2;
	struct dirent *dir1, *dir2;
	struct stat stat;

	char fullpath1[MAXPATHLEN];
	char fullpath2[MAXPATHLEN];

	d1 = opendir( dirname1 );
	d2 = opendir( dirname2 );

	if( d1 && d2 )
	{
		// Loop through first directory, create list
		dirlist_t *list1 = init_list();

		while( ( dir1 = readdir(d1) ) ) 
		{
			if( strcmp( ".", dir1->d_name ) == 0 || strcmp( "..", dir1->d_name ) == 0 )
				continue;
			sprintf( fullpath1, "%s/%s", dirname1, dir1->d_name );
			lstat( fullpath1, &stat );
			add_to_list( list1, dir1, &stat );

		}
		// Loop through second directory, compare to list
		while( ( dir2 = readdir(d2) ) ) 
		{
			if( strcmp( ".", dir2->d_name ) == 0 || strcmp( "..", dir2->d_name ) == 0 )
				continue;

			direntry_t *direntry = NULL;
			if( ( direntry = is_in_list( list1, dir2->d_name ) ) )
			{
				sprintf( fullpath2, "%s/%s", dirname2, dir2->d_name );
				sprintf( fullpath1, "%s/%s", dirname1, direntry->dir.d_name );
				lstat( fullpath2, &stat );

				//fprintf( stderr, "Comparing:\n%s\n%s\n", fullpath1, fullpath2 );
				//fprintf( stderr, "%s\n%s\n", dir2->d_name, direntry->dir.d_name );
						
				// If both are regular files
				if( S_ISREG(stat.st_mode) && S_ISREG(direntry->stat.st_mode) )
				{
					if( stat.st_ino != direntry->stat.st_ino )
					{
						if( stat.st_size == direntry->stat.st_size )
						{
							//fprintf( stderr, "size2: %ld size1: %ld\n", stat.st_size, direntry->stat.st_size );
							if( filediff( fullpath1, fullpath2 ) == 0 )
							{
								//fprintf( stderr, "%s and %s are identical\n", fullpath1, fullpath2 );
								//fprintf( stderr, "Deleting `%s'\n", fullpath1 );
								if( unlink( fullpath1 ) )
								{
									perror( "unlink" );
									fprintf( stderr, "Unlinking failed\n" );
								}
								
								fprintf( stderr, "Linking:\n`%s'\n`%s'\n", fullpath1, fullpath2 );
								if( link( fullpath2, fullpath1 ) )
								{
									perror( "link" );
									fprintf( stderr, "Linking failed\n" );
								}
							}
						}
					}
				}

				// If both are directories
				if( S_ISDIR(stat.st_mode) && S_ISDIR(direntry->stat.st_mode) )
				{
					sprintf( fullpath1, "%s/%s", dirname1, direntry->dir.d_name );

					// recurse
					parse_dirs( fullpath1, fullpath2 );
				}
			}
		}

		free_list( list1 );
	}
	else
	{
		fprintf( stderr, "Could not open %s and %s\n", dirname1, dirname2 );
	}
	closedir(d1);
	closedir(d2);
	return 0;
}

// Compares dir1 to dir2 and links identical files in dir1 to files in dir2
int main( int argc, char *argv[] ) 
{
	char *dirname1 = argv[1];
	char *dirname2 = argv[2];

	parse_dirs( dirname1, dirname2 );

	return 0;
}
#endif
