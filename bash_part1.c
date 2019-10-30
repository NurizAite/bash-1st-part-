#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
char *get_word (char *end) {
	char *wrd = NULL;
	int i = 0;
	*end = getchar();
	while (*end != ' ' && *end != '\t' && *end != '\n') {
		wrd = realloc(wrd, (i + 1) * sizeof(char));
		wrd[i] = *end;
		i++;
		*end = getchar();
	}
	wrd[i] = '\0';
	return wrd;
}

char **get_list () {
	char **list = NULL, last_sym = '\0';
	int i = 0;
	do {
		list = realloc(list, (i + 1) * sizeof(char *));
		list[i] = get_word(&last_sym);
		i++;
	}
	while (last_sym != '\n');
	list = realloc(list, (i + 1) * sizeof(char *));
	list[i] = NULL;
	return list;
}

void clear_list(char **list) {
    if (list) {
        for (int i = 0; list[i]; i++)
            free(list[i]);
        free(list);
    }
}

int sym_search (char **list, char *sym_ptr) {
	if (list) {
		for (int i = 0; list[i]; i++) {
			if (!strcmp(list[i], ">")) {
				*sym_ptr = '>';
				return i;
			}
			if (!strcmp(list[i], "<")) {
				*sym_ptr = '<';
				return i;
			}
		}
	}
	*sym_ptr = '\0';
	return 0;
}
int redir(char **list, char sym, int sym_pos) {
	char *tmp;
	int fd;
	if (sym == '>') {
            tmp = list[sym_pos];
            list[sym_pos] = NULL;
            fd = open(list[sym_pos + 1], O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR);
            if (fd < 0) {
                warnx("%s: file did not open\n", list[sym_pos + 1]);
            } else {
                if (fork() > 0) {
                    wait();
                } else {
                    dup2(fd, 1);
                    if (execvp(list[0], list) < 0) {
                        perror("exec trouble");
                        return 1;
                    }
                }
                close(fd);
            }
            list[sym_pos] = tmp;
        } else if (sym == '<') {
            tmp = list[sym_pos];
            list[sym_pos] = NULL;
            fd = open(list[sym_pos + 1], O_RDONLY);
            if (fd < 0) {
                warnx("%s: file did not open\n", list[sym_pos + 1]);
            } else {
                if (fork() > 0) {
                    wait();
                } else {
                    dup2(fd, 0);
                    if (execvp(list[0], list) < 0) {
                        perror("exec trouble");
                        return 1;
                    }
                }
                close(fd);
            }
            list[sym_pos] = tmp;
        }
}

int main (void) {
	char **list, *tmp, sym;
	int fd, sym_pos;
	int i = 0;
	list = get_list();
	sym_pos = sym_search(list, &sym);
	while (list && strcmp(list[0], "exit") && strcmp(list[0], "quit")) {
		if (!sym) {
			if (fork() > 0) {
				wait();
			} else {
				if (execvp(list[0], list) < 0) {
					perror("exec trouble");
					return 1;
				}
			}
		}
		else {
			redir(list, sym, sym_pos);
		}
		clear_list(list);
		list = get_list();
		sym_pos = sym_search(list, &sym);
	}
	clear_list(list);
	return 0;
}
