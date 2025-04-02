#include <stdio.h>      
#include <string.h>     
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/types.h>  
#include <stdlib.h>     

#define BUF_SIZE 100            
#define LINUX_PWD_MAX 4096      
#define ECHO_MAX_WORDS 20       
// ====================================
// 🔹 Global Variables
// ====================================
char *g_val = NULL;             // Global variable to hold a value from local variables

struct loc_var_node {
    char *keywords;             // Name of the local variable
    char *value;                // Value of the local variable
    struct loc_var_node *next;  // Pointer to the next node in the linked list
};

struct loc_var_node *head = NULL;    // Head pointer for the linked list of local variables
struct loc_var_node *current = NULL; // Pointer used for traversing the linked list

// ====================================
// 🔹 Function: Display Local Environment Variables
// ====================================
void print_env(void) {
    struct loc_var_node *ptr = head;
    
    // Loop through the linked list and print each variable in "name=value" format
    while (ptr != NULL) {
        printf("%s=%s\n", ptr->keywords, ptr->value);
        ptr = ptr->next;
    }
}

// ====================================
// 🔹 Function: Get Local Variable Value
// ====================================
void get_env(char *key) {
    struct loc_var_node *ptr = head;
    
    // Traverse the linked list until the matching variable is found
    while (ptr != NULL) {
        if (strcmp(ptr->keywords, key) == 0) {
            free(g_val); // Free previous global value if any
            g_val = (char *)malloc(strlen(ptr->value) + 1); // Allocate memory for new value
            strcpy(g_val, ptr->value); // Copy the value from the node into g_val
            return;
        }
        ptr = ptr->next;
    }
}

// ====================================
// 🔹 Function: Set Local Variable
// ====================================
void set_loc_env(char *keyword, char *value) {
    // Allocate memory for a new linked list node
    struct loc_var_node *link = (struct loc_var_node *)malloc(sizeof(struct loc_var_node));

    // Allocate memory for the keyword and value strings, then copy them
    link->keywords = (char *)malloc(strlen(keyword) + 1);
    link->value = (char *)malloc(strlen(value) + 1);
    strcpy(link->keywords, keyword);
    strcpy(link->value, value);
    link->next = NULL;  // New node's next pointer is set to NULL

    // If the list is empty, initialize it with the new node
    if (head == NULL) {
        head = link;
        return;
    }

    // Otherwise, traverse to the end of the list and append the new node
    current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = link;
}

