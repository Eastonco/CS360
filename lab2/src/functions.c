#include "header.h"

void initialize(void)
{
    root = new_node("/", DIRECTORY_TYPE);
    cwd = root;
    strcpy(pathname, cwd->name);
    printf("Initialized OK...\n");
}

void save(char *filename)
{
    char file[64]; /* target save file name */

    if (!strcmp(filename, "")) /* if no save name given */
    {
        if (!strcmp(savefile, "")) /* and no previously loaded file */
        {
            if (debug) printf("No previously loaded file, setting to default name\n");
            strcpy(file, "filesystem.txt"); /* set to default name */
        }
        else
        {
            if (debug) printf("Previously loaded file present\n");
            strcpy(file, savefile); /* set to previously loaded file */
        }
    }
    else
    {
        if (debug) printf("Save file name provided\n");
        strcpy(file, filename);
        strcpy(savefile, file);
    }

    if (debug) printf("Saving to %s\n", file);

    FILE *fp = fopen(file, "w+");
    print_filesystem(fp);
    fclose(fp);

    if (debug) printf("Saved.\n");
}

int find_command(char *command)
{
    char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "menu", "quit"};

    for (int i = 0; i < (int)(sizeof(cmd) / sizeof(char *)); i++)
    {
        if (!strcmp(command, cmd[i]))
        {
            return i; /* found command: return index i */
        }
    }
    return -1; /* not found: return -1 */
}

/* ── Tree Operations ──────────────────────────────────────────────── */

NODE *new_node(char *name, char type)
{
    NODE *node = (NODE *)malloc(sizeof(NODE));
    if (debug) printf("MADE A NEW NODE WITH NAME: %s\n", name);
    strcpy(node->name, name);
    node->type = type;
    node->childPtr = NULL;
    node->parentPtr = NULL;
    node->siblingPtr = NULL;
    return node;
}

/*
 * dbname: split a pathname into directory and base components.
 *
 * dirname() and basename() from <libgen.h> modify their argument in place
 * (POSIX), so we copy to a temporary buffer before each call.
 *
 *   pathname = "/a/b/c"  =>  dname = "/a/b",  bname = "c"
 *   pathname = "foo"      =>  dname = ".",      bname = "foo"
 */
void dbname(char *pathname)
{
    char temp[128];
    strcpy(temp, pathname);
    strcpy(dname, dirname(temp));  /* dirname modifies temp, so we re-copy */
    strcpy(temp, pathname);
    strcpy(bname, basename(temp));
}

void print_node(NODE *pcur)
{
    if (pcur->type == DIRECTORY_TYPE)
    {
        printf(SET_TEXT_BLUE);
        printf("%s\n", pcur->name);
        printf(RESET_TEXT);
    }
    else
    {
        printf("%s\n", pcur->name);
    }
}

void pwd()
{
    pwdhelper(cwd);
    printf("\n");
}

void mkdir(char *pathname)
{
    dbname(pathname);
    NODE *location = parse_pathname(pathname);
    if (location == NULL) /* parse_pathname returns NULL on invalid path */
    {
        return;
    }

    /* Walk sibling list to check for duplicate directory names */
    NODE *dupe_search = location->childPtr;
    while (dupe_search)
    {
        if (!strcmp(dupe_search->name, bname) && (dupe_search->type == DIRECTORY_TYPE))
        {
            printf("ERROR: Dirname already Exists");
            return;
        }
        dupe_search = dupe_search->siblingPtr;
    }
    insert_node(location, bname, DIRECTORY_TYPE);
}

/*
 * insert_node: add a new child node to a parent in the LC-RS tree.
 *
 * Two cases:
 *   1. Parent has no children yet  → set childPtr directly.
 *   2. Parent already has children → walk to the end of the sibling list
 *      and append the new node there.
 *
 * Returns the newly created node.
 */
