#ifndef FILE_SYSTEM
#define FILE_SYSTEM

#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define DIRECTORY_TYPE 'D'
#define FILE_TYPE      'F'
#define SET_TEXT_BLUE  "\e[36;1m"  /* ANSI escape: bold cyan for directories */
#define RESET_TEXT     "\033[0m"

/*
 * NODE: one entry in the in-memory filesystem tree.
 *
 * The tree uses a "left-child / right-sibling" (LC-RS) representation,
 * which allows an arbitrary number of children per node using only two
 * pointers. Instead of storing a list of children, each node stores:
 *   - childPtr:   the FIRST child of this node
 *   - siblingPtr: the NEXT sibling (another child of the same parent)
 *
 * Example tree for:  / → (a, b, c) where a → (x, y)
 *
 *     root
 *      |
 *      a ——> b ——> c        (siblings: follow siblingPtr)
 *      |
 *      x ——> y              (children of 'a': follow childPtr then siblingPtr)
 *
 * To list all children of a node: start at node->childPtr, then walk siblingPtr.
 */
typedef struct node
{
    char name[64];           /* node name (directory or file); BUG FIX: was char *name[64]
                                which declared an array of 64 pointers, not a 64-byte string */
    char type;               /* DIRECTORY_TYPE ('D') or FILE_TYPE ('F') */
    struct node *parentPtr;  /* pointer to parent node (NULL for root) */
    struct node *siblingPtr; /* pointer to next sibling in parent's child list */
    struct node *childPtr;   /* pointer to first child of this node */

} NODE;

/*
 * Global filesystem state.
 * These are defined here (in the header) — which works because -fcommon allows
 * multiple definitions of the same global symbol. Normally globals in headers
 * should be declared `extern` with one definition in a .c file.
 */
NODE *root, *cwd;  /* root of the tree; cwd = current working directory */
char line[128], command[16], pathname[64], dname[64], bname[64], savefile[64];
bool debug;

/*
 * Example of dirname/basename split:
 *   pathname = "/this/that/hello"
 *   dname = "/this/that"
 *   bname = "hello"
 */

void initialize(void);
int find_command(char *command);
NODE *new_node(char *name, char type);
void dbname(char *pathname);
void print_node(NODE *pcur);
void save(char *filename);
void pwd();
void reload(char *filename);
void pwdhelper(NODE *pcur);
void quit();
void menu();

void mkdir(char *pathname);
NODE *insert_node(NODE *parent, char *name, char type);
NODE *find_node(NODE *pcur, char *pathname);
NODE *find_helper(NODE *pcur, char *target, char file_type);

void rprint(NODE *pcur, FILE *fd);
void print_filesystem(FILE *fd);
void fpwd(NODE *pcur, FILE *fd);

void ls(char *pathname);
void rm(char *pathname);

NODE *parse_pathname(char *pathname);

void create(char *pathname);
void cd(char *pathname);

void delete_node(NODE *pcur);
void removedir(char *pathname);

#endif
