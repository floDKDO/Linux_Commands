#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CHK(op) do { if ((op) == -1) {perror(""); exit(EXIT_FAILURE);} } while (0)
#define NCHK(op) do { if ((op) == NULL) {perror(""); exit(EXIT_FAILURE);} } while (0)


int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: mv [src_file] [dst_file]\n");
		exit(EXIT_FAILURE);
	}
	
	char* src = argv[1];
	char* dst = argv[2];
	
	struct stat statbuf_src;
	CHK(stat(src, &statbuf_src));
	
	if(access(dst, F_OK) == -1) 
	{
		if(S_ISREG(statbuf_src.st_mode))
		{
			CHK(link(src, dst));
			CHK(unlink(src));
		}
		else if(S_ISDIR(statbuf_src.st_mode))
		{
			CHK(rename(src, dst));
		}
	}
	else 
	{
		struct stat statbuf_dst
		CHK(stat(dst, &statbuf_dst));
	
		if((S_ISDIR(statbuf_src.st_mode) && S_ISDIR(statbuf_dst.st_mode))
		|| (S_ISREG(statbuf_src.st_mode) && S_ISDIR(statbuf_dst.st_mode)))
		{
			char* dest;
			NCHK(dest = strdup(dst));
			size_t len_dest = strlen(dest);
			if(dest[len_dest - 1] != '/')
			{
				dest[len_dest] = '/';
				dest[len_dest + 1] = '\0';
				len_dest += 1;
			}
			strcat(dest, src);
			CHK(rename(src, dest));
			free(dest);
		}
		else if(S_ISREG(statbuf_src.st_mode) && S_ISREG(statbuf_dst.st_mode))
		{
			CHK(rename(src, dst));
		}
	}
	return 0;
}