NODE *insert_node(NODE *pcur, char *name, char type)
{
    NODE *new_child = new_node(name, type);

    if (pcur->childPtr == NULL) /* empty directory: insert as first child */
    {
        pcur->childPtr = new_child;
        new_child->parentPtr = pcur;
    }
    else
    {
        /* Walk to the last sibling, then append */
        NODE *sibling = pcur->childPtr;
        while (sibling->siblingPtr)
        {
            sibling = sibling->siblingPtr;
        }
        sibling->siblingPtr = new_child;
        new_child->parentPtr = pcur; /* parent is the same as existing siblings' parent */
    }
    return new_child;
}

/*
 * find_node: navigate from pcur through the tree using a tokenized pathname.
 *
 * Uses strtok() to split pathname on '/' and descend one level per token.
 * Only handles directory lookups (DIRECTORY_TYPE). Does not support '..' here.
 */
NODE *find_node(NODE *pcur, char *pathname)
{
    char *s;
    s = strtok(pathname, "/"); /* first call initializes strtok state */
    if (strcmp(s, "."))
    {
        while (s)
        {
            pcur = find_helper(pcur->childPtr, s, DIRECTORY_TYPE);
            if (pcur == NULL)
            {
                if (debug) printf("ERROR IN FINDNODE\n");
                break;
            }
            s = strtok(0, "/"); /* NULL first arg continues from last position */
        }
    }
    return pcur;
}

/*
 * find_helper: linear search through a sibling list for a node by name and type.
 *
 * The sibling list is a singly-linked list; we recurse (or could iterate)
 * through it until we find a match or reach NULL.
 */
NODE *find_helper(NODE *pcur, char *target, char file_type)
{
    if (pcur == NULL)
    {
        printf("Invalid path: ");
        if (file_type == DIRECTORY_TYPE)
        {
            printf("Directory %s not found\n", target);
        }
        else
        {
            printf("File %s not found\n", target);
        }
        return pcur;
    }
    else if (!strcmp(pcur->name, target) && pcur->type == file_type)
    {
        if (debug) printf("%s == %s\n", pcur->name, target);
        return pcur;
    }
    else
    {
        if (debug) printf("%s != %s\n", pcur->name, target);
        return find_helper(pcur->siblingPtr, target, file_type); /* check next sibling */
    }
}

/*
 * pwdhelper: recursively build the current working directory path.
 *
 * Walk up to root via parentPtr, then print each node's name on the way back
 * down (post-order), producing the correct left-to-right path string.
 *
 * BUG FIX: was `pcur->name == root->name` (pointer comparison, always false
 * for different heap allocations). Fixed to use strcmp() against "/".
 */
void pwdhelper(NODE *pcur)
{
    if (!strcmp(pcur->name, "/")) /* base case: we've reached root */
    {
        printf("/");
        return;
    }
    pwdhelper(pcur->parentPtr);
    if (pcur->parentPtr == root)
    {
        printf("%s", pcur->name);
    }
    else
    {
        printf("/%s", pcur->name);
    }
}

/* Same as pwdhelper but writes to a file descriptor (used by save()) */
void fpwd(NODE *pcur, FILE *fd)
{
    if (!strcmp(pcur->name, "/")) /* BUG FIX: was pointer comparison == root->name */
    {
        fprintf(fd, "/");
        return;
    }
    fpwd(pcur->parentPtr, fd);
    if (pcur->parentPtr == root)
    {
        fprintf(fd, "%s", pcur->name);
    }
    else
    {
        fprintf(fd, "/%s", pcur->name);
    }
}

/*
 * rprint: pre-order recursive traversal of the tree, printing each node.
 *
 * Visits current node, then recursively visits all children, then siblings.
 * This writes entries in the format used by reload() to reconstruct the tree.
 */
void rprint(NODE *pcur, FILE *fd)
{
    if (pcur == NULL)
    {
        return;
    }
    fprintf(fd, "%c\t", pcur->type);
    fpwd(pcur, fd);
    fprintf(fd, "\n");
    rprint(pcur->childPtr, fd);   /* depth-first: visit children before siblings */
    rprint(pcur->siblingPtr, fd);
}

