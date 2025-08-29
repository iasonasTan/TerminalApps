#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

void get_input();
void greet();
void view_notes();
void create_note();
void delete_note();
void edit_note();
void init_files();
void write_editor(char*);
void change_editor();
void load_preferred_editor();

char *home = NULL;
char data[200];
char config[200];
char editor[50];

int main() {
    init_files();
    load_preferred_editor();
    printf("Everything is set up, starting program...\n");

    greet();
    while(true) {
        get_input();
    }

    return 0;
}

void load_preferred_editor() {
    printf("Checking if configuration file exists...\n");
    struct stat config_st;
    printf("Building configuration file path...\n");
    snprintf(config, sizeof(config), "%s/.config/c_notepad.config", home);
    printf("Config file is %s.\n", config);
    if(stat(config, &config_st) != 0) {
        printf("File HOME/.config/c_notepad.config does not exist...\n");
        char command[100];
        snprintf(command, sizeof(command), "touch %s", config);
        int touch_result = system(command);
        if(touch_result == 0) {
            printf("Config file HOME/.config/c_notepad.config created successfully.\n");
        } else {
            printf("Cannot create file HOME/.config/c_notepad.config. Terminating application...\n");
            exit(touch_result);
        }
    }
    printf("Getting editor command from file %s...\n", config);
    FILE *fp = fopen(config, "r");
    if(fp == NULL) {
        printf("Cannot open config file. Terminating app...\n");
        exit(1);
    } else {
        // read config file, get editor command
        fgets(editor, sizeof(editor), fp);
        if(editor == NULL) {
            printf("Editor is null, please fill in HOME/.config/c_notepad.config with your preferred editor command...\n");
            exit(1);
        } else {
            int len = strlen(editor);
            while (len > 0 && isspace((unsigned char)editor[len - 1])) {
                editor[len - 1] = '\0';
                len--;
            }
        }
        printf("Preferred editor is %s.\n", editor);
    }
}

void write_editor(char* new_editor) {
    FILE *fp = fopen(config, "w");
    if(fp == NULL) {
        printf("Cannot open file. terminating app...");
        exit(1);
    } else {
        printf("Writing editor %s to config file...\n", new_editor);
        fprintf(fp, new_editor);
    }
}

void init_files() {
    home = getenv("HOME");
    printf("Checking if HOME/notepad_data dir exists...\n");
    struct stat st;
    printf("Building working directory...\n");
    snprintf(data, sizeof(data), "%s/notepad_data", home);
    printf("Working directory is %s, checking...\n", data);
    if(stat(data, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("Directory HOME/notepad_data does not exist, creating directory...\n");
        char command[100];
        snprintf(command, sizeof(command), "mkdir %s", data);
        int mkdir_result = system(command);
        if(mkdir_result == 0) {
            printf("Directory HOME/notepad_data created.\n");
        } else {
            printf("Cannot create directory HOME/notepad_data, terminating app.\n");
            exit(mkdir_result);
        }
    }
}

void change_editor() {
    printf("Enter editor command you want to use: ");
    char editor_l[50];
    scanf("%s", &editor_l);
    write_editor(editor_l);
    init_files();
    load_preferred_editor();
}

void get_input() {
    printf("What do you want to do?\n");
    printf("Edit: e; View: v; Delete: d; New n; Quit: q; ChangeEditor: s;\n");
    char input;
    scanf(" %c ", &input);
    switch(input) {
        case 'e': edit_note(); break;
        case 'd': delete_note(); break;
        case 'v': view_notes(); break;
        case 'n': create_note(); break;
        case 'q': exit(0); break;
        case 's': change_editor(); break;
        default:
            printf("This is not a valid operation, please select something else.");
    }
}

void edit_note() {
    printf("Which note do you want to edit? ");
    char name[50];
    scanf("%s", &name);
    char command[54];
    snprintf(command, sizeof(command), "%s %s/%s", editor, data, name);
    int result = system(command);
    if(result != 0) {
        printf("Failed!\n");
    }
}

void delete_note() {
    printf("Which note do you want to delete? ");
    char name[50];
    scanf("%s", &name);
    char command[55];
    snprintf(command, sizeof(command), "rm -r %s/%s", data, name);
    int result = system(command);
    if(result != 0) {
        printf("Failed!\n");
    }
}

void view_notes() {
    printf("Notes:\n");
    char command[100];
    snprintf(command, sizeof(command), "ls -la %s", data);
    system(command);
    printf("\n");
}

void create_note() {
    printf("Note name: ");
    const int COMMAND_LEN = 6, NAME_MAX_LEN = 50;
    char name[NAME_MAX_LEN];
    scanf("%s", &name);

    char command[COMMAND_LEN + NAME_MAX_LEN];
    snprintf(command, sizeof(command), "touch %s/%s", data, name);
    int result = system(command);
    if(result != 0) {
        printf("Failed to create note. another note with the same name might exist.\n");
    }
}

void greet() {
    printf("**************************************************************************\n");
    printf("************************* WELCOME TO C NOTEPAD ***************************\n");
    printf("**************************************************************************\n");
}