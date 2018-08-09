/*
 *	Author:		Dknight
 *	Date:		2018-07-25
 *	Desrciption:    My very primitive shell.
 *	Options:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "mysh.h"

int 
main(int argc, char *argv[])
{
	char *cmdl;
	char *parsed_args[CMDMAX];
	char *parsed_args_piped[CMDMAX];
	char *prog = argv[0];
	int execflg;

	while(1) {
		cmd_invite();

		if (take_command(cmdl)) {
			continue;
		}

		if (has_redirect(cmdl)) {
			handle_redirect(cmdl);
		}

		execflg = parse_command(cmdl, parsed_args);
		if (execflg == 1) {
			exec_command(parsed_args);
		}
		if (execflg == 2) {
			exec_command_piped(parsed_args);
		}
	}
	exit(EXIT_SUCCESS);
}

/*
 * Print welcome message
 */
void 
welcome_msg()
{
	fprintf(stdout, "Welome to your Doom!\n");
	return;
}

/*
 * Print invitaion message.
 */
void 
cmd_invite()
{
	fprintf(stdout, "mysh:%s$ ", getenv("PWD"));
	return;
}

/*
 * Reads input command.
 */
int 
take_command(char *str)
{
	char c;
	int i = 0;
	char buf[CMDBUFSIZ];

	while ((c = getc(stdin)) != '\n') {
		buf[i] = c;
		++i;
	}
	buf[i] = '\0';

	if (strlen(buf) != 0 && buf[0] != '\0') {
		strcpy(str, buf);
		strcpy(history[histno++], buf);
		/*if history is higher than stack's size write history at the*/
		/*beginning*/
		if (histno > HISTMAX) {
			histno = 0;
		}
		return 0;
	} else {
		return 1;
	}
}

/*
 * Searches the pipes.
 */
int 
parse_pipe(char *str, char **striped)
{
	int i = 0;
	int piped = 0;
	char *token;

	token = strtok(str, "|");
	while (token != NULL && i < PIPEMAX) {
		token = trim_spaces(token);
		striped[i++] = token;
		token = strtok(NULL, "|");
	}

	/*If second isn't defined then it means that string hasn't pipe*/
	if (i > 1) {
		piped = 1;
	}
	return piped;
}

/*
 * Parses the input command.
 */
int 
parse_command(char *str, char **parsed)
{
	char *striped[PIPEMAX];
	int piped = 0;

	piped = parse_pipe(str, parsed);
	if (exec_command_my(parsed[0])) {
		return 0;
	} else {
		return 1 + piped;
	}
}

/*
 * Executes system command.
 */
void 
exec_command(char **parsed)
{
	int pipefd[1];
	int status;

	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Cannot fork child");
		return;
	} else if (pid == 0) {
		if (system(parsed[0]) < 0) {
			fprintf(stderr, "Cannot exec command");
		}
		exit(EXIT_SUCCESS);
	} else {
		wait(&status);
		return;
	}
}

/*
 * Execute piped command.
 */
void 
exec_command_piped(char **parsed)
{
	int pipefd[PIPEMAX];
	int status;
	char *p1_args[CMDBUFSIZ], *p2_args[CMDBUFSIZ];
	int testdup2;
	pid_t pid, tpid;

	if (pipe(pipefd) < 0) {
		fprintf(stderr, "\nCannot do pipe");
	}

	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Couldn't fork");
		return;
	}
	if (pid == (pid_t) 0) {
		tpid = fork();
		if (tpid == (pid_t) 0) {
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			close(pipefd[1]);

			parse_spaces(parsed[1], p1_args);
			if (execvp(p1_args[0], p1_args) < 0) {
				fprintf(stderr, "\nCannot execute command 1");
				exit(EXIT_FAILURE);
			}
		} else {
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
			close(pipefd[1]);

			parse_spaces(parsed[0], p2_args);
			if (execvp(p2_args[0], p2_args) < 0) {
				fprintf(stderr, "\nCannot execute command 2");
				exit(EXIT_FAILURE);
			}
		}
	}
	close(pipefd[0]);
	close(pipefd[1]);

	wait(&status);
	wait(&status);
}

/*
 * Removes leading and trailing spaces for string.
 * Honorably copied from:
 * https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 */
char 
*trim_spaces(char *str)
{
	char *end;
	/*Trim leading space*/
	while(isspace((unsigned char)*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	/*Trim trailing space*/
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;

	/*Write new null terminator character*/
	end[1] = '\0';

	return str;
}

/*
 * Executes user's defined command
 */
int 
exec_command_my(char *cmd)
{
	int i, pid;
	/*-1 means nothing to exec*/
	int to_exec = -1;
	int cmd_num = 4;
	char *cmds[cmd_num];

	cmds[0] = "help";
	cmds[1] = "hello";
	cmds[2] = "bye";
	cmds[3] = "history";

	for (i = 0; i < cmd_num; ++i) {
		if (strcmp(cmd, cmds[i]) == 0) {
			to_exec = i;
			break;
		}
	}

	switch (to_exec) {
		case 0:
			help();
			return 1;
		case 1:
			hello();
			return 1;
		case 2:
			bye();
			exit(EXIT_SUCCESS);
		case 3:
			show_history();
			return 1;
		default:
			break;
	}
	return 0;
}

/*
 * Parsing command arguments
 */
void 
parse_spaces(char* str, char** parsed)
{
	char *sep = " ";
	for (int i = 0; i < CMDMAX; i++) {
		explode(str, sep, parsed);
		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

/*
 * Gets stream redirect filename if existx
 */
char 
*redirect_file_name(char *cmd)
{
	return strchr(cmd, '>');
}

/*
 * if has redirect stream
 */
int 
has_redirect(char *cmd)
{
	char *fname = redirect_file_name(cmd);
	return (fname == NULL) ? 0 : 1;
}

/*
 * Redirect streaem from in to out.
 */
int 
do_redirect(int in, int out)
{
	if (dup2(in, out) < 0) {
		fprintf(stderr, "Cannot do redirect");
		return 1;
	}
	return 0;
}

/*
 * process redirect procedure.
 */
void 
handle_redirect(char *cmd) {
	
	int ofile, omod, oflags;
	char *fname;

	oflags = O_WRONLY | O_TRUNC | O_CREAT;
	omod = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	fname = redirect_file_name(cmd);
	if (fname != NULL) {
		/*+1 to remove the '>' char*/
		fname = trim_spaces(++fname);
		ofile = open(fname, oflags, omod);
		do_redirect(STDOUT_FILENO, ofile);
		close(ofile);
	} else {
		fprintf(stderr, "Cannot write to redirect file\n");
		exit(EXIT_FAILURE);
	}
}

int
explode(char *str, char *repl, char **strings)
{
  int k = 0;
  char *token = strtok(str, repl);
  while (token != NULL) {
    *strings++ = token;
    token = strtok(NULL, " ");
    ++k;
  }
  return k;
}