void print_filesystem(FILE *fd)
{
    fprintf(fd, "type\tpathname\n\n");
    rprint(root, fd);
}

void ls(char *pathname)
{
    NODE *temp;
    if (!strcmp(pathname, "")) /* no argument: list current directory */
    {
        temp = cwd;
    }
    else
    {
        dbname(pathname);
        temp = parse_pathname(pathname);
        if (temp == NULL) /* BUG FIX: NULL check before dereferencing */
        {
            printf("ERROR: invalid path\n");
            return;
        }
        temp = find_helper(temp->childPtr, bname, DIRECTORY_TYPE);
    }

    if (temp == NULL)
    {
        return;
    }
    /* Walk the child list and print each entry */
    temp = temp->childPtr;
    while (temp)
    {
        print_node(temp);
        temp = temp->siblingPtr;
    }
}

/*
 * parse_pathname: resolve a pathname to its parent directory NODE.
 *
 * Determines whether the path is absolute (starts with '/') or relative,
 * then walks the tree to find the directory containing the target.
 * Returns NULL if the path is invalid.
 *
 * Examples:
 *   "/a/b/c"  → returns NODE for "/a/b"  (absolute)
 *   "foo/bar" → returns NODE for "foo"   (relative to cwd)
 *   ".."      → returns cwd->parentPtr
 */
NODE *parse_pathname(char *pathname)
{
    dbname(pathname);
    if (debug) printf("dname = %s, bname = %s\n", dname, bname);

    NODE *temp;
    if (dname[0] == '/') /* ABSOLUTE PATH: start from root */
    {
        if (!strcmp(dname, "/"))
        {
            temp = root;
        }
        else
        {
            temp = find_node(root, dname);
        }
    }
    else /* RELATIVE PATH: start from cwd */
    {
        if (!strcmp(dname, "."))
        {
            temp = cwd;
        }
        else if (!strcmp(dname, ".."))
        {
            temp = cwd->parentPtr;
        }
        else
        {
            temp = find_node(cwd, dname);
        }
    }
    return temp;
}

void cd(char *pathname)
{
    NODE *temp;
    dbname(pathname);
    if (pathname != NULL)
    {
        dbname(pathname);
        temp = parse_pathname(pathname);
        if (!strcmp(bname, ".."))
        {
            if (temp->parentPtr)
            {
                temp = temp->parentPtr;
            }
        }
        else if (!strcmp(bname, "/"))
        {
            /* cd / — temp is already root from parse_pathname */
        }
        else
        {
            temp = find_helper(temp->childPtr, bname, DIRECTORY_TYPE);
        }
    }
    else
    {
        return;
    }
    if (temp == NULL)
    {
        printf("Directory not found\n");
        return;
    }
    cwd = temp;
}

void create(char *pathname)
{
    if (debug) printf("Creat called\n");
    dbname(pathname);
    NODE *location = parse_pathname(pathname);
    if (location == NULL) /* BUG FIX: NULL check before dereferencing */
    {
        printf("ERROR: invalid path\n");
        return;
    }
    NODE *dupe_search = location->childPtr;
    while (dupe_search)
    {
        if (!strcmp(dupe_search->name, bname) && (dupe_search->type == FILE_TYPE))
        {
            printf("ERROR: Filename already Exists");
            return;
        }
        dupe_search = dupe_search->siblingPtr;
    }
    if (!strcmp(bname, "."))
    {
        printf("Please provide a valid filename\n");
        return;
    }
    insert_node(location, bname, FILE_TYPE);
}

/*
 * delete_node: unlink a node from the tree and free its memory.
 *
 * Two cases based on where the node sits in the sibling list:
 *   1. It is the first child (parent->childPtr == pcur):
 *      Redirect parent->childPtr to pcur's next sibling.
 *   2. It is somewhere in the middle/end of the sibling list:
 *      Walk to the node just before pcur and bridge over it.
 */