// ====================================
// 🔹 Main Function: Shell Execution Loop
// ====================================
int main() {
    char input_buf[BUF_SIZE];              // Buffer for user input
    char CWD_buf[LINUX_PWD_MAX];             // Buffer for storing the current working directory
    char *token;                           // Temporary pointer for tokenizing input
    int echo_words_counter = 0;            // Counter for number of words parsed from input
    char *passed_args[ECHO_MAX_WORDS];       // Array of pointers to hold the parsed arguments
    char *ext_passed_args[ECHO_MAX_WORDS - 1]; // Array of pointers for external command arguments
    int equal_counter;                     // Counter for '=' in input for local variable assignment
    char *delimeter = "=";                 // Delimiter used to split local variable assignments
    char *var_keyword;                     // Variable to hold the name extracted from a local variable assignment
    char *var_value;                       // Variable to hold the value extracted from a local variable assignment
    int expo_equal_counter;                // Counter for '=' in the export command argument

    while (1) {
        // ----------------------------------
        // 🔹 Display Shell Prompt
        // ----------------------------------
        echo_words_counter = 0;
        equal_counter = 0;
        expo_equal_counter = 0;
        printf("Femto Shell: %s >> ", getcwd(CWD_buf, sizeof(CWD_buf) - 1));

        fgets(input_buf, BUF_SIZE, stdin);
        input_buf[strlen(input_buf) - 1] = 0;
        if (strlen(input_buf) == 0)
            continue;

        // ----------------------------------
        // 🔹 Parse Input
        // ----------------------------------
        for (int j = 0; j < sizeof(ext_passed_args) / sizeof(ext_passed_args[0]); j++) {
            ext_passed_args[j] = NULL;
        }
        input_buf[strcspn(input_buf, "\n")] = '\0';
        token = strtok(input_buf, " ");
        while (token != NULL) {
            passed_args[echo_words_counter] = token;
            ext_passed_args[echo_words_counter] = token;
            token = strtok(NULL, " ");
            echo_words_counter++;
        }
        ext_passed_args[echo_words_counter] = NULL;

        // ----------------------------------
        // 🔹 Built-in Commands
        // ----------------------------------
        if (strcmp(passed_args[0], "echo") == 0) {
            for (int i = 1; i < echo_words_counter; i++) {
                // Check if the argument starts with '$' for variable substitution
                if (passed_args[i][0] == '$') {
                    get_env(passed_args[i] + 1);
                    printf("%s ", g_val);
                } else {
                    printf("%s ", passed_args[i]);
                }
            }
            printf("\n");
        }
        else if (strcmp(passed_args[0], "exit") == 0) {
            printf("Good Bye :)\n");
            break;
        }
        else if (strcmp(passed_args[0], "pwd") == 0) {
            printf("%s\n", getcwd(CWD_buf, sizeof(CWD_buf) - 1));
        }
        else if (strcmp(passed_args[0], "cd") == 0) {
            if (passed_args[1][0] == '$') {
                get_env(passed_args[1] + 1);
                if (chdir(g_val) == -1)
                    printf("Usage: cd /path\n");
            } else {
                if (chdir(passed_args[1]) == -1)
                    printf("Usage: cd /path\n");
            }
        }
        else if (strcmp(passed_args[0], "export") == 0) {
            // ----------------------------------
            // 🔹 Handle the Export Command
            // ----------------------------------
            // Count '=' characters in the argument for export
            for (char *ch = passed_args[1]; *ch != '\0'; ch++) {
                if (*ch == '=') {
                    expo_equal_counter++;
                }
                if (*ch == ' ') {
                    perror("Use correct format for env. variable\n");
                    expo_equal_counter++;
                    break;
                }
            }
            
            if (expo_equal_counter > 1) {
                continue;
            }
            else if (expo_equal_counter == 1) {
                var_keyword = strtok(passed_args[1], delimeter);
                if (var_keyword == NULL) {
                    perror("Use correct format for env. variable\n");
                    continue;
                }
                var_value = strtok(NULL, delimeter); // Continue tokenizing after delimiter
                if (var_value == NULL) {
                    perror("Use correct format for env. variable\n");
                    continue;
                }
                setenv(var_keyword, var_value, 1);  // Set the environment variable
            }
            else if (expo_equal_counter == 0) {
                get_env(passed_args[1]);
                setenv(passed_args[1], g_val, 1);
            }
            else if (strcmp(passed_args[1], "printenv") == 0) {
                print_env();
                printf("\n");
            }
        }
        // ----------------------------------
        // 🔹 External Commands
        // ----------------------------------
        else {
            if (fork() != 0) {
                wait(NULL); // Parent process waits for the child to finish
            } else {
                // Variable substitution for external command arguments
                for (char **ch = ext_passed_args; *ch != NULL; ch++) {
                    if ((*ch)[0] == '$') {
                        get_env(*ch + 1);
                        strcpy(*ch, g_val);
                    }
                }
                // First try executing with var_value (if set from local variable handling)
                execvp(var_value, ext_passed_args);
                // If the above fails, try executing using the first argument
                execvp(passed_args[0], ext_passed_args);
                
                // If execution reaches here, the command was not found
                printf("Cannot find command!!\n");
                exit(1);
            }
        }
        
        // ----------------------------------
        // 🔹 Local Variable Handling
        // ----------------------------------
        for (char *ch = passed_args[0]; *ch != '\0'; ch++) {
            if (*ch == '=') {
                equal_counter++;
            }
            if (*ch == ' ') {
                perror("Use correct format for local variable\n");
                equal_counter--;
                break;
            }
        }
        if (equal_counter == 1) {
            var_keyword = strtok(passed_args[0], delimeter);
            if (var_keyword == NULL) {
                perror("Use correct format for local variable\n");
                continue;
            }
            var_value = strtok(NULL, delimeter); // Continue tokenizing after delimiter
            if (var_value == NULL) {
                perror("Use correct format for local variable\n");
                continue;
            }
            set_loc_env(var_keyword, var_value);
        }
    }
    
    return 0;
}
