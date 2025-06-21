#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <sys/sysmacros.h>

#define MAX_DIR_LENGTH 124


void handle_file_type(mode_t mode, char mode_string[])
{
	switch(mode & S_IFMT)
	{
		case S_IFSOCK:
			mode_string[0] = 's';
			break;
			
		case S_IFLNK:
			mode_string[0] = 'l';
			break;
			
		case S_IFREG:
			mode_string[0] = '-';
			break;
			
		case S_IFBLK:
			mode_string[0] = 'b';
			break;
			
		case S_IFDIR:
			mode_string[0] = 'd';
			break;
			
		case S_IFCHR:
			mode_string[0] = 'c';
			break;
			
		case S_IFIFO:
			mode_string[0] = 'p';
			break;
	
		default:
			mode_string[0] = '-';
			break;
	}
}


void handle_file_permissions(mode_t mode, char mode_string[])
{
	if((mode & S_IRWXU) == S_IRWXU) //attention, le "==" est obligatoire. Sinon, si un ou deux des 3 bits sont positionnés, cette condition vaudra true (Exemple : 110 & 111 = 110 != 0 == true)
	{
		mode_string[1] = 'r';
		mode_string[2] = 'w';
		mode_string[3] = 'x';
	}
	else
	{
		if(mode & S_IRUSR)
			mode_string[1] = 'r';

		if(mode & S_IWUSR)
			mode_string[2] = 'w';

		if(mode & S_IXUSR)
			mode_string[3] = 'x';
	}
	
	
	if((mode & S_IRWXG) == S_IRWXG) 
	{
		mode_string[4] = 'r';
		mode_string[5] = 'w';
		mode_string[6] = 'x';
	}
	else
	{
		if(mode & S_IRGRP)
			mode_string[4] = 'r';

		if(mode & S_IWGRP)
			mode_string[5] = 'w';
			
		if(mode & S_IXGRP)
			mode_string[6] = 'x';
	}
	
	
	if((mode & S_IRWXO) == S_IRWXO)
	{
		mode_string[7] = 'r';
		mode_string[8] = 'w';
		mode_string[9] = 'x';
	}
	else
	{
		if(mode & S_IROTH)
			mode_string[7] = 'r';
			
		if(mode & S_IWOTH)
			mode_string[8] = 'w';

		if(mode & S_IXOTH)
			mode_string[9] = 'x';
	}
	
	if(mode & S_ISUID)
		mode_string[3] = 's';

	if(mode & S_ISGID)
		mode_string[6] = 's';

	if(mode & S_ISVTX)
		mode_string[9] = 't';
}

void handle_owner_name_group(uid_t st_uid, gid_t st_gid)
{
	errno = 0;
	struct passwd* passwd;
	passwd = getpwuid(st_uid);
	if(errno != 0)
	{
		perror("getpwuid");
		exit(1);
	}
	fprintf(stdout, "%s ", passwd->pw_name);
	
	errno = 0;
	struct group* group;
	group = getgrgid(st_gid);
	if(errno != 0)
	{
		perror("getgrgid");
		exit(1);
	}
	fprintf(stdout, "%s ", group->gr_name);
}


void handle_date(time_t st_mtim)
{
	struct tm *my_tm;
	if((my_tm = localtime(&st_mtim)) == NULL)
	{
		perror("localtime");
		exit(1);
	}
			
	if(my_tm->tm_mon == 0)
		fprintf(stdout, "%s ", "janv.");
	else if(my_tm->tm_mon == 1)
		fprintf(stdout, "%s ", "févr.");
	else if(my_tm->tm_mon == 2)
		fprintf(stdout, "%s ", "mars");
	else if(my_tm->tm_mon == 3)
		fprintf(stdout, "%s ", "avr.");
	else if(my_tm->tm_mon == 4)
		fprintf(stdout, "%s ", "mai");
	else if(my_tm->tm_mon == 5)
		fprintf(stdout, "%s ", "juin");
	else if(my_tm->tm_mon == 6)
		fprintf(stdout, "%s ", "juil.");
	else if(my_tm->tm_mon == 7)
		fprintf(stdout, "%s ", "août");
	else if(my_tm->tm_mon == 8)
		fprintf(stdout, "%s ", "sept.");
	else if(my_tm->tm_mon == 9)
		fprintf(stdout, "%s ", "oct.");
	else if(my_tm->tm_mon == 10)
		fprintf(stdout, "%s ", "nov.");
	else if(my_tm->tm_mon == 11)
		fprintf(stdout, "%s ", "déc.");
		
	fprintf(stdout, "%d ", my_tm->tm_mday);

	fprintf(stdout, "%d:%d ", my_tm->tm_hour, my_tm->tm_min);
}