void delete_node(NODE *pcur)
{
    NODE *parent = pcur->parentPtr;
    NODE *temp;
    if (!strcmp(pcur->name, "/"))
    {
        printf("STOP TRYING TO DELETE THE ROOT\n");
        return;
    }
    if (parent->childPtr == pcur)
    {
        if (pcur->siblingPtr == NULL)
        {
            free(pcur);
            parent->childPtr = NULL;
        }
        else
        {
            parent->childPtr = pcur->siblingPtr;
            free(pcur);
        }
    }
    else
    {
        temp = parent->childPtr;
        while (temp->siblingPtr != pcur) /* find the node before pcur */
        {
            temp = temp->siblingPtr;
        }
        temp->siblingPtr = pcur->siblingPtr; /* bridge over pcur */
        free(pcur);
    }
}

void removedir(char *pathname)
{
    if (debug) printf("rmdir called\n");
    dbname(pathname);
    NODE *location = parse_pathname(pathname);
    if (location == NULL) /* BUG FIX: NULL check before dereferencing */
    {
        printf("ERROR: invalid path\n");
        return;
    }
    if (!strcmp(bname, "."))
    {
        printf("Please provide a valid filename\n");
        return;
    }
    location = find_helper(location->childPtr, bname, DIRECTORY_TYPE);
    if (location == NULL)
    {
        return;
    }
    if (location->childPtr != NULL)
    {
        printf("Error: Can't remove a non empty dir\n");
        return;
    }
    if (location->type != DIRECTORY_TYPE)
    {
        printf("Error: %s is not a directory", bname);
        return;
    }
    delete_node(location);
}

void rm(char *pathname)
{
    if (debug) printf("rm called\n");
    dbname(pathname);
    NODE *location = parse_pathname(pathname);
    if (!strcmp(bname, "."))
    {
        printf("Please provide a valid filename\n");
        return;
    }
    location = find_helper(location->childPtr, bname, FILE_TYPE);
    if (location == NULL)
    {
        printf("Error: File not found\n");
        return;
    }
    if (location->type != FILE_TYPE)
    {
        printf("Error: %s is not a file\n", bname);
        return;
    }
    delete_node(location);
}

void quit()
{
    save("");
    printf("Goodbye\n");
}

void menu()
{
    printf("valid commands include: ls, pwd, menu, mkdir, rmdir, rm, creat, save, reload, & quit\n");
}

/*
 * reload: restore the filesystem tree from a saved text file.
 *
 * File format (written by save/print_filesystem):
 *   Line 1: "type\tpathname"  (header)
 *   Line 2: ""                (blank)
 *   Lines 3+: "<D|F>\t<absolute_path>"  one entry per node
 *
 * Reads each entry and calls mkdir() or create() to rebuild the tree.
 */
void reload(char *filename)
{
    char save[64], buf[128], path[128], type;

    if (!strcmp(filename, ""))
    {
        if (!strcmp(savefile, ""))
        {
            printf("Please provide a save file to load\n");
            return;
        }
        else
        {
            strcpy(save, savefile);
        }
    }
    else
    {
        strcpy(save, filename);
    }

    FILE *fp = fopen(save, "r");
    if (fp == NULL) /* BUG FIX: NULL check — fopen returns NULL on failure */
    {
        printf("ERROR: could not open file '%s'\n", save);
        return;
    }

    /* Skip the header line ("type\tpathname") and blank line */
    fgets(buf, 128, fp);
    fgets(buf, 128, fp);
    fgets(buf, 128, fp);

    while (fgets(buf, 128, fp))
    {
        buf[strlen(buf) - 1] = 0; /* strip trailing newline */
        sscanf(buf, "%c\t%s", &type, path);

        if (debug) printf("%c %s", type, path);
        switch (type)
        {
        case DIRECTORY_TYPE:
            mkdir(path);
            break;
        case FILE_TYPE:
            create(path);
            break;
        default:
            printf("ERROR reading file\n");
            break;
        }
    }

    printf("System Reloaded successfully\n");
    fclose(fp);
}
