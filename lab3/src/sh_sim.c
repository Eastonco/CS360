/************** sh_sim.c — Shell Simulator **************/
/*
 * Architecture overview:
 *
 *   main()
 *     ├── scan PATH from env[], decompose into dir[] array
 *     └── REPL loop:
 *           fgets() → tokenize → handle builtins (cd, exit)
 *           └── fork()
 *                 parent: wait() for child
 *                 child:  doPipe(line, 0)
 *
 *   doPipe(cmdLine, pd):
 *     If a pipe '|' exists in cmdLine → fork again:
 *       parent: read end of new pipe → doCommand(tail)
 *       child:  doPipe(head, pipe_write_end)   ← recursive!
 *     Else: doCommand(cmdLine)
 *
 *   doCommand(cmdLine):
 *     tokenize → ioRedirection → execve() through dir[] search paths
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX 128

/* environ: the process environment, accessible as an extern on both Linux and macOS.
 * Linux also provides environ; we use the standard POSIX name for portability. */
extern char **environ;

/* PATH directories — populated by scanning env[] for "PATH=" at startup */
char gdir[MAX];  /* storage for strtok to carve up the PATH string */
char *dir[64];   /* dir[0..ndir-1] point into gdir[] */
int ndir;        /* number of directories in PATH */

char gpath[MAX]; /* storage for tokenizeLine to carve up command lines */
char *name[64];  /* name[0] = command, name[1..] = args, NULL-terminated conceptually */
int ntoken;      /* number of tokens in the current command */

int fd[2];       /* saved original stdin/stdout fds for resetStreams() */
char *s, line[MAX];
int i;
int pid, status;
char *head, *tail; /* set by scan(): head = left of '|', tail = right */
int pd[2];
int lpd[2];

char *home_dir;  /* BUG FIX: pointer to HOME directory, found dynamically at startup */

/* Forward declarations */
int  doPipe(char *cmdLine, int *pd);
int  scan(char *cmdLine);
int  doCommand(char *cmdLine);
void tokenizeLine(char *line);
void ioRedirection(void);
void resetStreams(void);

int main(int argc, char *argv[], char *env[])
{
  /* Save copies of original stdin (0) and stdout (1) so resetStreams() can
   * restore them after I/O redirection modifies the child's fd table. */
  fd[0] = dup(1); /* save stdout */
  fd[1] = dup(0); /* save stdin  */

  printf("************* Welcome to kcsh **************\n");

  /*
   * Scan env[] to find PATH= and HOME=.
   * env[] is a NULL-terminated array of "KEY=value" strings from the OS.
   * We search every entry rather than assuming a fixed index (like env[28])
   * because the order of environment variables is not guaranteed.
   */
  i = 0;
  while (env[i])
  {
    printf("env[%d] = %s\n", i, env[i]);

    if (strncmp(env[i], "PATH=", 5) == 0)
    {
      printf("show PATH: %s\n", env[i]);
      printf("decompose PATH into dir strings in gdir[ ]\n");

      /* Copy PATH value (skip "PATH=" prefix) into gdir for strtok to modify */
      strcpy(gdir, &env[i][5]);

      /* strtok splits gdir on ':' in-place, returning pointer to each token.
       * dir[] stores pointers into gdir so we can look up each directory later. */
      ndir = 0;
      char *p = strtok(gdir, ":");
      while (p != NULL)
      {
        dir[ndir++] = p;
        p = strtok(NULL, ":");
      }

      for (int j = 0; j < ndir; j++)
      {
        printf("%d. %s\n", j, dir[j]);
      }
    }

    /* BUG FIX: was &env[28][5] — hardcoded index that is not guaranteed to be HOME.
     * Now we search every env entry for the "HOME=" prefix. */
    if (strncmp(env[i], "HOME=", 5) == 0)
    {
      home_dir = &env[i][5]; /* point directly into env string (safe — env persists) */
    }

    i++;
  }

  printf("*********** kcsh processing loop **********\n");

  while (1)
  {
    printf("\nkcsh $: ");
    fgets(line, MAX, stdin);
    line[strlen(line) - 1] = 0; /* strip trailing '\n' that fgets includes */

    if (line[0] == 0)
      continue;
    printf("line = %s\n", line);

    /* Tokenize the line into name[0..ntoken-1] for the cd/exit builtin checks */
    strcpy(gpath, line);
    tokenizeLine(line); /* BUG FIX: was tokenizeLine(&line); line already decays to char* */

    /* Builtin: cd
     * cd with no argument changes to HOME; cd <path> changes to that path.
     * This must be handled in the shell process itself (not a child), because
     * chdir() only affects the calling process — a child exec'ing cd would
     * change directory in the child, then the child exits and the parent is unchanged. */
    if (strcmp(name[0], "cd") == 0)
    {
      if (name[1] == NULL)
      {
        if (home_dir)
        {
          printf("%s\n", home_dir);
          chdir(home_dir);
        }
        else
        {
          printf("cd: HOME not set\n");
        }
      }
      else
      {
        chdir(name[1]);
      }
      continue;
    }

    if (!strcmp(name[0], "exit"))
    {
      exit(1);
    }

    /*
     * Fork/exec model:
     *   The shell forks a child process to run each command.
     *   The parent blocks in wait() until the child exits.
     *   The child calls doPipe() which eventually calls execve() to replace
     *   itself with the target program. If execve succeeds, the child never
     *   returns; if it fails (command not found), the child exits with code 123.
     */
    pid = fork();

    if (pid) /* parent: wait for child to finish */
    {
      printf("parent sh %d waits\n", getpid());
      pid = wait(&status);
      printf("child sh %d died : exit status = %04x\n", pid, status);
      continue;
    }
    else /* child: run the command (possibly with pipes) */
    {
      printf("child sh %d begins\n", getpid());
      doPipe(gpath, 0); /* gpath holds the full command line for re-tokenization */
    }
  }
}

