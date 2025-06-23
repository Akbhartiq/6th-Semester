#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

// Function to print the Message
void print(int node, int count, int arr[])
{
    // Count represents the number of children
    // Arr stores the Children
    if (count == 0)
    {
        char buff[200];
        sprintf(buff, "foo%d rebuilt", node);
        printf("%s\n", buff);
    }
    else
    {
        char buff1[200];
        sprintf(buff1, "foo%d rebuilt from foo%d", node, arr[1]);
        printf("%s ", buff1);
        for (int i = 2; i <= count; i++)
        {
            char buff2[200];
            sprintf(buff2, ", foo%d", arr[i]);
            printf("%s", buff2);
        }
        printf("\n");
    }
}

// Driver code
int main(int argc, char *argv[])
{
    char *node_char = argv[1];  // Get the node to be rebuid
    int node = atoi(node_char); // Convert it to Integer

    FILE *file;                        // File ptr
    const char *filename = "done.txt"; // Done.txt

    // Check if the Call is for the Root
    if (argc == 2)
    {
        // First get the Number of the foo-packets
        FILE *foodep;
        const char *foodepFile = "foodep.txt";
        foodep = fopen(foodepFile, "r");

        // Read the value of n from the foodep.txt
        char n_char[100];
        fgets(n_char, sizeof(n_char), foodep);
        int n_val = atoi(n_char);

        // Close the foodep file
        fclose(foodep);

        // Create the visiting array
        char data[n_val + 1];
        for (int i = 0; i < n_val; i++)
            data[i] = '0';
        data[n_val] = '\0';

        // Open the data.txt and write zeros
        file = fopen(filename, "w");
        if (file == NULL)
        {
            printf("Error opening the File\n");
            exit(0);
        }
        // Write the data into the file
        fputs(data, file);

        // Close the file
        fclose(file);
    }

    // File Handling of foodep.txt
    FILE *foo_file;
    const char *foo_filename = "foodep.txt";
    foo_file = fopen(foo_filename, "r");

    // Check for the Child
    char line[256];               // Buffer to hold each line
    char target[10];              // Buffer to hold the formatted node
    sprintf(target, "%d:", node); // Formatted as node: (to match)

    // Allocate the memory for the Integer array
    int numbers[256];
    int count = 0; // Representing the number of the child

    // Read the file line by line
    while (fgets(line, sizeof(line), foo_file))
    {
        // Check if the line starts with target key
        if (strncmp(line, target, strlen(target)) == 0)
        {

            // Tokenize the line to extract numbers (skip the key and colon)
            char *token = strtok(line + strlen(target), " ");

            // Store the key at the 0th position
            numbers[count++] = node;

            // Parse the remaining numbers
            while (token != NULL)
            {

                int val = atoi(token); // Convert token to integer and store
                if (val)
                {
                    // To avoid converting empty strings to zero
                    numbers[count++] = val;
                }
                token = strtok(NULL, " "); // Move to the next token
            }

            break;
        }
    }

    // close the foo-file
    fclose(foo_file);

    count--; // To remove the count of the parent

    // Read the status of this child
    file = fopen(filename, "rw");

    // Start from 1 to avoid the parent itself
    for (int i = 1; i <= count; i++)
    {

        // Move the file-Ptr to the Desired Location
        int pos = numbers[i] - 1;
        fseek(file, pos, SEEK_SET);

        // Read the status of the node
        char ch;
        fread(&ch, sizeof(char), 1, file);

        fflush(file);

        int status = ch - '0';

        if (status == 0)
        {
            pid_t cpid = fork();
            if (cpid == 0)
            {
                // Child Process
                char child_node[20];
                sprintf(child_node, "%d", numbers[i]);
                char *arg[] = {"./rebuild", child_node, "child", NULL};

                // Rebuild the child
                execv(arg[0], arg);
            }
            else
            {
                // Parent-Process
                waitpid(cpid, NULL, 0);
            }
        }
    }

    // Close the File
    fclose(file);

    // Print the buildin Information
    print(node, count, numbers);

    // Update the Status of this Node
    file = fopen(filename, "r+");

    // Move the ptr in the file
    fseek(file, node - 1, SEEK_SET);

    // Write 1 as status
    fputc('1', file);

    fflush(file);

    fclose(file);

    // Return
    return 0;
}