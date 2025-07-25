#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>

#define CHK(op) do { if ((op) == -1) {perror(""); exit(EXIT_FAILURE);} } while (0)
#define NCHK(op) do { if ((op) == NULL) {perror(""); exit(EXIT_FAILURE);} } while (0)

#define READ_END  0
#define WRITE_END 1

#define DEFAULT_SIZE 10 


struct process
{
	bool redir_stdin;
	char* file_redir_stdin;
	bool redir_stdout;
	char* file_redir_stdout;
	bool is_job;
	bool is_cd;
	char* dir_cd;
	char** command_string; 
	int nb_command;
};

struct command
{
	struct process* p;
	int** pipes;
	int nb_pipe;
	int index_command;
	int index_process;
};


int main(/*int argc, char* argv[]*/)
{
	while(1)
	{
		char* cwd = getcwd(NULL, 0);
		char* login;
		NCHK(login = getlogin());
		char hostname[512];
		CHK(gethostname(hostname, 512));
		
		printf("%s@%s:%s$ ", login, hostname, cwd);
		CHK(fflush(stdout));
	
		char* line = NULL;
		size_t n = 0;
		if(getline(&line, &n, stdin) == -1)
		{
			perror("getline");
			free(line);
			exit(EXIT_FAILURE);
		}
		
		char* token = strtok(line, " \n");
		
		struct command c;
		c.index_command = 0;
		c.index_process = 0;
		c.nb_pipe = 0;
		NCHK(c.p = calloc(DEFAULT_SIZE, sizeof(struct process))); 
		for(int i = 0; i < DEFAULT_SIZE; ++i)
		{
			NCHK(c.p[i].command_string = calloc(DEFAULT_SIZE, sizeof(char*))); 
		}
		
		NCHK(c.pipes = calloc(DEFAULT_SIZE, sizeof(int*)));

		for(int i = 0; i < DEFAULT_SIZE; ++i)
		{
			NCHK(c.pipes[i] = calloc(2, sizeof(int))); 
		}
		
		for(int i = 0; i < DEFAULT_SIZE; ++i)
		{
			c.p[i].redir_stdin = false;
			c.p[i].file_redir_stdin = NULL;
			c.p[i].redir_stdout = false;
			c.p[i].file_redir_stdout = NULL;
			c.p[i].is_job = false;
			c.p[i].is_cd = false;
			c.p[i].dir_cd = NULL;
		}	
		
		int max_size_nb_command = DEFAULT_SIZE;
		int max_size_nb_process = DEFAULT_SIZE;
		int max_size_nb_pipe = DEFAULT_SIZE;

		while(token != NULL)
		{
			if(strcmp(token, "|") == 0)
			{
				c.p[c.index_process].command_string[c.index_command] = NULL;
				c.index_process += 1;
				c.nb_pipe += 1;
				c.p[c.index_process].nb_command = c.index_command;
				c.index_command = 0;
			}
			else if(strcmp(token, ">") == 0)
			{
				c.p[c.index_process].redir_stdout = true;
				c.p[c.index_process].file_redir_stdout = strtok(NULL, " \n"); //next token = filename
			}
			else if(strcmp(token, "<") == 0)
			{
				c.p[c.index_process].redir_stdin = true;
				c.p[c.index_process].file_redir_stdin = strtok(NULL, " \n"); //next token = filename
			}
			else if(strcmp(token, "&") == 0)
			{
				c.p[c.index_process].is_job = true;
			}
			else if(strchr(token, '&') != NULL)
			{
				c.p[c.index_process].is_job = true;
				token[strlen(token)-1] = '\0';
				c.p[c.index_process].command_string[c.index_command] = token;
				c.index_command += 1;
				c.p[c.index_process].nb_command = c.index_command;
			}
			else if(strcmp(token, "cd") == 0)
			{
				c.p[c.index_process].is_cd = true;
				c.p[c.index_process].dir_cd = strtok(NULL, " \n"); //next token = dir name
			}
			else
			{
				c.p[c.index_process].command_string[c.index_command] = token;
				c.index_command += 1;
				c.p[c.index_process].nb_command = c.index_command;
			}
			
			if(c.p[c.index_process].nb_command == max_size_nb_command)
			{
				max_size_nb_command *= 2;
				if((c.p[c.index_process].command_string = reallocarray(c.p[c.index_process].command_string, max_size_nb_command, sizeof(char*))) == NULL)
				{
					fprintf(stderr, "Error: reallocarray\n");
					exit(EXIT_FAILURE);
				}
			}
			
			if(c.index_process == DEFAULT_SIZE)
			{
				max_size_nb_process *= 2;
				if((c.p = reallocarray(c.p, max_size_nb_process, sizeof(struct process))) == NULL)
				{
					fprintf(stderr, "Error: reallocarray\n");
					exit(EXIT_FAILURE);
				}
			}
			
			if(c.nb_pipe == DEFAULT_SIZE)
			{
				max_size_nb_pipe *= 2;
				if((c.pipes = reallocarray(c.pipes, max_size_nb_pipe, sizeof(int*))) == NULL)
				{
					fprintf(stderr, "Error: reallocarray\n");
					exit(EXIT_FAILURE);
				}
				
				for(int i = 0; i < max_size_nb_pipe; ++i)
				{
					NCHK(c.pipes[i] = calloc(2, sizeof(int))); 
				}
			}
			
			token = strtok(NULL, " \n");
		}
		c.p[c.index_process].command_string[c.index_command] = NULL;
		
		for(int i = 0; i < c.nb_pipe; ++i)
		{
			NCHK(c.pipes[i] = calloc(1, 2 * sizeof(int)));
			CHK(pipe(c.pipes[i]));
		}
		
		pid_t pid;
		int reason;
		for(int i = 0; i <= c.index_process; ++i)
		{
			switch(pid = fork())
			{
				case -1:
					perror("fork");
					exit(EXIT_FAILURE);
					break;
				
				case 0: 
					if(c.nb_pipe > 0)
					{
						if(i == 0) //premier processus
						{
							CHK(close(c.pipes[i][READ_END]));
							CHK(close(STDOUT_FILENO));
							CHK(dup(c.pipes[i][WRITE_END]));
							CHK(close(c.pipes[i][WRITE_END]));
						}
						else if(i == c.index_process) //dernier processus
						{
							CHK(close(c.pipes[i-1][WRITE_END]));
							CHK(close(STDIN_FILENO));
							CHK(dup(c.pipes[i-1][READ_END]));
							CHK(close(c.pipes[i-1][READ_END]));
						}
						else //processus intermédiaire
						{
							CHK(close(c.pipes[i-1][WRITE_END]));
							CHK(close(STDIN_FILENO));
							CHK(dup(c.pipes[i-1][READ_END]));
							CHK(close(c.pipes[i-1][READ_END]));
							
							CHK(close(c.pipes[i][READ_END]));
							CHK(close(STDOUT_FILENO));
							CHK(dup(c.pipes[i][WRITE_END]));
							CHK(close(c.pipes[i][WRITE_END]));
						}
					}
					
					if(c.p[i].redir_stdin)
					{
						CHK(close(STDIN_FILENO));
						CHK(open(c.p[i].file_redir_stdin, O_RDONLY, 0666));
					}
					
					if(c.p[i].redir_stdout)
					{
						CHK(close(STDOUT_FILENO));
						CHK(open(c.p[i].file_redir_stdout, O_WRONLY|O_CREAT|O_TRUNC, 0666));
					}
					
					if(!c.p[i].is_cd)
					{
						CHK(execvp(c.p[i].command_string[0], c.p[i].command_string));
					}
					else 
					{
						free(line);
						free(cwd);
						exit(EXIT_SUCCESS);
					}
					break;
				
				default:
					if(c.nb_pipe > 0)
					{
						if(i > 0) //fermer le tube "précédent"
						{
							CHK(close(c.pipes[i-1][READ_END]));
							CHK(close(c.pipes[i-1][WRITE_END]));
						}
					}
					
					if(c.p[i].is_cd)
					{
						CHK(chdir(c.p[i].dir_cd));
					}
					
					if(!c.p[i].is_job)
					{
						CHK(wait(&reason));
					}
					break;
			}
		}
		
		for(int i = 0; i < max_size_nb_pipe; ++i)
		{
			free(c.pipes[i]);
		}
		
		free(c.pipes);
		
		for(int i = 0; i < max_size_nb_command; ++i) 
		{
			free(c.p[i].command_string);
		}
		
		free(c.p);
		free(line);
		free(cwd);
	}
	return 0;
}
