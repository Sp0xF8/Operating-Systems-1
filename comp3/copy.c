#include <stdlib.h>
#include <stdio.h>
//include <dirent.h>
#include <string.h>

#define _DEBUG

struct CopyData { 

    char* source;
    int source_is_directory;
    char* destination;

    //if the source is a directory, these are used to store the directory path
    char* source_directory_path;
    DIR* source_directory;

    //if the destination exists, these are used to store the destination path and directory path
    char* destination_directory_path;
    DIR* destination_directory;

    //if the source is a file, these are used to store the file name and extension
    FILE* source_file;
    char* source_file_name;
    char* source_file_extension;
    char* source_name_w_extension;
    char* source_file_path;

    //cloned file data
    FILE* new_file;
    char* new_file_name;
    char* new_file_path;


};


int cycle_directory(struct CopyData* data);

int build_file_name(struct CopyData* data);

int copy_file(struct CopyData* data);
int write_file(struct CopyData* data, char* buffer, int buffer_size, int offset);

int null_object(struct CopyData* data);




int main(int argc, char* argv[]){
    struct CopyData* data = malloc(sizeof(struct CopyData));

    if(argc < 2 || argc > 3){
        printf("Invalid number of arguments.\n");
        printf("Usage: gcopy <source> <destination>\n");
        free(data);
        return 1;
    }

    data->source = argv[1];
    data->destination = argv[2];


    //check if source is a file
    data->source_file = fopen(data->source, "rb");

    if(data->source_file == NULL){
        //check if source is a directory
        data->source_directory = opendir(data->source);

        if(data->source_directory == NULL){
            printf("Source does not exist.\n");
            free(data);
            return 1;
        } else {
            data->source_is_directory = 1;
        }
    } 

    //check if destination exists
    data->destination_directory = opendir(data->destination);

    if(data->destination_directory == NULL){
        printf("Destination does not exist.\n");
        free(data);
        return 1;
    }

    //close the file and directory pointers
    if(data->source_file != NULL){
        fclose(data->source_file);
        data->source_file_path = data->source; //set the source file path to the source (since it is a file)
    }
    
    if(data->source_is_directory == 1){
        cycle_directory(data);
    } else {
        copy_file(data);
    }




    free(data);
    return 0;
}


int null_object(struct CopyData* data){

    data->new_file = NULL;
    data->new_file_name = NULL;
    data->new_file_path = NULL;

    data->source_file = NULL;
    data->source_file_path = NULL;
    data->source_file_name = NULL;
    data->source_file_extension = NULL;
    data->source_name_w_extension = NULL;

    return 0;
}

