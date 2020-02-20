#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#define GREEN   "\x1b[32;1m"
#define BLUE    "\x1b[34;1m"
#define WHITE   "\x1b[0m"

int bg = 0;

void change_dir(char ***cmd, int n) {
    char dir[256];
//    printf("%s\n", getcwd(dir, 256));
//    printf("%s\n", dir);
//    printf("change dir\n");
/*    if (cmd[0][1] != NULL)
        printf("%s\n", cmd[0][1]);*/
    char *home = getenv("HOME");
    if (cmd[0][1] == NULL || strcmp(cmd[0][1] , "~") == 0) {
        chdir(home);
    } else {
        chdir(cmd[0][1]);
    }

}

void print() {
  const char* pwd = getenv("PWD");
  const char* user = getenv("USER");
  char host[256], dir[256];
  gethostname(host, 256);
  getcwd(dir, 256);
  printf(GREEN "%s@%s" WHITE ":" BLUE "%s" WHITE "$ ", user, host, dir);
  fflush(stdout);
}

int exec_cmd(char ***cmd, int n) {
    if (strcmp(cmd[0][0], "cd") == 0) {
        change_dir(cmd, n);
        if (cmd[0][2] == NULL)
            return 0;
    }
    if (n == 1) {
        no_pipes(cmd);
    }
    else
        pipes(cmd, n);
    if (bg) {
      n--;
      bg = 0;
    }
    return 0;
}

int no_pipes(char ***cmd) {
    int fd1, fd2;
    if (fork() == 0) {
        fd1 = redir(cmd[0]);
        fd2 = redir(cmd[0]);
        if (execvp(cmd[0][0], cmd[0]) < 0) {
                free_cmd(cmd);
                perror("exec failed");
                if (fd1 != 0 && fd1 != 1) {
                    close(fd1);
                }
                if (fd2 != 0 && fd2 != 1) {
                    close(fd2);
                }
                exit(1);
            }
        } else {
            if (!bg) {
                wait(NULL);
            }
            bg = 0;
            return 0;
        }

}

int pipes(char ***cmd, int n){
    int fd1, fd2;
    int pipefd[n - 1][2], pid;
    for (int i = 0; i < n; i++) {
        if (i != n - 1) {
            pipe(pipefd[i]);
        }
        if ((pid = fork()) == 0) {
            if (i == 0) {
                fd1 = redir(cmd[i]);
            } else if (i == n - 1) {
                fd2 = redir(cmd[i]);
            }
            if (i != 0) {
                dup2(pipefd[i - 1][0], 0);
            }
            if (i != n - 1) {
                dup2(pipefd[i][1], 1);
            }
            n--;
            for (int j = 0; j < i + 1; j++) {
                if (j == n) {
                    break;
                }
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }
            if (execvp(cmd[i][0], cmd[i]) < 0) {
                free_cmd(cmd);
                perror("exec failed");
                      close(fd1);
                      close(fd2);
                exit(1);
            }
        }
    }
    for (int i = 0; i < n; i++) {
      if (i != n - 1) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
      }
      wait(NULL);
    }
    if (fd1 != 0 && fd1 != 1) {
        close(fd1);
    }
    if (fd2 != 0 && fd2 != 1) {
        close(fd2);
    }

}

char *get_word(char *end) {
    char *word = NULL;
    int i = 0, flag = 0;
    if (*end == '&') {
        bg = 1;
        read(0, end, 1);
        return word;
    }
    if (*end == '|') {
      return word;
    }
    if (*end == '>' || *end == '<') {
        word = (char *)realloc(word, 2);
        if (word == NULL) {
            perror("realloc");
            return NULL;
        }
        word[0] = *end;
        word[1] = '\0';
        if (read(0, end, 1) < 0) {
            perror("read");
            free(word);
            return NULL;
        }
        return word;
    }
    if (*end == '"') {
            flag = 1;
            if (read(0, end, 1) < 0) {
                perror("read");
                free(word);
                return NULL;
            }
            if (*end == '"') {
                word = (char *)realloc(word, sizeof(char));
                if (word == NULL) {
                    perror("realloc");
                    return NULL;
                }
                *end = ' ';
                word[0] = '\0';
                return word;
            }
    }
    while (1) {
        if ((*end == ' ' || *end == '\n' || *end == '\t') && (flag != 1)) {
            break;
        }
        if (*end == '"' && flag == 1) {
            *end = ' ';
            break;
        }
        if (*end == '<' || *end == '>' || *end == '|') {
            break;
        }
        word = (char *)realloc(word, (i + 2) * sizeof(char));
        if (word == NULL) {
            perror("realloc");
            return NULL;
        }
        word[i] = *end;
        if (read(0, end, 1) < 0) {
            perror("read");
            free(word);
            return NULL;
        }
        i++;
    }
    word[i] = '\0';
    return word;
}

