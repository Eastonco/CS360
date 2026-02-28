#include "header.h"

int main(int argc, char *argv[], char *env[])
{
    int cmdIndex;
    debug = true;

    initialize(); /* initialize root node of the file system tree */
    menu();

    while (true)
    {
        memset(pathname, 0, sizeof pathname); /* clear buffers */
        memset(command, 0, sizeof command);

        printf("$ ");
        fgets(line, sizeof(line), stdin);         /* read into buffer */
        line[strlen(line) - 1] = 0;               /* remove trailing \0 */
        sscanf(line, "%s %s", command, pathname); /* split input into cmd + pathnames */

        cmdIndex = find_command(command);
        switch (cmdIndex)
        {
        case 0:
            mkdir(pathname);
            break;
        case 1:
            removedir(pathname);
            break;
        case 2:
            ls(pathname);
            break;
        case 3:
            cd(pathname);
            break;
        case 4:
            pwd();
            break;
        case 5:
            create(pathname);
            break;
        case 6:
            rm(pathname);
            break;
        case 7:
            reload(pathname);
            break;
        case 8:
            save(pathname);
            break;
        case 9:
            menu();
            break;
        case 10:
            quit();
            return 0;
        default:
            printf("invalid command %s\n", command);
            break;
        }
    }
}