int build_file_name(struct CopyData* data){

    //define local variables needed
    char* last_slash = strrchr(data->source_file_path, '/');
    char* last_dot = strrchr(data->source_file_path, '.');
    char* name_pos = NULL;
    
    if(last_slash == NULL){
        name_pos = data->source_file_path;
    } else {
        name_pos = last_slash + 1;
    }



    if(last_dot == NULL){

        //get length of the file name
        size_t filename_length = strlen(name_pos);


        data->source_file_name = malloc(filename_length + 1); //+1 for null terminator
        if(data->source_file_name == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for source file name.");
            #endif

            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for source file name.\n", filename_length + 1); //+1 for null terminator
        #endif

        strncpy(data->source_file_name, name_pos, filename_length);
        data->source_file_name[filename_length] = '\0';

        data->new_file_name = malloc(filename_length + 8); //+8 for null terminator  + 1
        if(data->new_file_name == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for new file name.");
            #endif

            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for new file name.\n", filename_length + 8); //+8 for null terminator  + 1
        #endif

        snprintf(data->new_file_name, filename_length + 8, "%s_(copy)", data->source_file_name);
        data->new_file_name[filename_length + 7] = '\0';
        
    } else {
        size_t extension_length =  strlen(last_dot + 1);
        size_t filename_length =  strlen(name_pos) - strlen(last_dot);

        printf("Extension length: %d\n", extension_length);
        printf("Filename length: %d\n", filename_length);

        data->source_file_extension = malloc(extension_length + 1); //+1 for null terminator
        if(data->source_file_extension == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for source file extension.");
            #endif

            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for source file extension.\n", extension_length + 1); //+1 for null terminator
        #endif

        strncpy(data->source_file_extension, last_dot + 1, extension_length);
        data->source_file_extension[extension_length] = '\0';

        data->source_file_name = malloc(filename_length + 1); //+1 for null terminator
        if(data->source_file_name == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for source file name.");
            #endif

            free(data->source_file_extension);
            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for source file name.\n", filename_length + 1); //+1 for null terminator
        #endif

        strncpy(data->source_file_name, name_pos, filename_length);
        data->source_file_name[filename_length] = '\0';

        data->source_name_w_extension = malloc(filename_length + extension_length + 1); //+1 for null terminator
        if(data->source_name_w_extension == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for source file name with extension.");
            #endif

            free(data->source_file_extension);
            free(data->source_file_name);
            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for source file name with extension.\n", filename_length + extension_length + 1); //+1 for null terminator
        #endif

        strncpy(data->source_name_w_extension, name_pos, filename_length + extension_length + 1);
        data->source_name_w_extension[filename_length + extension_length] = '\0';


        data->new_file_name = malloc(filename_length + extension_length + 9); //+9 for null terminator, +1 for underscorem bracket, and period
        
        if(data->new_file_name == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for new file name.");
            #endif

            free(data->source_file_extension);
            free(data->source_file_name);
            free(data->source_name_w_extension);
            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for new file name.\n", filename_length + extension_length + 9); //+9 for null terminator, +1 for underscorem bracket, and period
        #endif

        snprintf(data->new_file_name, filename_length + extension_length + 9, "%s_(copy).%s", data->source_file_name, data->source_file_extension);
        data->new_file_name[filename_length + extension_length + 8] = '\0';


    }

    char* dest_slash = strrchr(data->destination, '/');

    if(dest_slash != NULL){
        if(dest_slash == data->destination + strlen(data->destination) - 1){

            data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 1); //+1 for null terminator
            if(data->new_file_path == NULL){
                #ifdef _DEBUG
                    perror("Failed to allocate memory for new file path.");
                #endif

                if (last_dot != NULL){
                    free(data->source_file_extension);
                    free(data->source_name_w_extension);
                    
                }

                free(data->source_file_name);
                free(data->new_file_name);
                return 1;
            } 

            #ifdef _DEBUG
                printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 1); //+1 for null terminator
            #endif

            snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 1, "%s%s", data->destination, data->new_file_name);
            data->new_file_path[strlen(data->destination) + strlen(data->new_file_name)] = '\0';


        } else {

            data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 2);
            if(data->new_file_path == NULL){
                #ifdef _DEBUG
                    perror("Failed to allocate memory for new file path.");
                #endif

                if (last_dot != NULL){
                    free(data->source_file_extension);
                    free(data->source_name_w_extension);
                    
                }

                free(data->source_file_name);
                free(data->new_file_name);
                return 1;
            }

            #ifdef _DEBUG
                printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 2);
            #endif

            snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 2, "%s/%s", data->destination, data->new_file_name);
            data->new_file_path[strlen(data->destination) + strlen(data->new_file_name) + 1] = '\0';
        }
    } else {

        data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 2);
        if(data->new_file_path == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for new file path.");
            #endif

            if (last_dot != NULL){
                free(data->source_file_extension);
                free(data->source_name_w_extension);
                
            }

            free(data->source_file_name);
            free(data->new_file_name);
            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 2);
        #endif

        snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 2, "%s/%s", data->destination, data->new_file_name);
        data->new_file_path[strlen(data->destination) + strlen(data->new_file_name) + 1] = '\0';
    }

    #ifdef _DEBUG
        printf("Original file name: %s\n", data->source_file_name);
        printf("New file name: %s\n", data->new_file_name);
        printf("New file path: %s\n", data->new_file_path);
    #endif
    

    return 0;

}


int cycle_directory(struct CopyData* data){
    #ifdef _DEBUG
        printf("Copying directory...\n");
    #endif

    struct dirent* current_file = NULL;
    char* last_slash = strrchr(data->source, '/');

    while((current_file = readdir(data->source_directory)) != NULL) {
        if(strcmp(current_file->d_name, ".") == 0 || strcmp(current_file->d_name, "..") == 0){ //skip . and ..
            continue;
        }

        
    }
}


