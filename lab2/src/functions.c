#include "header.h"

void initialize(void){
    root = new_node("/", DIRECTORY_TYPE);
    cwd = root;
    strcpy(pathname, cwd->name);
    printf("Initialized OK...\n");
}

void save(char *filename){ 
    char file[64];

    if(!strcmp(filename, "")) {
        if(!strcmp(savefile, "")){
            strcpy(file, "filesystem.txt");
        } else {
            strcpy(file, savefile);
        }
    } else {
        strcpy(file, filename);
        strcpy(savefile, file);
    }

    (debug) ? printf("Saving to %s\n", file) : NULL;


    FILE *fp = fopen(file, "w+");
    print_filesystem(fp);
    fclose(fp);

    (debug) ? printf("Saved.\n") : NULL;
}

int find_command(char *command){
    char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "menu", "quit", NULL};

    int i = 0;
    while(cmd[i]){
        if (!strcmp(command, cmd[i])){
            return i; // found command: return index i 
        }
        i++;
    }
    return -1; // not found: return -1
}

// ***************************************TREE THINGS******************************************************
NODE *new_node(char *name, char type){
    NODE *node = (NODE *)malloc(sizeof(NODE));
    (debug) ? printf("MADE A NEW NODE WITH NAME: %s\n", name) : NULL;
    strcpy(node->name, name);
    node->type = type;
    node->childPtr = NULL;
    node->parentPtr = NULL;
    node->siblingPtr = NULL;
    return node;
}

// NOTE: dirname truncates the string, basename copies to a new variable
void dbname(char *pathname){
    char temp[128]; // dirname(), basename() destroy original pathname 
    strcpy(temp, pathname);
    strcpy(dname, dirname(temp));
    strcpy(temp, pathname);
    strcpy(bname, basename(temp));
}

void print_node(NODE *pcur){
    if(pcur->type == DIRECTORY_TYPE){
        printf(SET_TEXT_BLUE);
        printf("%s\n", pcur->name);
        printf(RESET_TEXT);
    }
    else
    {
        printf("%s\n", pcur->name);
    }
}

void pwd(){
    pwdhelper(cwd);
    printf("\n");
}


void mkdir(char *pathname){ 
    dbname(pathname);
    NODE * location = parse_pathname(pathname);
    if(location == NULL){
        return;
    }
    NODE * dupe_search = location->childPtr;
    while(dupe_search){
        if(!strcmp(dupe_search->name, bname) && (dupe_search->type == DIRECTORY_TYPE)){
            printf("ERROR: Dirname already Exists");
            return;
        }
        dupe_search = dupe_search->siblingPtr;
    }
    insert_node(location, bname, DIRECTORY_TYPE);

}

NODE *insert_node(NODE *pcur, char *name, char type){ //pcur is the directory to insert
    NODE *new_child = new_node(name, type);

    if (pcur->childPtr == NULL){ //pcur is parent, empty directory, direct insert
        pcur->childPtr = new_child;
        new_child->parentPtr = pcur;
    }
    else{
        pcur = pcur->childPtr; // pcur has children, need to go to end of list
        while (pcur->siblingPtr){ // goes to end of children list
            pcur = pcur->siblingPtr;
        }
        pcur->siblingPtr = new_child;
        new_child->parentPtr = pcur->parentPtr;

    }
}

NODE *find_node(NODE *pcur, char *pathname){ //pcur is start directory
    char *s;
    s = strtok(pathname, "/"); // first call to strtok() 
    if(strcmp(s, ".")){
        while(s){
            pcur = find_helper(pcur->childPtr, s, DIRECTORY_TYPE);
            if(pcur== NULL){
                (debug) ? printf("ERROR IN FINDNODE\n") : NULL;
                break;
            }
            s = strtok(0, "/"); // call strtok() until it returns NULL 
        }
    }
    return pcur;
}

NODE *find_helper(NODE *pcur, char *target, char file_type){
    if(pcur == NULL){
        printf("Invalid path: ");
        if(file_type == DIRECTORY_TYPE){
            printf("Directory %s not found\n", target);
        }
        else{
            printf("File %s not found\n", target);
        }
        return pcur;
    }
    else if(!strcmp(pcur->name, target) && pcur->type == file_type){
        (debug) ? printf("%s == %s\n", pcur->name, target) : NULL;
        return pcur;
    }
    else{
        (debug) ? printf("%s != %s\n", pcur->name, target) : NULL;
        return find_helper(pcur->siblingPtr, target, file_type);
    }
} 



void pwdhelper(NODE * pcur){
    if(pcur->name == root->name){
        printf("/");
        return;
    }
    pwdhelper(pcur->parentPtr);
    if(pcur->parentPtr == root){
        printf("%s", pcur->name);
    }
    else{
        printf("/%s", pcur->name);
    }
}

void fpwd(NODE *pcur, FILE * fd){
    if(pcur->name == root->name){
        fprintf(fd,"/");
        return;
    }
    fpwd(pcur->parentPtr, fd);
    if(pcur->parentPtr == root){
        fprintf(fd, "%s", pcur->name);
    }
    else{
        fprintf(fd, "/%s", pcur->name);
    }
}


void rprint(NODE * pcur, FILE *fd){
    if(pcur == NULL){
        return;
    }
    fprintf(fd, "%c\t", pcur->type);
    fpwd(pcur, fd);
    fprintf(fd, "\n");
    rprint(pcur->childPtr, fd);
    rprint(pcur->siblingPtr, fd);
}

