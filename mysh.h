#define CMDBUFSIZ 1024
#define CMDMAX    64
#define PIPEMAX   2 /* support right now only 2 pipes */
#define HISTMAX   256 /* maximum history stack */

/* do i need this? */
#define clear_term() fprintf(stdout, "\e[1;1H\e[2J")

/* history is global variable */
char history[HISTMAX][CMDBUFSIZ];
int histno = 0;

/*Functions prototypes*/

void welcome_msg();
void cmd_invite();
int take_command(char *);
int parse_command(char *, char **);
int parse_pipe(char *, char **);
void exec_command(char **);
void exec_command_piped(char **);
char *trim_spaces(char *);
int exec_command_my(char *);
char *getdir();
void parse_spaces(char*, char**);
int has_redirect(char *);
char *redirect_file_name(char *);
int do_redirect(int in, int out);
void handle_redirect(char *);
int explode(char *, char *, char **);

/* User defined commands's protypes */
void bye();
void help();
void hello();
void show_history();

/* User defined commands */

void
bye()
{
	fprintf(stdout, "\n%s\n", "Goobye!");
	return;
}

void
help()
{
	printf("=====================================================\n");
	printf("mysh - my shell.\n\n");
	printf("Copyright 2018 Dknight <smirnov.dmitri@gmail.com>\n\n");
	printf("The MIT License\n\n");
	printf("This is my primitive shell. It has a lot limitations like:");
	printf("\n- Support only 2 pipes.\n");
	printf("- Stream redirection to append file isn't suppoerted (>>)");
	printf("- This is very very basic version with bugs.");
	printf("\n\nNB! USE IT AT OWN RISK!");
	printf("\n\nHere are some special functions:");
	printf("\nhelp - prints this message");
	printf("\nhello - prints welcome message.");
	printf("\nbye - same as exit, exis from program.");
	printf("\nhistory - prints commands history");
	printf("\n\nlicense: you are free to do whatever you want with\n");
	printf("this software at own risk. This software is provided\n");
	printf("WITHOUT ANY WARRANTY.\n\n");
	printf("Author: xdknight <dmitri@mailbox.org>\n");
	printf("=====================================================\n");
	return;
}

void 
hello()
{
	char *user = getenv("USER");
	fprintf(stdout, "Hello, %s!\n", user);
	return;
}

void 
show_history()
{
	int i;

	for(i = 0; i < HISTMAX; ++i) {
		if (strlen(history[i]) == 0) {
			break;
		}
		fprintf(stdout, "%d. %s\n", i+1, history[i]);
	}
	return;

}
