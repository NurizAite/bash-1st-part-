#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
char *get_word (char *end) {
	char *buf;
	char *init = *buf;
	a = read(0, &buf, 1);
	while (a > 0 && a != ' ' && a != '\t' && a != '\n') {
		write(1, &buff, 1);
	}
	*end = *buf;
	buf = '\0';
	return *init;
}

char **get_list () {
	char *buff;
	char **arr;
	char *last_sym;
	int i = 0;
	arr = (char **)malloc(sizeof(char *);
	do {
		arr = (char *)malloc(sizeof(char *));
		buff = get_word(*last_sym);
		arr[i] = buff;
		i++;
	}
	while (arr[i - 1] != NULL)
	return **arr;
}

int run () {
	char *cmd;
	char **arr = get_list();
	while (arr[i] != NULL) {
		cmd = arr[i];
        	cmd[strlen(cmd) - 1] = '\0';
        	if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit")) {
			return 0;
		}
		pid_t pid = fork();
		if (pid == 0) {
			if (execlp(cmd, cmd, NULL) < 0) {
				err(STDOUT_FILENO, NULL);
				return 1;
			}
			return 0;
		}
	}
}
int main () {
	run();
	return 0;
}
