#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAXUID 1e6
#define MAXLINESIZE 500
#define MAXLENNAME 1000

// Map for the UID->LOGINID
char *loginId[(int)MAXUID + 1];

// Global counter
int counter = 0;

// Create the Hash map...
void uidToLoginId()
{
    // Open the /etc/passwd
    FILE *fptr = fopen("/etc/passwd", "r");
    if (fptr == NULL)
    {
        printf("Error opening /etc/passwd\n");
        exit(1);
    }

    // Read line by line and Parse it...
    char line[500];
    while (fgets(line, sizeof(line), fptr))
    {
        char *login;
        login = strtok(line, ":");
        strtok(NULL, ":");
        char *uid;
        uid = strtok(NULL, ":");

        int UID = atoi(uid);
        if (UID > MAXUID)
            continue;
        loginId[UID] = (char *)malloc((strlen(login) + 1) * sizeof(char));
        strcpy(loginId[UID], login);

        // printf("loginId[%d] = %s\n", UID, login);
    }

    fclose(fptr);
}

// Check if the str1[last len char] == str2[last len char]
int endsWith(char *str1, char *str2, int len)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    for (int i = 0; i < len; i++)
    {
        if (str1[len1 - i] != str2[len2 - i])
            return 0;
    }
    return 1;
}

/*Recursive Search*/
void findRecurse(char dir[], char extn[])
{

    DIR *dirPtr = opendir(dir);
    if (dirPtr == NULL)
    {
        printf("Error opening dir : %s\n", dir);
        perror("Error reason : ");
        return;
    }

    // Entry and FCB struct..
    struct dirent *entry;
    struct stat fileStat;

    /*Read all the Directory*/
    while ((entry = readdir(dirPtr)) != NULL)
    {
        // Avoid . and .. dir
        if (strcmp(entry->d_name, ".") == 0)
            continue;
        if (strcmp(entry->d_name, "..") == 0)
            continue;

        // Create the concatenated Directory Name(or file Name)
        char newName[3 * MAXLENNAME];
        sprintf(newName, "%s/%s", dir, entry->d_name);

        // Get the File Stat...
        if (lstat(newName, &fileStat) == 0)
        {
            // Check if it is Directory
            if (S_ISDIR(fileStat.st_mode))
            {
                findRecurse(newName, extn);
            }
            else
            {
                // If Regular File...
                if (S_ISREG(fileStat.st_mode))
                {
                    if (endsWith(entry->d_name, extn, 4) == 1)
                    {
                        counter++;
                        printf("%-3d : %-10s %-10ld %s\n", counter, loginId[fileStat.st_uid], fileStat.st_size, newName);
                    }
                }
            }
        }
        else
        {
            printf("Error getting the FCB : %s\n", entry->d_name);
            perror("Error reason.. : ");
        }
    }

    closedir(dirPtr);
}

/*Driver code...*/
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <dirName> <extn>\n", argv[0]);
        return 1;
    }

    printf("Received %s\n", argv[1]);

    // create the hash-Map...
    uidToLoginId();

    printf("NO  : OWNER      SIZE       NAME\n");
    printf("--  : ---------- ---------- ----------------------------------------\n");

    // Add . in front of Extension
    char *extn = (char *)malloc((strlen(argv[2]) + 1) * sizeof(char));
    strcpy(extn + 1, argv[2]);
    extn[0] = '.';

    // Call the Recurse Function...
    findRecurse(argv[1], extn);

    printf("+++ %d files match the extension %s\n", counter, argv[2]);

    return 0;
}