int copy_file(struct CopyData* data){
    #ifdef _DEBUG
        printf("Copying file...\n");
    #endif

    if(build_file_name(data) == 1){
        #ifdef _DEBUG
            printf("Failed to build file name.\n");
        #endif

        return 1;
    }

    #ifdef _DEBUG
        printf("File name built.\n");
    #endif


    data->source_file = fopen(data->source_file_path, "rb");

    if(data->source_file == NULL){
        #ifdef _DEBUG
            perror("Failed to open source file.");
        #endif

        return 1;
    }

    #ifdef _DEBUG
        printf("Source file opened.\n");
    #endif

    //get file size
    fseek(data->source_file, 0, SEEK_END);
    long file_size = ftell(data->source_file);
    fseek(data->source_file, 0, SEEK_SET);

    #ifdef _DEBUG
        printf("File size: %d\n", file_size);
    #endif

    //if file size is less than 10MB, read the whole file into a buffer and write it to the new file

    if(file_size < 10485760){
        #ifdef _DEBUG
            printf("File is small, reading into buffer...\n");
        #endif

        char* buffer = malloc(file_size);
        if(buffer == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for buffer.");
            #endif

            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes, (%f MB) for buffer.\n", file_size, (float)file_size / 1048576);
        #endif

        fread(buffer, file_size, 1, data->source_file);

        #ifdef _DEBUG
            printf("Buffer filled.\n");
        #endif

        if(write_file(data, buffer, file_size, 0) == 1){
            #ifdef _DEBUG
                printf("Failed to write file.\n");
            #endif

            free(buffer);
            return 1;
        }

        #ifdef _DEBUG
            printf("File written.\n");
        #endif

        free(buffer);
        return 0;
    } else {
        #ifdef _DEBUG
            printf("File is large, reading in chunks...\n");
        #endif

        char* buffer = malloc(10485760);
        if(buffer == NULL){
            #ifdef _DEBUG
                perror("Failed to allocate memory for buffer.");
            #endif

            return 1;
        }

        #ifdef _DEBUG
            printf("Allocated %d bytes for buffer.\n", 10485760);
        #endif

        int offset = 0;
        int bytes_read = 0;

        while(offset < file_size){
            if(offset + 10485760 > file_size){
                bytes_read = fread(buffer, file_size - offset, 1, data->source_file);
            } else {
                bytes_read = fread(buffer, 10485760, 1, data->source_file);
            }

            if(bytes_read == 0){
                #ifdef _DEBUG
                    perror("Failed to read file.");
                #endif

                free(buffer);
                return 1;
            }

            if(write_file(data, buffer, bytes_read, offset) == 1){
                #ifdef _DEBUG
                    printf("Failed to write file.\n");
                #endif

                free(buffer);
                return 1;
            }

            offset += bytes_read;
        }

        #ifdef _DEBUG
            printf("File written.\n");
        #endif

        free(buffer);
        return 0;
    }




    

    return 0;
}

int write_file(struct CopyData* data, char* buffer, int buffer_size, int offset){
    #ifdef _DEBUG
        printf("Writing file...");
    #endif


    if(data->new_file == NULL){
        #ifdef _DEBUG
            printf("File is not open.\n");
        #endif

        if(offset == 0){
            data->new_file = fopen(data->new_file_path, "wb");
        } else {
            data->new_file = fopen(data->new_file_path, "ab");
        }

        if(data->new_file == NULL){
            #ifdef _DEBUG
                perror("Failed to open new file.");
            #endif
            return 1;
        }

    } 

    #ifdef _DEBUG
        printf("File is open.\n");
    #endif

    if(offset == 0){
        fwrite(buffer, buffer_size, 1, data->new_file);
    } else {
        fseek(data->new_file, offset, SEEK_SET);
        fwrite(buffer, buffer_size, 1, data->new_file);
    }

    #ifdef _DEBUG
        printf("File written.\n");
    #endif

    return 0;
    
}