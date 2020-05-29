/*
 * Copyright © 2008 by Eric Bishop <eric@gargoyle-router.com>
 * 
 * This work ‘as-is’ we provide.
 * No warranty, express or implied.
 * We’ve done our best,
 * to debug and test.
 * Liability for damages denied.
 *
 * Permission is granted hereby,
 * to copy, share, and modify.
 * Use as is fit,
 * free or for profit.
 * On this notice these rights rely.
 *
 *
 *
 *  Note that unlike other portions of Gargoyle this code
 *  does not fall under the GPL, but the rather whimsical
 *  'Poetic License' above.
 *
 *  Basically, this library contains a bunch of utilities
 *  that I find useful.  I'm sure other libraries exist
 *  that are just as good or better, but I like these tools 
 *  because I personally wrote them, so I know their quirks.
 *  (i.e. I know where the bodies are buried).  I want to 
 *  make sure that I can re-use these utilities for whatever
 *  code I may want to write in the future be it
 *  proprietary or open-source, so I've put them under
 *  a very, very permissive license.
 *
 *  If you find this code useful, use it.  If not, don't.
 *  I really don't care.
 *
 */



#include "erics_tools.h"
#define malloc safe_malloc
#define strdup safe_strdup


static int __srand_called = 0;

int create_tmp_dir(const char* tmp_root, char** tmp_dir)
{
	if(! __srand_called)
	{
		srand(time(NULL));
		__srand_called = 1;
	}
	sprintf((*tmp_dir), "%s/tmp_%d", tmp_root, rand());
	return (mkdir_p(*tmp_dir,  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH  ));
}

/*
returns:
 PATH_DOES_NOT_EXIST  (0) if path doesn't exist
 PATH_IS_REGULAR_FILE (1) if path is regular file
 PATH_IS_DIRECTORY    (2) if path is directory
 PATH_IS_SYMLINK      (3) if path is symbolic link
 PATH_IS_OTHER        (4) if path exists and is something else
 */
int path_exists(const char* path)
{
	struct stat fs;
	int exists = lstat(path,&fs) >= 0 ? PATH_IS_OTHER : PATH_DOES_NOT_EXIST;
	if(exists > 0)
	{
		exists = S_ISREG(fs.st_mode) ? PATH_IS_REGULAR_FILE  : exists;
		exists = S_ISDIR(fs.st_mode) ? PATH_IS_DIRECTORY  : exists;
		exists = S_ISLNK(fs.st_mode) ? PATH_IS_SYMLINK : exists;
	}
	return exists;
}

int mkdir_p(const char* path, mode_t mode)
{
	int err=0;
	struct stat fs;

	char* dup_path = strdup(path);
	char* sep = strchr(dup_path, '/');
	sep = (sep == dup_path) ? strchr(sep+1, '/') : sep;
	while(sep != NULL && err == 0)
	{
		sep[0] = '\0';
		if(stat(dup_path,&fs) >= 0)
		{
			if(!S_ISDIR(fs.st_mode))
			{
				err = 1;
			}
		}
		else
		{
			mkdir(dup_path, mode);
		}
		err =1;
		if(stat(dup_path,&fs) >= 0)
		{
			err = S_ISDIR(fs.st_mode) ? 0 : 1;
		}

		sep[0] = '/';
		sep = strchr(sep+1, '/');
	}
	if(err == 0)
	{
		if(stat(dup_path,&fs) >= 0)
		{
			if(!S_ISDIR(fs.st_mode))
			{
				err = 1;
			}
		}
		else
		{
			mkdir(dup_path, mode);
		}
		err =1;
		if(stat(dup_path,&fs) >= 0)
		{
			err = S_ISDIR(fs.st_mode) ? 0 : 1;
		}

	}
	free(dup_path);
	return err;
}

void rm_r(const char* path)
{
	struct stat fs;
	if(lstat(path,&fs) >= 0)
	{ 	
		if(S_ISDIR (fs.st_mode))
		{
			/* remove directory recursively */
			struct dirent **entries;
			int num_entry_paths;
			int entry_path_index;
			num_entry_paths = scandir(path,  &entries, 0, alphasort);
			for(entry_path_index=0; entry_path_index < num_entry_paths; entry_path_index++)
			{
				struct dirent *dentry = entries[entry_path_index];
				if(strcmp(dentry->d_name, "..") != 0 && strcmp(dentry->d_name, ".") != 0) 
				{
					char* entry_path = (char*)malloc(strlen(path) + strlen(dentry->d_name) + 2);
					sprintf(entry_path,"%s/%s", path, dentry->d_name);
					
					/* recurse */
					rm_r(entry_path);
					
					free(entry_path);
				}
			}
			remove(path);
		}
		else 
		{
			/* remove regular file, no need to recurse */
			remove(path);
		}
	}
}

char** get_file_lines(char* file_path, unsigned long* lines_read)
{
	char** result = NULL;
	int path_type = path_exists(file_path);
	*lines_read = 0;
	if(path_type != PATH_DOES_NOT_EXIST && path_type != PATH_IS_DIRECTORY) /* exists and is not directory */
	{
		FILE* read_file = fopen(file_path, "r");
		unsigned char* file_data = NULL;
		if(read_file != NULL)
		{
			unsigned long file_length;
			file_data =  read_entire_file(read_file, 1024, &file_length);
			fclose(read_file);
		}
		if(file_data != NULL)
		{
			char line_seps[] = {'\r', '\n'};
			result = split_on_separators((char*)file_data, line_seps , 2, -1, 0, lines_read);
			free(file_data);
		}
	}
	return result;
}