void print_filesystem(FILE * fd){
    fprintf(fd, "type\tpathname\n\n");
    rprint(root, fd);
}

void ls(char * pathname){
    NODE * temp;
    if(!strcmp(pathname, "")){
        temp = cwd;

    }
    else{
        dbname(pathname);
        temp = parse_pathname(pathname);
        temp = find_helper(temp->childPtr, bname, DIRECTORY_TYPE);
    }

    temp = temp->childPtr;
    while(temp){
        print_node(temp);
        temp = temp->siblingPtr;
    }
}

NODE * parse_pathname(char *pathname){
    dbname(pathname);
    (debug) ? printf("dname = %s, bname = %s\n", dname, bname) : NULL;

    NODE * temp;
    if (dname[0]== '/'){ // ABSOLUTE PATH -> start at root
        if(!strcmp(dname, "/") ){
            temp = root;

        }
        else{
            temp = find_node(root, dname);

        }
    }
    else{ // RELATIVE PATH -> start at cwd
        if(!strcmp(dname, ".")){
            temp = cwd;

        }
        else if(!strcmp(dname, "..")){
            temp = cwd->parentPtr;
        }
        else{
            temp = find_node(cwd, dname);

        }
    }
    return temp;
}

void cd(char *pathname){
    NODE * temp;
    dbname(pathname);
    if(pathname != NULL){
        dbname(pathname);
        temp = parse_pathname(pathname);
        if(!strcmp(bname, "..")){
            if(temp->parentPtr){
                temp = temp->parentPtr;
            }
        }
        else if(!strcmp(bname, "/")){

        }
        else{
            temp = find_helper(temp->childPtr, bname, DIRECTORY_TYPE);
        }
    }
    else{
        return;
    }
    if(temp == NULL){
        printf("Directory not found\n");
        return;
    }
    cwd = temp;
}

void create(char *pathname){
    (debug) ? printf("Creat called\n") : NULL ;
    dbname(pathname);
    NODE * location = parse_pathname(pathname);
    NODE * dupe_search = location->childPtr;
    while(dupe_search){
        if(!strcmp(dupe_search->name, bname) && (dupe_search->type == FILE_TYPE)){
            printf("ERROR: Dirname already Exists");
            return;
        }
        dupe_search = dupe_search->siblingPtr;
    }
    if(!strcmp(bname, ".")){
        printf("Please provide a valid filename\n");
        return;
    }
    insert_node(location, bname, FILE_TYPE);
}

void delete_node(NODE *pcur){
    NODE * parent = pcur->parentPtr;
    NODE * temp;
    if(!strcmp(pcur->name, "/")){
        printf("STOP TRYING TO DELETE THE ROOT\n");
        return;
    }
    if(parent->childPtr == pcur){
        if(pcur->siblingPtr == NULL){
            free(pcur);
            parent->childPtr = NULL;
        }
        else{
            parent->childPtr = pcur->siblingPtr;
            free(pcur);
        }
    }
    else{
        temp = parent->childPtr;
        while(temp->siblingPtr != pcur){
            temp = temp->siblingPtr;
        }
        temp->siblingPtr = pcur->siblingPtr;
        free(pcur);
    }
}

void removedir(char *pathname){
    (debug) ? printf("rmdir called\n") : NULL ;
    dbname(pathname);
    NODE * location = parse_pathname(pathname);
    if(!strcmp(bname, ".")){
        printf("Please provide a valid filename\n");
        return;
    }
    location = find_helper(location->childPtr, bname, DIRECTORY_TYPE);
    if(location->childPtr != NULL ){
        printf("Error: Can't remote a non empty dir\n");
        return;
    }
    if(location->type != DIRECTORY_TYPE){
        printf("Error: %s is not a directory", bname);
        return;
    }
    delete_node(location);
}

void rm(char * pathname){
    (debug) ? printf("rm called\n") : NULL ;
    dbname(pathname);
    NODE * location = parse_pathname(pathname);
    if(!strcmp(bname, ".")){
        printf("Please provide a valid filename\n");
        return;
    }
    location = find_helper(location->childPtr, bname, FILE_TYPE);
    if(location == NULL){
        printf("Error: File not found\n");
        return;
    }
    if(location->type != FILE_TYPE){
        printf("Error: %s is not a file\n", bname);
        return;
    }
    delete_node(location);
}

void quit(){
    save("");
    printf("Goodbye\n");
}

void menu(){
    printf("valid commands include: ls, pwd, menu, mkdir, rmdir, rm, creat, save, reload, & quit\n");
}

void reload(char *filename){ //TODO: fixme
    char save[64], buf[128], path[128], type;

    if(!strcmp(filename, "")) {
        if(!strcmp(savefile, "")){
            printf("Please provide a save file to load\n");
            return;
        } else {
            strcpy(save, savefile);
        }
    } else {
        strcpy(save, filename);
    }

    FILE *fp = fopen(save, "r");
    fgets(buf,128,fp); 
    fgets(buf,128,fp);
    fgets(buf,128,fp); 
    
    while(fgets(buf, 128, fp)){
        buf[strlen(buf)-1] = 0;
        sscanf(buf, "%c\t%s", &type, &path);

        (debug) ? printf("%c %s", type, path) : NULL;
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

