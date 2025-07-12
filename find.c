#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_DIR_LENGTH 512


void recursive_search(char* folder)
{
	DIR* dir;
	if((dir = opendir(folder)) == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}
	
	struct dirent* d;
	errno = 0;
	while((d = readdir(dir)) != NULL)
	{
		if(d == NULL && errno != 0)
		{
			perror("readdir");
			exit(EXIT_FAILURE);
		}
		
		char filename[MAX_DIR_LENGTH];
		snprintf(filename, MAX_DIR_LENGTH, "%s", folder);
		strcat(filename, d->d_name);
		
		//ignore . and ..
		if((d->d_name[0] == '.' && d->d_name[1] == '\0') || (d->d_name[0] == '.' && d->d_name[1] == '.'))
		{
			continue;
		}
		
		printf("%s\n", filename);
		
		struct stat statbuf;
		if(stat(filename, &statbuf) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}
		
		if(S_ISDIR(statbuf.st_mode))
		{
			strcat(filename, "/");
			recursive_search(filename);
		}
	}
	
	if(closedir(dir) == -1)
	{
		perror("closedir");
		exit(EXIT_FAILURE);
	}
}


int main(int argc, char* argv[])
{
	char folder[MAX_DIR_LENGTH];
	if(argc == 1)
	{
		snprintf(folder, MAX_DIR_LENGTH, "%s", "./");
	}
	else if(argc == 2)
	{
		snprintf(folder, MAX_DIR_LENGTH, "%s", argv[1]);
	}
	else 
	{
		fprintf(stderr, "Usage: ./myfind [dir]\n");
		exit(EXIT_FAILURE);
	}
	
	//Add a trailing slash if there is not already
	for(int i = 0; folder[i] != '\0'; i++)
	{
		if(folder[i+1] == '\0')
		{
			if(folder[i] != '/')
			{
				folder[i+1] = '/';
				folder[i+2] = '\0';
			}
		}
	}
	
	recursive_search(folder);

	return 0;
}
