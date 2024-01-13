#include <stdlib.h>
#include <stdio.h>
//include <dirent.h> 			//DONT FORGET TO RE-ENABLE THIS WHEN DONE DEVELOPING
//include <sys/stat.h> 			//DONT FORGET TO RE-ENABLE THIS WHEN DONE DEVELOPING
#include <string.h>

#define _DEBUG



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	STRUCT DECLIRATIONS
//
//			USAGE:
//				- This structure is designed to hold all the data needed to copy a file.
//				- It is also used as an easier way to pass data between functions.
//				- It is used to ensure that all the data needed is in one place, and to ensure that the data is not lost.
//			
//			Tips:
//				- Reccomended to be used as a pointer
//
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FUNCTION DECLIRATIONS
//

int cycle_directory(struct CopyData* data);
int build_file_name(struct CopyData* data);
int copy_file(struct CopyData* data);
int write_file(struct CopyData* data, char* buffer, int buffer_size, int offset);
int null_object(struct CopyData* data);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (main)
//
//		Takes Arguments:
//			- char* Source
//			- char* Destination
//
//		Does:
//			- Checks if the correct number of arguments are passed
//			- Checks if the source & destination exists
//			- Checks if the source is a directory or a file
//			- Frees the memory allocated for the data struct
//
int main(int argc, char* argv[]){
	struct CopyData* data = malloc(sizeof(struct CopyData)); //											allocate memory on the heap for the data struct

	// Sanity check to ensure memory is loaded
	if(data == NULL){ //																				if memory is not allocated
		#ifdef _DEBUG //																					if debug is enabled
			perror("Failed to allocate memory for data struct."); //											print error
		#endif

		return 1; //																						return 1 to indicate failure
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Sanity check to ensure the correct number of arguments are passed
//	
	if(argc != 3){ //																					if the correct number of arguments are not passed
		printf("Invalid number of arguments.\n"); //														print error
		perror("Usage: gcopy <source> <destination>\n"); //													print correct usage
		free(data); //																						free the memory allocated for the data struct
		return 1; //																						return 1 to indicate failure
	}

	data->source = argv[1]; //																			set the source path in the data struct
	data->destination = argv[2]; //																		set the destination path in the data struct


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Check if source exists
//
	struct stat source_stat; //																			declare a stat struct to hold the source file stats
	if(stat(data->source, &source_stat) != 0){ //															if the source does not exist
		perror("Source does not exist.\n"); //																	print error
		free(data); //																							free the memory allocated for the data struct
		return 1; //																							return 1 to indicate failure
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Check if destination exists
//
	struct stat destination_stat; //																		declare a stat struct to hold the destination file stats
	if(stat(data->destination, &destination_stat) != 0){ //													if the destination does not exist
		perror("Destination does not exist.\n"); //																print error
		free(data); //																							free the memory allocated for the data struct
		return 1; //																							return 1 to indicate failure
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Check if Source is a directory
//
	if(S_ISDIR(source_stat.st_mode)){ //																	if the source is a directory
		#ifdef _DEBUG //																						if debug is enabled
			printf("Source is a directory.\n"); //																	print message
		#endif

		data->source_is_directory = 1; //																		set the source is directory flag in the data struct to true

		cycle_directory(data); //																				cycle through the directory
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Check if Source is a file
//
	if(S_ISREG(source_stat.st_mode)){ //																	if the source is a file
		#ifdef _DEBUG //																						if debug is enabled
			printf("Source is a file.\n"); //																		print message
		#endif 

		data->source_is_directory = 0; //																		set the source is directory flag in the data struct to false

		data->source_file_path = data->source; //																set the source file path in the data struct
		copy_file(data); //																						copy the file
	}


	free(data);  //																							free the memory allocated for the data struct
	return 0; //																							return 0 to indicate success
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (null_object)
//
//		Takes Arguments:
//			- struct CopyData* data
//
//		Does:
//			- Sets all pointers in the data struct to NULL
//
//	NOTE: This function is used to ensure that the data struct is only holding useful data
//
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (build_file_name)
//
//		Takes Arguments:
//			- struct CopyData* data
//
//		Does:
//			- Extracts the file name from the source file path
//			- Extracts the file extension from the source file path
//			- Builds the new file name from the origonal file name, adding _(copy) to the end
//			- Builds the new file path from the destination path and the new file name
//
int build_file_name(struct CopyData* data){

	printf("\n\n====================================\nBuilding file name...\n");

	//define local variables needed
	char* last_slash = strrchr(data->source_file_path, '/'); //										find the last slash in the file path
	char* last_dot = strrchr(data->source_file_path, '.'); //										find the last dot in the file path
	char* name_pos = NULL; //																		declare a pointer to hold the position of the file name in the file path

	if(last_slash == NULL){ //																		if there is no slash in the file path
		name_pos = data->source_file_path; //															the file name starts at the beginning of the file path
	} else { //																						if there is a slash in the file path
		name_pos = last_slash + 1; //																	the file name starts after the last slash
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//		BASE LOGIC OF THE FOLLOWING IF STATEMENT
	//
	//			if there is no dot in the file name, there is no extension
	//				the file name is the string from the beginning of the file name to the end of the string
	//				the new file name is  "<file name>_(copy)"
	//			if there is a dot in the file name, there is an extension
	//				the extension is the string from the pos after the last dot to the end of the string
	//				the file name is the string from the beginning of the file name to the last dot
	//				the file name with extension is the string from the beginning of the file name to the end of the string
	//				the new file name is "<file name>_(copy).<extension>"
	//
	//


	if(last_dot == NULL){ //																		if there is no period in the origonal file name, there is no extension

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Get the file name
	//

		size_t filename_length = strlen(name_pos); // 													gets the string length from the beginning of the file name

		data->source_file_name = malloc(filename_length + 1); // 										Allocate memory on the heap for the origonal file name +1 for null terminator
		// Sanity check to ensure memory is loaded
		if(data->source_file_name == NULL){ //															if memory is not allocated
			#ifdef _DEBUG //																				if debug is enabled
				perror("Failed to allocate memory for source file name."); //									print error
			#endif

			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source file name.\n", filename_length + 1);
		#endif

		strncpy(data->source_file_name, name_pos, filename_length); //									copy the origonal file name into the data struct
		data->source_file_name[filename_length] = '\0'; //												add null terminator to the end of the string

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Build the new file name
	//
		data->new_file_name = malloc(filename_length + 8); // 											Allocate memory on the heap for the new file name +1 for null terminator + "_(copy)"
		// Sanity check to ensure memory is loaded
		if(data->new_file_name == NULL){ //																if memory is not allocated
			#ifdef _DEBUG //																				if debug is enabled
				perror("Failed to allocate memory for new file name."); //										print error
			#endif

			free(data->source_file_name); //																free the memory allocated for the origonal file name

			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for new file name.\n", filename_length + 8);
		#endif

		snprintf(data->new_file_name, filename_length + 8, "%s_(copy)", data->source_file_name); //		build the new file name with the format "<filename>_(copy)"
		data->new_file_name[filename_length + 7] = '\0'; //												add null terminator to the end of the string

	} else { //																						if there is a dot in the file name, there is an extension

		// Defines for readability
		size_t extension_length =  strlen(last_dot + 1); // 											gets the string length from the pos after the last dot
		size_t filename_length =  strlen(name_pos) - strlen(last_dot); //								gets the string length from the beginning of the file name to the last dot


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Get the file extension
	//

		data->source_file_extension = malloc(extension_length + 1); //									Allocate memory on the heap for the extension +1 for null terminator

		// Sanity check to ensure memory is loaded
		if(data->source_file_extension == NULL){ //														if memory is not allocated
			#ifdef _DEBUG	//																				if debug is enabled
				perror("Failed to allocate memory for source file extension."); //								print error
			#endif

			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source file extension.\n", extension_length + 1);
		#endif

		strncpy(data->source_file_extension, last_dot + 1, extension_length); //						copy the extension into the data struct
		data->source_file_extension[extension_length] = '\0';//											add null terminator to the end of the string


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Get the origonal file name
	//

		data->source_file_name = malloc(filename_length + 1); // 										Allocate memory on the heap for the file name +1 for null terminator

		// Sanity check to ensure memory is loaded
		if(data->source_file_name == NULL){ //															if memory is not allocated
			#ifdef _DEBUG //																				if debug is enabled
				perror("Failed to allocate memory for source file name.");//									print error
			#endif

			free(data->source_file_extension);//															free the memory allocated for the extension
			return 1;	//																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source file name.\n", filename_length + 1);
		#endif

		strncpy(data->source_file_name, name_pos, filename_length); //									copy the file name into the data struct
		data->source_file_name[filename_length] = '\0'; //												add null terminator to the end of the string


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Get the origonal file name with extension
	//

		data->source_name_w_extension = malloc(filename_length + extension_length + 1); //				Allocate memory on the heap for the file name + extemsion +1 for null terminator
		// Sanity check to ensure memory is loaded
		if(data->source_name_w_extension == NULL){ //													if memory is not allocated
			#ifdef _DEBUG	//																				if debug is enabled
				perror("Failed to allocate memory for source file name with extension."); //					print error
			#endif

			free(data->source_file_extension); //															free the memory allocated for the extension
			free(data->source_file_name); //																free the memory allocated for the file name
			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source file name with extension.\n", filename_length + extension_length + 1);
		#endif

		strncpy(data->source_name_w_extension, name_pos, filename_length + extension_length + 1); //	copy the file name into the data struct
		data->source_name_w_extension[filename_length + extension_length] = '\0'; //					add null terminator to the end of the string


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 		Build the new file name
	//

		data->new_file_name = malloc(filename_length + extension_length + 9); //  						Allocate memory on the heap for the new file name + extension +1 for null terminator + "_(copy)."
		// Sanity check to ensure memory is loaded
		if(data->new_file_name == NULL){ //																if memory is not allocated
			#ifdef _DEBUG //																				if debug is enabled
				perror("Failed to allocate memory for new file name."); //										print error
			#endif

			free(data->source_file_extension); //															free the memory allocated for the extension
			free(data->source_file_name); //																free the memory allocated for the source file name
			free(data->source_name_w_extension); //															free the memory allocated for the source file name with extension
			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for new file name.\n", filename_length + extension_length + 9);
		#endif

		snprintf(data->new_file_name, filename_length + extension_length + 9,
				"%s_(copy).%s", data->source_file_name, data->source_file_extension); //				build the new file name with the format "<filename>_(copy).<extension>"
		data->new_file_name[filename_length + extension_length + 8] = '\0';//							add null terminator to the end of the string
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//      AT THIS POINT THE NEW FILE NAME IS BUILT, READY TO BE CRAFTED INTO A NEW FILE PATH
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	char* dest_slash = strrchr(data->destination, '/');//											find the last slash in the destination path

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//		BASE LOGIC OF THE FOLLOWING IF STATEMENT
	//
	//			if there is a slash in the destination path
	//				if the slash is at the end of the destination path
	//					build the new file path with the format "<destination path><new file name>"
	//				if the slash is not at the end of the destination path
	//					build the new file path with the format "<destination path>/<new file name>"
	//			if there is no slash in the destination path
	//				build the new file path with the format "<destination path>/<new file name>"
	//
	//			NOTE: the slash is added to the end of the destination path if it is not already there, this is to ensure the new file path is built correctly
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(dest_slash != NULL){ //																		if there is a slash in the destination path
		if(dest_slash == data->destination + strlen(data->destination) - 1){ //							if the slash is at the end of the destination path

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		//	Build the new file path, without adding a slash to the end of the destination path for the format "<destination path><new file name>"
		//
			data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 1); //	Allocate memory on the heap for the new file path +1 for null terminator
			// Sanity check to ensure memory is loaded
			if(data->new_file_path == NULL){ //																if memory is not allocated
				#ifdef _DEBUG //																				if debug is enabled
					perror("Failed to allocate memory for new file path."); //										print error
				#endif

				// Check to see what needs to be freed
				if (last_dot != NULL){ //																		if there is a dot in the file name
					free(data->source_file_extension); //															free the memory allocated for the extension
					free(data->source_name_w_extension); //															free the memory allocated for the source file name with extension
				}

				free(data->source_file_name); //																free the memory allocated for the source file name
				free(data->new_file_name); //																	free the memory allocated for the new file name
				return 1; //																					return 1 to indicate failure
			}

			#ifdef _DEBUG
				printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 1);
			#endif

			snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 1,
					"%s%s", data->destination, data->new_file_name); //										build the new file path with the format "<destination path><new file name>"
			data->new_file_path[strlen(data->destination) + strlen(data->new_file_name)] = '\0'; //			add null terminator to the end of the string

		} else { //																						if the slash is not at the end of the destination path

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		//	Build the new file path, adding a slash to the end of the destination path for the format "<destination path>/<new file name>"
		//
			data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 2); //	Allocate memory on the heap for the new file path +1 for null terminator +1 for slash
			// Sanity check to ensure memory is loaded
			if(data->new_file_path == NULL){ //																if memory is not allocated
				#ifdef _DEBUG //																				if debug is enabled
					perror("Failed to allocate memory for new file path.");//										print error
				#endif

				// Check what needs to be freed
				if (last_dot != NULL){ //																		if there is a dot in the file name
					free(data->source_file_extension); //															free the memory allocated for the extension
					free(data->source_name_w_extension); //															free the memory allocated for the source file name with extension
				}

				free(data->source_file_name); //																free the memory allocated for the source file name
				free(data->new_file_name); //																	free the memory allocated for the new file name
				return 1; //																					return 1 to indicate failure
			}

			#ifdef _DEBUG
				printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 2);
			#endif

			snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 2,
					"%s/%s", data->destination, data->new_file_name); //										build the new file path with the format "<destination path>/<new file name>"
			data->new_file_path[strlen(data->destination) + strlen(data->new_file_name) + 1] = '\0'; //			add null terminator to the end of the string
		}
} else { //																							if there is no slash in the destination path

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Build the new file path, adding a slash to the end of the destination path for the format "<destination path>/<new file name>"
	//
		data->new_file_path = malloc(strlen(data->destination) + strlen(data->new_file_name) + 2); //	Allocate memory on the heap for the new file path +1 for null terminator +1 for slash
		// Sanity check to ensure memory is loaded
		if(data->new_file_path == NULL){ //																if memory is not allocated
			#ifdef _DEBUG //																				if debug is enabled
				perror("Failed to allocate memory for new file path."); //										print error
			#endif

			// Check what needs to be freed
			if (last_dot != NULL){ //																		if there is a dot in the file name
				free(data->source_file_extension); //															free the memory allocated for the extension
				free(data->source_name_w_extension); //															free the memory allocated for the source file name with extension
			}

			free(data->source_file_name); //																free the memory allocated for the source file name
			free(data->new_file_name); //																	free the memory allocated for the new file name
			return 1; //																					return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for new file path.\n", strlen(data->destination) + strlen(data->new_file_name) + 2);
		#endif

		snprintf(data->new_file_path, strlen(data->destination) + strlen(data->new_file_name) + 2,
			"%s/%s", data->destination, data->new_file_name); //										build the new file path with the format "<destination path>/<new file name>"
		data->new_file_path[strlen(data->destination) + strlen(data->new_file_name) + 1] = '\0'; //		add null terminator to the end of the string
	}

	#ifdef _DEBUG
		printf("Original file name: %s\n", data->source_name_w_extension);
		printf("New file name: %s\n", data->new_file_name);
		printf("New file path: %s\n", data->new_file_path);
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 	Cleanup
//
	// Check what needs to be freed
	if (last_dot != NULL){ //																		if there is a dot in the file name
		free(data->source_file_extension); //															free the memory allocated for the extension
		free(data->source_name_w_extension); //															free the memory allocated for the source file name with extension
	}

	free(data->source_file_name); //																free the memory allocated for the source file name
	free(data->new_file_name); //																	free the memory allocated for the new file name

	#ifdef _DEBUG
		printf("Memory freed.\n");
		printf("File name finished building.\n====================================\n\n");
	#endif


	return 0;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (cycle_directory)
//
//		Takes Arguments:
//			- struct CopyData* data
//
//		Does:
//			- Creates a uniform directory path to be used later
//			- Opens the directory
//			- Cycles through the directory
//			- Copies each file in the directory
//


int cycle_directory(struct CopyData* data){
	#ifdef _DEBUG
		printf("Copying directory...\n");
	#endif

	struct dirent* current_file = NULL; //																declare a dirent struct to hold the current file (directory entry)
	char* last_slash = strrchr(data->source, '/'); //													find the last slash in the source path


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		BASE LOGIC OF THE FOLLOWING IF STATEMENT
//
//			if the slash is at the end of the source path
//				allocate memory for the source directory path
//			if the slash is not at the end of the source path
//				allocate memory for the source directory path +1 for slash
//
	if (last_slash == strlen(data->source) - 1){ //														if the slash is at the end of the source path

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Build the source directory path
	//
		data->source_directory_path = malloc(strlen(data->source) + 1); //									Allocate memory on the heap for the source directory path +1 for null terminator
		// Sanity check to ensure memory is loaded
		if(data->source_directory_path == NULL){ //															if memory is not allocated
			#ifdef _DEBUG //																					if debug is enabled
				perror("Failed to allocate memory for source file path."); //										print error
			#endif

			return 1; //																						return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source file path.\n", strlen(data->source) + 1);
		#endif

		strncpy(data->source_directory_path, data->source, strlen(data->source)); //						copy the source path into the data struct
		data->source_directory_path[strlen(data->source)] = '\0'; //										add null terminator to the end of the string
	} else { //																							if the slash is not at the end of the source path

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Build the source directory path
	//
		data->source_directory_path = malloc(strlen(data->source) + 2); //									Allocate memory on the heap for the source directory path +1 for null terminator +1 for slash
		// Sanity check to ensure memory is loaded
		if(data->source_directory_path == NULL){ //															if memory is not allocated
			#ifdef _DEBUG //																					if debug is enabled
				perror("Failed to allocate memory for source directory path."); //									print error
			#endif

			return 1; //																						return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for source directory path.\n", strlen(data->source) + 2);
		#endif

		snprintf(data->source_directory_path, strlen(data->source) + 2, "%s/", data->source); //			build the source directory path with the format "<source path>/"
		data->source_directory_path[strlen(data->source) + 1] = '\0'; //									add null terminator to the end of the string
	}

	#ifdef _DEBUG
		printf("Source directory path: %s\n", data->source_directory_path);
	#endif

	data->source_directory = opendir(data->source_directory_path); //									open the source directory

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		BASE LOGIC OF THE FOLLOWING WHILE LOOP
//
//			while there are still files in the directory
//				if the current file is . or .., skip it
//				otherwise, copy the file
//				close the file
//				free the memory allocated for the source file path
//				null the data struct
//
	while((current_file = readdir(data->source_directory)) != NULL) { //											while there are still files in the directory


		if(strcmp(current_file->d_name, ".") == 0 || strcmp(current_file->d_name, "..") == 0){ //						if the current file is . or ..
			continue; //																									skip it
		}

		#ifdef _DEBUG
			printf("\n\n===============================================\n Current file: %s\n===============================================\n\n", current_file->d_name);
		#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Build the source file path
	//
		data->source_file_path = malloc(strlen(data->source_directory_path) + strlen(current_file->d_name) + 1); //		Allocate memory on the heap for the source file path +1 for null terminator
		// Sanity check to ensure memory is loaded
		if(data->source_file_path == NULL){ //																			if memory is not allocated
			#ifdef _DEBUG //																								if debug is enabled
				perror("Failed to allocate memory for source file path."); //													print error
			#endif

			return 1; //																									return 1 to indicate failure
		}

		snprintf(data->source_file_path, strlen(data->source_directory_path) + strlen(current_file->d_name) + 1,
				"%s%s", data->source_directory_path, current_file->d_name); //											build the source file path with the format "<source directory path><current file name>"
		data->source_file_path[strlen(data->source_directory_path) + strlen(current_file->d_name)] = '\0'; //			add null terminator to the end of the string

		#ifdef _DEBUG
			printf("Source file path: %s\n", data->source_file_path);
		#endif


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Copy the current file
	//
		if(copy_file(data) == 1){ //																					if the file is not copied successfully

			perror("Failed to copy file.\n"); //																			print error


			free(data->source_file_path); //																				free the memory allocated for the source file path
			return 1; //																									return 1 to indicate failure
		}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Cleanup
	//
		fclose(data->source_file); //																					close the file
		free(data->source_file_path); //																				free the memory allocated for the source file path
		null_object(data); //																							null the data struct

		#ifdef _DEBUG
			printf("\n\n===============================================\n\n"); //											print a line break for readability
		#endif

	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (copy_file)
//
//		Takes Arguments:
//			- struct CopyData* data
//
//		Does:
//			- Calls build_file_name to build the file name
//			- Opens the source file
//			- Gets the file size
//			- If the file size is less than 10MB, reads the whole file into a buffer and writes it to the new file
//			- If the file size is greater than 10MB, reads the file in 10MB chunks and writes them to the new file
//
int copy_file(struct CopyData* data){
	#ifdef _DEBUG
		printf("Copying file...\n");
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Build the file name and open the source file in read binary mode
//
	if(build_file_name(data) == 1){ //																				if the file name is not built successfully
		#ifdef _DEBUG //																								if debug is enabled
			printf("Failed to build file name.\n"); //																		print error
		#endif

		return 1; //																									return 1 to indicate failure
	}


	data->source_file = fopen(data->source_file_path, "rb"); //														open the source file in read binary mode

	if(data->source_file == NULL){ //																				if the file is not opened successfully
		#ifdef _DEBUG //																								if debug is enabled
			perror("Failed to open source file."); //																		print error
		#endif

		return 1; //																									return 1 to indicate failure
	}

	#ifdef _DEBUG
		printf("Source file opened.\n");
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Read the file size
//
	fseek(data->source_file, 0, SEEK_END); //																			go to the end of the file
	long file_size = ftell(data->source_file); //																		get the position of the "cursor" in the file
	fseek(data->source_file, 0, SEEK_SET); //																			go back to the beginning of the file, so it can be read properly

	#ifdef _DEBUG
		printf("File size: %d\n", file_size);
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 	BASE LOGIC OF THE FOLLOWING IF STATEMENT
//
// 		if the file size is less than 10MB
//			read the whole file into a buffer
//		if the file size is greater than 10MB
//			read the file in 10MB chunks
//
	if(file_size < 10485760){ //																						if the file size is less than 10MB
		#ifdef _DEBUG //																									if debug is enabled
			printf("File is small, reading into buffer...\n"); //																print message to indicate the whole file is being read into a buffer
		#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Read the whole file into a buffer
	//

		char* buffer = malloc(file_size); //																				allocate memory on the heap for the buffer
		// Sanity check to ensure memory is loaded
		if(buffer == NULL){ //																								if memory is not allocated
			#ifdef _DEBUG //																									if debug is enabled
				perror("Failed to allocate memory for buffer."); //																	print error
			#endif

			return 1; //																										return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes, (%f MB) for buffer.\n", file_size, (float)file_size / 1048576);
		#endif

		fread(buffer, file_size, 1, data->source_file);  //																	read the whole file into the buffer

		#ifdef _DEBUG
			printf("Buffer filled.\n");
		#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Write the buffer to the new file
	//
		if(write_file(data, buffer, file_size, 0) == 1){ //																	if the file is not written successfully
			#ifdef _DEBUG //																									if debug is enabled
				printf("Failed to write file.\n"); //																				print error
			#endif

			free(buffer); //																									free the memory allocated for the buffer
			return 1; //																										return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("File written.\n");
		#endif
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Memory Cleanup
	//
		free(buffer); //																										free the memory allocated for the buffer

	} else { //																												if the file size is greater than 10MB
		#ifdef _DEBUG //																										if debug is enabled
			printf("File is large, reading in chunks...\n"); //																		print message to indicate the file is being read in chunks
		#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Allocate memory for the file chunks (10MB) - ensures the buffer is not too large for the sys
	//
		char* buffer = malloc(10485760); //																						allocate memory on the heap for the buffer
		// Sanity check to ensure memory is loaded
		if(buffer == NULL){ //																									if memory is not allocated
			#ifdef _DEBUG //																										if debug is enabled
				perror("Failed to allocate memory for buffer."); //																		print error
			#endif

			return 1; //																											return 1 to indicate failure
		}

		#ifdef _DEBUG
			printf("Allocated %d bytes for buffer.\n", 10485760);
		#endif

		int offset = 0;
		int bytes_read = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//		BASE LOGIC OF THE FOLLOWING WHILE LOOP
	//
	//			while the offset is less than the file size
	//
	//				if the offset + 10MB is greater than the file size
	//					read the remaining bytes into the buffer
	//
	//				if the offset + 10MB is less than the file size
	//					read 10MB into the buffer
	//
	// 				Write the buffer to the new file
	//				Add the bytes read to the offset

		while(offset < file_size){ //																								while the offset is less than the file size
			if(offset + 10485760 > file_size){ //																						if the offset + 10MB is greater than the file size
				bytes_read = fread(buffer, file_size - offset, 1, data->source_file); //														read the remaining bytes into the buffer
			} else { //																													Othewise
				bytes_read = fread(buffer, 10485760, 1, data->source_file); //																	read 10MB into the buffer
			}

			if(bytes_read == 0){ //																										if the bytes read is 0
				#ifdef _DEBUG //																											if debug is enabled
					perror("Failed to read file."); //																							print error
				#endif

				free(buffer); //																											free the memory allocated for the buffer
				return 1; //																												return 1 to indicate failure
			}

			if(write_file(data, buffer, bytes_read, offset) == 1){ //																	if the file is not written successfully
				#ifdef _DEBUG //																											if debug is enabled
					printf("Failed to write file.\n"); //																						print error
				#endif

				free(buffer); //																											free the memory allocated for the buffer
				return 1; //																												return 1 to indicate failure
			}
			offset += bytes_read; //																										add the bytes read to the offset
		}

		#ifdef _DEBUG
			printf("File written.\n");
		#endif

		free(buffer); //																												free the memory allocated for the buffer
	}

	return 0; //																													return 0 to indicate successfull execution
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BASE LOGIC OF THE FOLLOWING FUNCTION (write_file)
//
//		Takes Arguments:
//			- struct CopyData* data
//			- char* buffer
//			- int buffer_size
//			- int offset
//
//		Does:
//			- Opens the new file in either write binary or append binary mode
//			- Writes the buffer to the file
//
int write_file(struct CopyData* data, char* buffer, int buffer_size, int offset){
	#ifdef _DEBUG
		printf("Writing file...");
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 	Open the new file in either write binary or append binary mode
//
	if(data->new_file == NULL){ //																						if the file is not open
		#ifdef _DEBUG //																									if debug is enabled
			printf("File is not open.\n"); //																					print message to indicate the file is not open
		#endif

		if(offset == 0){ //																								if the offset is 0
			data->new_file = fopen(data->new_file_path, "wb"); //															open the file in write binary mode
		} else { //																										otherwise
			data->new_file = fopen(data->new_file_path, "ab"); //															open the file in append binary mode
		}

		if(data->new_file == NULL){ //																					if the file is not opened successfully
			#ifdef _DEBUG //																								if debug is enabled
				perror("Failed to open new file."); //																			print error
			#endif
			return 1; //																									return 1 to indicate failure
		}
	}

	#ifdef _DEBUG
		printf("File is open.\n");
	#endif

	if(offset == 0){ //																									if the offset is 0
		fwrite(buffer, buffer_size, 1, data->new_file); //																	write the buffer to the file
	} else { //																											otherwise
		fseek(data->new_file, offset, SEEK_SET); //																			go to the offset in the file
		fwrite(buffer, buffer_size, 1, data->new_file); //																	write the buffer to the file
	}

	#ifdef _DEBUG
		printf("File written.\n");
	#endif

	return 0; //																										return 0 to indicate successfull execution

}