int main(int argc, char* argv[])
{
	int opt;
	
	int flag_a = 0;
	int flag_i = 0;
	int flag_l = 0;
	int flag_s = 0;
	
	while((opt = getopt(argc, argv, "ails")) != -1)
	{
		switch(opt)
		{
			case 'a':
				flag_a = 1;
				break;
				
			case 'i':
				flag_i = 1;
				break;
				
			case 'l':
				flag_l = 1;
				break;
				
			case 's':
				flag_s = 1;
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
	
	char current_dir[MAX_DIR_LENGTH] = "./";
	
	if(optind == argc - 1) //one more argument = folder path
	{
		if(snprintf(current_dir, MAX_DIR_LENGTH * sizeof(char), "%s", argv[argc - 1]) < 0)
		{
			fprintf(stderr, "Error : snprintf\n");
			exit(1);
		}
	}
	else if(optind + 1 < argc) //too many arguments => error
	{
		fprintf(stderr, "Error: too many arguments... Usage: ./my_ls [-ails] [directory]\n");
	}
	
	DIR* dir = NULL;
	if((dir = opendir(current_dir)) == NULL)
	{
		perror("opendir");
		exit(1);
	}
	
	struct dirent* d;
	struct stat statbuf;
	
	errno = 0;
	while((d = readdir(dir)) != NULL)
	{
		if(errno != 0 && d == NULL)
		{
			perror("readdir");
			exit(1);
		}
		
		if(flag_a != 1 && d->d_name[0] == '.')
		{
			continue;
		}
		
		if(flag_i == 1 || flag_s == 1 || flag_l == 1)
		{
			size_t len = strlen(current_dir) + strlen(d->d_name) + 2; //2 * "\0"
			char* absolute_path;
			if((absolute_path = malloc(len)) == NULL)
			{
				fprintf(stderr, "Error : malloc\n");
				exit(1);
			}
			
			if(snprintf(absolute_path, len, "%s", current_dir) < 0)
			{
				fprintf(stderr, "Error : snprintf\n");
				exit(1);
			}
			
			if((absolute_path = strncat(absolute_path, d->d_name, len)) == NULL)
			{
				fprintf(stderr, "Error : strcat\n");
				exit(1);
			}
			
			printf("Path : %s\n", absolute_path);

			if(stat(absolute_path, &statbuf) == -1)
			{
				perror("stat");
				exit(EXIT_FAILURE);
			}
			
			free(absolute_path);
			
			if(flag_i == 1)
				fprintf(stdout, "%ld ", statbuf.st_ino);
			if(flag_s == 1)
				fprintf(stdout, "%ld ", statbuf.st_blocks);
			if(flag_l == 1)
			{
				mode_t mode = statbuf.st_mode;
				char mode_string[11] = "----------"; //10 letters + \0 = 11
				
				handle_file_type(mode, mode_string);
				handle_file_permissions(mode, mode_string);
				
				fprintf(stdout, "%s ", mode_string);
			}
			
			fprintf(stdout, "%ld ", statbuf.st_nlink); 
		
			handle_owner_name_group(statbuf.st_uid, statbuf.st_gid);
			
			if(S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
			{
				fprintf(stdout, "%u, %u ", major(statbuf.st_rdev), minor(statbuf.st_rdev));
			}
			else fprintf(stdout, "%ld ", statbuf.st_size);
			 
			handle_date(statbuf.st_mtime);
		}
		
		fprintf(stdout, "%s\n", d->d_name);
	}
	
	
	if(closedir(dir) == -1)
	{
		perror("closedir");
		exit(1);
	}
	
	return 0;
}