/*
 * doPipe: recursively handle pipe chains in a command line.
 *
 * pd: if non-NULL, this process is the WRITE side of a pipe from the caller.
 *     We close the read end and dup2 the write end onto stdout before proceeding.
 *
 * scan() splits cmdLine at the RIGHTMOST '|' into head and tail.
 * When a pipe is found, we:
 *   1. Create a new pipe (lpd)
 *   2. Fork: parent reads from lpd (exec tail), child writes to lpd (doPipe head)
 *
 * This recurses leftward through the pipe chain.
 * Example: "a | b | c"
 *   scan finds rightmost '|' → head="a | b", tail="c"
 *   fork: parent execs "c" reading from pipe; child calls doPipe("a | b")
 *     scan finds '|' → head="a", tail="b"
 *     fork: parent execs "b"; child doPipe("a") → doCommand("a")
 */
int doPipe(char *cmdLine, int *pd)
{
  if (pd) /* we are the write side of a pipe from our caller */
  {
    close(pd[0]);       /* don't need read end */
    dup2(pd[1], 1);     /* redirect our stdout to the pipe write end */
    close(pd[1]);
  }

  int hasPipe = scan(cmdLine);
  if (hasPipe)
  {
    pipe(lpd);
    int pid = fork();
    if (pid) /* parent: reads from pipe, runs the right-hand command */
    {
      close(lpd[1]);    /* parent doesn't write to pipe */
      dup2(lpd[0], 0);  /* redirect stdin to pipe read end */
      close(lpd[0]);
      doCommand(tail);
    }
    else /* child: writes to pipe, handles the left-hand part (may recurse) */
    {
      doPipe(head, lpd);
    }
  }
  else
  {
    doCommand(cmdLine);
  }
  return 0;
}

/*
 * scan: find the RIGHTMOST pipe symbol in cmdLine.
 *
 * Scanning right-to-left ensures that "a | b | c" splits as:
 *   head = "a | b", tail = "c"
 * rather than head = "a", tail = "b | c", which would mishandle further pipes.
 *
 * On finding '|': terminates head at that position, sets tail to the char after.
 * Returns 1 if pipe found, 0 otherwise.
 */
