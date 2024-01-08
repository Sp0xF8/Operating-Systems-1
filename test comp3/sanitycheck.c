#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *directoryPath = "/home/spoon/Dev/README_(copy).md";  // Replace with the actual directory path


    // Attempt to open the file in write mode
    FILE *filePointer = fopen(directoryPath, "w");

    if (filePointer == NULL) {
        perror("Error");
        return 1;  // Indicates an error
    }

    printf("File created successfully: %s\n", directoryPath);

    // Close the file when you're done with it
    fclose(filePointer);

    return 0;
}