void free_cmd(char ***cmd) {
    for (int i = 0; cmd[i] != NULL; i++) {
        for (int j = 0; cmd[i][j] != NULL; j++) {
            free(cmd[i][j]);
        }
        free(cmd[i]);
    }
    free(cmd);
}

char **get_list(char *end) {
    char **words = NULL;
    ssize_t err = 0;
    int i;
    if (read(1, end, 1) < 0) {
        perror("read");
        return NULL;
    }
    for (i = 0; *end != '\n' && *end != '|'; i++) {
        words = (char **)realloc(words, (i + 2) * sizeof(char *));
        if (words == NULL) {
            perror("realloc");
            for (int j = 0; j < i; j++) {
                free(words[j]);
            }
            free(words);
            return NULL;
        }
        while (*end == ' ' || *end == '\t') {
            err = read(0, end, 1);
            if (err < 0) {
                perror("read");
                for (int j = 0; j < i; j++) {
                    free(words[j]);
                }
                free(words);
                return NULL;
            } else if (*end == '\n') {
                words[i] = NULL;
                return words;
            }
        }
        words[i] = get_word(end);
        if (bg == 1) {
            words[i + 1] = NULL;
            return words;
        }
        if (*end == '|') {
          words[i + 1] = NULL;
          return words;
        }
        if (words[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(words[j]);
            }
            free(words);
            return NULL;
        }
    }
    if (words != NULL) {
        words[i] = NULL;
    }
    return words;
}


char ***get_cmd(int *n) {
    char  ***cmd = NULL, end = '\0';
    int i = 0;
    while (end != '\n') {
        cmd = (char ***)realloc(cmd, (i + 2) * sizeof(char **));
        if (cmd == NULL) {
            perror("realloc char***");
            return NULL;
        }
        cmd[i] = get_list(&end);
        if (cmd[i] == NULL && end == '\n') {
            free_cmd(cmd);
            *n = -1;
            return NULL;
        }
        if (cmd[i] == NULL) {
            perror("get_list");
            free_cmd(cmd);
            return NULL;
        }
        i++;
    }
    *n = i;
    if (cmd[i] != NULL) {
        cmd[i] = NULL;
    }
    return cmd;
}

void free_list(char **list) {
    for (int i = 0; list[i] != NULL; i++) {
        free(list[i]);
    }
    free(list);
}

void del_word(char **cmd, int n) {
    char *word;
    while (cmd[n + 1] != NULL) {
        printf("7%s7\n", cmd[n]);
        printf("8%s8\n", cmd[n+1]);
        word = cmd[n];
        cmd[n] = cmd[n + 1];
        cmd[n + 1] = word;
        n++;
    }
    free(cmd[n]);
    free(cmd[n + 1]);
    cmd[n] = NULL;
}

int redir(char **cmd) {
    int fd;
    int i;
    for (i = 0; cmd[i] != NULL; i++) {
        if (strcmp(cmd[i], ">") == 0) {
            fd = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("failed to open file after <");
                exit(1);
            }
            dup2(fd, 1);
            break;
        } else if (strcmp(cmd[i], "<") == 0) {
            fd = open(cmd[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("failed to open file after >");
                exit(1);
            }
            dup2(fd, 0);
            break;
        }
    }
    if (cmd[i] != NULL) {
        del_word(cmd, i);
        del_word(cmd, i);
    }
    return fd;
}


void handler (int signo) {
	kill(signo, SIGINT);
}

int main(void) {
    char ***cmd = NULL;
    int n = 0;
    while (1) {
        print();
        cmd = get_cmd(&n);
        if (n == -1) {
          continue;
        }
        if (cmd == NULL) {
            perror("something went wrong");
            return 1;
        }
        if (strcmp(cmd[0][0], "quit") == 0 ||
            strcmp(cmd[0][0], "exit") == 0) {
            free_cmd(cmd);
            break;
        }
        signal(SIGINT, handler);
	exec_cmd(cmd, n);
        free_cmd(cmd);
    }
    return 0;
}