int scan(char *cmdLine)
{
  for (int i = strlen(cmdLine) - 1; i > 0; i--)
  {
    if (cmdLine[i] == '|')
    {
      cmdLine[i] = 0;        /* null-terminate the head portion */
      tail = cmdLine + i + 1;
      head = cmdLine;
      return 1;
    }
  }
  head = cmdLine;
  tail = NULL;
  return 0;
}

/*
 * doCommand: tokenize a command line, apply I/O redirection, then exec.
 *
 * PATH search: for each directory in dir[], build "dir/command" and try execve().
 * execve() only returns if it fails (ENOENT etc.), so the loop continues until
 * one succeeds or we exhaust all directories.
 *
 * Relative path shortcut: if name[0] starts with "./" use cwd instead of PATH.
 */
int doCommand(char *cmdLine)
{
  memset(name, 0, sizeof(name));
  tokenizeLine(cmdLine);
  ioRedirection();

  /* BUG FIX: use snprintf to prevent buffer overflow when building full command path.
   * dir[i] + "/" + name[0] can exceed 64 bytes if either is long. */
  char cmd[MAX];

  if (name[0][0] == '.' && name[0][1] == '/')
  {
    /* Relative path (./program): prepend cwd to get absolute path */
    char temp[MAX];
    getcwd(temp, sizeof(temp));
    snprintf(cmd, sizeof(cmd), "%s/%s", temp, name[0]);
    execve(cmd, name, environ);
  }

  for (int i = 0; i < ndir; i++)
  {
    snprintf(cmd, sizeof(cmd), "%s/%s", dir[i], name[0]);
    execve(cmd, name, environ); /* only returns on failure — try next dir */
  }

  printf("cmd %s not found, child sh exit\n", name[0]);
  exit(123);
}

/*
 * tokenizeLine: split a space-delimited command string into tokens.
 *
 * Uses strtok() to split line on spaces in-place.
 * name[0..ntoken-1] point into the modified line buffer.
 * After this call, name[ntoken] is implicitly NULL (from memset in doCommand).
 */
void tokenizeLine(char *line)
{
  ntoken = 0;
  char *p = strtok(line, " ");
  while (p != NULL)
  {
    name[ntoken++] = p;
    p = strtok(NULL, " ");
  }
}

/*
 * ioRedirection: apply stdin/stdout redirections found in name[].
 *
 * The close+open+dup2 idiom:
 *   close(1)              — free file descriptor slot 1 (stdout)
 *   fd = open("file", …) — open file, gets the lowest available fd (now 1)
 *   dup2(fd, 1)           — if open didn't get fd 1, force-copy it there
 *
 * After this, fd 1 points to the file. When execve() replaces the process,
 * the new program inherits the redirected file descriptors.
 *
 * We also set name[i] = NULL to remove the redirection token from the
 * argument list that execve() sees.
 *
 * BUG FIX: open() with O_CREAT requires a third argument: the permission mode.
 * Without it, the file gets random permissions from stack garbage. 0644 gives
 * owner read+write, group/other read — the standard Unix default.
 */
void ioRedirection(void)
{
  for (int i = 0; i < ntoken; i++)
  {
    if (strcmp(name[i], ">") == 0)
    {
      name[i] = NULL;
      close(1);
      int rfd = open(name[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(rfd, 1);
    }
    else if (strcmp(name[i], ">>") == 0)
    {
      name[i] = NULL;
      close(1);
      int rfd = open(name[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
      dup2(rfd, 1);
    }
    else if (strcmp(name[i], "<") == 0)
    {
      name[i] = NULL;
      close(0);
      int rfd = open(name[i + 1], O_RDONLY);
      dup2(rfd, 0);
    }
  }
}

/*
 * resetStreams: restore stdin and stdout to the saved originals.
 *
 * BUG FIX: was dup2(fd[0], stdout) and dup2(fd[1], stdin).
 * stdout and stdin are FILE* pointers, not file descriptors.
 * dup2() requires integer file descriptor numbers: 1 (stdout) and 0 (stdin).
 */
void resetStreams(void)
{
  dup2(fd[0], 1); /* restore stdout */
  dup2(fd[1], 0); /* restore stdin  */
}
