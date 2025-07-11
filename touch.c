#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_FILE_LENGTH 124


int main(int argc, char* argv[])
{
	bool opt_a = false, opt_c = false, opt_m = false, opt_r = false;

	char file_to_copy[MAX_FILE_LENGTH];

	int opt;
	while((opt = getopt(argc, argv, "acmr:")) != -1)
	{
		switch(opt)
		{
			case 'a':
				opt_a = true;
				break;
				
			case 'c':
				opt_c = true;
				break;
			
			case 'm':
				opt_m = true;
				break;
			
			case 'r':
				opt_r = true;
				
				snprintf(file_to_copy, MAX_FILE_LENGTH, "%s", optarg);
				break;
		
			case ':':
				fprintf(stderr, "Missing argument\n");
				break;
				
			case '?':
				fprintf(stderr, "Not in optstring\n");
				break;
		
			default:
				fprintf(stderr, "Unknown option\n");
				break;
		}
	}
	
	char file[MAX_FILE_LENGTH];
	if(optind == argc - 1)
	{
		snprintf(file, MAX_FILE_LENGTH, "%s", argv[optind]);
	}
	else 
	{
		fprintf(stderr, "Error: wrong argument number... Usage: ./my_touch [-acm -r FILE] [FILE]\n");
		exit(EXIT_FAILURE);
	}
	
	
	if(opt_c)
	{
		if(access(file, F_OK) == -1)
		{
			fprintf(stderr, "Error: the file '%s' doesn't exist!\n", file);
			exit(EXIT_FAILURE);
		}
		
	}
	
	int fd;
	if((fd = open(file, O_CREAT|O_RDONLY, 0666)) == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	
	struct stat statbuf;
	if(stat(file, &statbuf) == -1)
	{
		perror("stat");
		exit(EXIT_FAILURE);
	}
	
	int fd_to_copy;
	struct stat statbuf_to_copy;
	if(opt_r)
	{
		if(access(file_to_copy, F_OK) == -1)
		{
			fprintf(stderr, "Error: the file '%s' doesn't exist!\n", file_to_copy);
			exit(EXIT_FAILURE);
		}
	
		if((fd_to_copy = open(file_to_copy, O_CREAT|O_RDONLY, 0666)) == -1)
		{
			perror("open");
			exit(EXIT_FAILURE);
		}
		
		if(stat(file_to_copy, &statbuf_to_copy) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}
		
		if(close(fd_to_copy) == -1)
		{
			perror("close");
			exit(EXIT_FAILURE);
		}
	}
	
	struct utimbuf u = {.actime = statbuf.st_atime, .modtime = statbuf.st_mtime};
	
	if(opt_a || opt_r)
	{
		if(opt_r)
		{
			u.actime = statbuf_to_copy.st_atime;
		}
		else
		{
			if((u.actime = time(NULL)) == ((time_t) -1))
			{
				perror("time");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	if(opt_m || opt_r)
	{
		if(opt_r)
		{
			u.modtime = statbuf_to_copy.st_mtime;
		}
		else
		{
			if((u.modtime = time(NULL)) == ((time_t) -1))
			{
				perror("time");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	if(!opt_a && !opt_m && !opt_r)
	{
		if((u.actime = time(NULL)) == ((time_t) -1))
		{
			perror("time");
			exit(EXIT_FAILURE);
		}
			
		if((u.modtime = time(NULL)) == ((time_t) -1))
		{
			perror("time");
			exit(EXIT_FAILURE);
		}
	}
	
	
	if(utime(file, &u) == -1)
	{
		perror("utime");
		exit(EXIT_FAILURE);
	}
	
	if(close(fd) == -1)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
	
	return 0;
}
