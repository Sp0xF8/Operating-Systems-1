#include <stdlib.h>
#include <stdio.h>
// #include <dirent.h>
#include <string.h>



// this data structure holds all necessasary data for a file to be copied
struct CopyData {
	char* source; // 				just a c+p of argv[1]
	DIR* source_directory; //				not always used
	FILE* file; 	// 				file to be cloned


	char* path;//					path of origonal file
	char* file_name;//				origonal file name
	char* file_extension;//			file extension

	char* name_w_extension;//		origonal name with extension
	char* new_file_name;//			clone name with extension
	char* new_file_w_path_w_extension;

	char* destination;//			just a c+p of argv[2]
	DIR* destination_directory;//	destination directory

	FILE* destination_file;//		clone file
};

int copy_file(struct CopyData* data);

int copy_dir(struct CopyData* data);

int write_to_file(char* byte_buffer, struct CopyData* data, int ammend, long long start_point);

int get_file_data(struct CopyData* data);


/*
	main function for the program

		takes arguments-

			Source:
				can be path
				can be filename

			Destination:
				can be path
				can be null

			Modifiers:
				-d	:	this toggles debug mode, showing more indepth fail causes


		NOTES:--

			In cases where destination is null, the file is copied to the origonal directory- simply adding " (copy)"
*/
int main(int argc, char *argv[]) {

	//sanity check on number of arguments
	if(argc < 2 || argc > 4) { // 																											if between 1 and 3 arguments are not passed
		printf("Usage: %s <source> <destination> (-d)\n", argv[0]); // 																			instructs user on how to use the copy command
		return 1;//																																failed execution
	}

	int source_is_directory = 0; //																											changed to 1 when source is directory.
	struct CopyData* copy_data = malloc(sizeof(struct CopyData)); // 																											this structure holds all necessasary data for the copy

	if(!copy_data){
		perror("memory for copy data could not be allocated\n");
		return 1;
	}

	copy_data->source = argv[1];
	copy_data->destination = argv[2];




	copy_data->file = fopen(argv[1], "rb");//																								assumes file by default- opens in binary mode

	// FILE* source_file = fopen(argv[1], "rb");


	//sanity check on Source
	if(!copy_data->file){//																													if the origonal file is null

		if(argv[3] == "d") { perror("Error finding file, attempting directory...\n"); }//															if argument -d is used, debug is enabled

		copy_data->source_directory= opendir(argv[1]);//																								instead try a directory

		if(!copy_data->source_directory){//																											if the directory isnt found, the Source must be wrong.
			perror("Couldnt find Source. Please confirm path and try again.\n");
			return 1;//																																failed execution
		}

		source_is_directory = 1;//																												switches to directory copying
	}

	copy_data->destination_directory = opendir(argv[2]);//																					attempts to open destination directory

	//sanity check on destination
	if(!copy_data->destination_directory){//																								if the destination directory is null
		perror("Error opening destination: check file path\n");
		return 1;//																																failed execution
	}


	//at this point, source is defined (and decided if its a directory or not) and destination has also been discovered.

	if(source_is_directory == 0) { //																										the source is not a directroy
		copy_data->path = argv[1];
		if(copy_file(copy_data) == 1){//																										if copying the file fails
			perror("File not copied.\n");
			return 1;//																																failed execution
		}

	} else{//																																the source is a directory
		if(copy_dir(copy_data) == 1){//																											if copying the directory fails
			perror("Folder not copied.\n");//
			return 1;//																																failed execution
		}
	}


	return 0;//																																execution success
}




// function for copying a single file
int copy_file(struct CopyData* data){

	//null every var that changes per file
	data->file_name = NULL;
	data->file_extension = NULL;
	data->name_w_extension = NULL;
	data->new_file_name = NULL;


	//sanity check on file data
	if(get_file_data(data) != 0){//														if the file data couldnt be generated
		perror("Error getting path data.\n");
		return 1;//																			failed execution
	}

	

	


	fseek(data->file, 0, SEEK_END); // 													moves "cursor" from the beginning of the file to the end
	long long file_size = ftell(data->file); // 										gets the position of the cursor after moving through the file in bytes
	fseek(data->file, 0, SEEK_SET); // 													resets the cursor so the file can be read from the beginning

	printf("about to allocate");

    printf(file_size);
	//sanity check to ensure a file too big for our ram isnt moved
	if(file_size < 2147483648){ // 														if file size is smaller than 2gb
		printf("file is under 2gb\n");

		char* byte_buffer = malloc(file_size);//											allocate the space in memory for the bytes in the whole file

		if (!byte_buffer){//																if the byte buffer is not null
			perror("Couldnt allocate memory\n");
			fclose(data->file);//																close the origonal file
			closedir(data->destination_directory);//											close the destination directory
			return 1;//																			execution failure
		}

		fread(byte_buffer, 1, file_size, data->file);//										read the bytes in the entire file

		write_to_file(byte_buffer, data, 0, 0);//											write the bytes to the new file

		free(byte_buffer);//																frees memory

	} else {//																			otherwise, the file is over 2gb
		printf("file is over 2gb\n");

		long long reading_iteration_start = 1;//											defines the initial reading position
		long long amount_to_read = file_size;//												defines the amount to be read
		long long reading_iteration_end;//													declares the reading iterations end



		while (amount_to_read > 0){//														while the file hasnt been fully read, ensures an escape clause

			char* byte_buffer;//																declares the byte buffer

			if (amount_to_read > 1073741824){//													if the amount to read is still over a gb, it should be on the first two passes (atleast)
				reading_iteration_end = reading_iteration_start + 1073741824;//						updates how far into the file should be read
				byte_buffer = malloc(1073741824);//													allocates 1gb in the memory for the byte buffer

			} else {//																			if the amount is no longer over one gb, after the first two passes
				reading_iteration_end = file_size;//												the reading iteration end should become the file end
				byte_buffer = malloc(amount_to_read);//												the memory allocated to the correct length of the reamining data

			}


			if (!byte_buffer){//																if the  byte buffer is null
				perror("Couldnt allocate memory");
				fclose(data->file);//																close the file
				closedir(data->destination_directory);//											close the directory
				return 1;
			}

			//long long bytes_read =
			fread(byte_buffer, reading_iteration_start, reading_iteration_end, data->file); //	reads bytes into the memory buffer from the reading start point to the reading end point

			write_to_file(byte_buffer, data, 1, reading_iteration_start); // 					writes the buffer to the file in ammend mode, passing the correct starting point

			amount_to_read -= reading_iteration_end - reading_iteration_start;//				updates the amount to read
			reading_iteration_start += reading_iteration_end - reading_iteration_start;//		updates the reading start
			free(byte_buffer);//																frees memory
		}

	}
	
	fclose(data->file);//																	close the file
	fclose(data->destination_file);//														close the destination file

	return 0;//																			successful execution
}

//function for copying an entire directory
int copy_dir(struct CopyData* data){


	struct dirent* current_file;
	char* slash = strrchr(data->source, '/');

	while((current_file = readdir(data->source_directory)) != NULL){


		if(slash == strlen(data->source)){ //if the last character in the path is a slash
			snprintf(data->path, strlen(data->source) + strlen(current_file) + 1, "%s%s", data->source, current_file);// simply add the two strings together
		} else{// otherwise, if the last character is not a slash
			snprintf(data->path, strlen(data->source) + strlen(current_file) + 2, "%s/%s", data->source, current_file);// simply add a slash between the two strings

		}

		data->file = fopen(data->path, "rb");

		if(!data->file){
			perror("couldnt open file in directory");
			return 1;
		}

		if(!copy_file(data)){
			perror("Couldnt copy file");
			fclose(data->file);
			return 1;
		}

		fclose(data->file);

		data->file = NULL;

	}

}

// write a string of bytes to a predefined new file.
int write_to_file(char* byte_buffer, struct CopyData* data, int ammend, long long start_point){

	printf("opening file %s\n", data->new_file_w_path_w_extension);

	if(ammend == 1){ // 																		if ammend mode is enabled (copying a file over 2gb)
		if(!data->destination_file){ // 														if the file has not already been opened
			data->destination_file = fopen(data->new_file_w_path_w_extension, "rb+"); // 							oepn the file in read/write binary mode

			if(!data->destination_file){ //															sanity check to ensure file is opened correctly
				perror("Error: couldnt open destination file in read/write binary\n");
				return 1;//																				execution failure
			}
		}

		fseek(data->destination_file, start_point, SEEK_SET); //								update the cursor position every iteration

	} else { // 																			otherwise, ammend mode is disabled (copying file under 2gb)
		
		data->destination_file = fopen(data->new_file_w_path_w_extension, "wb"); //								openes file in write binary mode

		if(data->destination_file == NULL){ //															sanity check to ensure a file is created
			perror("Error: couldnt create destination file in write binary\n");
			return 1;//																				execution failure
		}

		


	}

	fwrite(byte_buffer, sizeof(char), sizeof(byte_buffer), data->destination_file); //		writes buffer to file


	return 0; //																			successful execution
}


//extracts path, file name and extension type and builds the new file name
int get_file_data(struct CopyData* data){

	char* slash = strrchr(data->path, '/'); // 												gets the last slash in the path
	char* dot = strrchr(data->path, '.'); // 													gets the last dot in the path
	char* file_name_pos; // 																	used to store the position of the final name
	
	if(slash){ // 																			if a slash is present
		file_name_pos = slash + 1; // 															there is a path- so the name starts one position after the slash
	} else {// 																				otherwise, no slash is present
		file_name_pos = data->path;// 															the origonal input is the filename
	}

	if(dot){ // 																				if there is an extension 
		size_t name_length = dot - file_name_pos;// 												calculates the name length
		size_t extension_length =  strlen(dot + 1);// 												calculates the extension length, excluding the period

		data->file_name = malloc(name_length + 1);// 												allocates memory for the file name
		if(!data->file_name){
			perror("Couldnt allocate memory for file name");
			return 1;
		}
		strncpy(data->file_name, file_name_pos, name_length);// 									copies from file name start to the last period 
		data->file_name[name_length] = '\0';

		data->file_extension = malloc(extension_length + 1);// 									allocates memory for the extension
		if(!data->file_extension){
			perror("Couldnt allocate memory for extension");
			return 1;
		}
		strncpy(data->file_extension, dot + 1, extension_length);// 								copies from the extension period to end of the extension type
		data->file_extension[extension_length] = '\0';

		size_t ostring_length = (name_length + 1) + (extension_length + 1);// 			calculates the length of the two strings, plus the null terminiator & period
		data->name_w_extension = malloc(ostring_length);// 									allocates memory for the name with extension
		if(!data->name_w_extension){
			perror("Couldnt allocate memory for name with extension");
			return 1;
		}
		strncpy(data->name_w_extension, file_name_pos, name_length + extension_length + 2);// 	copies the origonal file name + period + extension + nullterminator
		data->name_w_extension[name_length + extension_length + 2] = '\0';// 					sets the null terminator


		size_t total_string_length = ostring_length + 8;// 									calculates the length of the two strings, plus the null terminiator & period- also adding 7 spaces for "_(copy)."
		data->new_file_name = malloc(total_string_length);// 										allocates memory for the new file name
		if(!data->new_file_name){
			perror("Couldnt allocate memory for new file name");
			return 1;
		}
		snprintf(data->new_file_name, total_string_length, "%s_(copy).%s", data->file_name, data->file_extension);//builds the new file name (data->new_file_name), formats with "<filename>_(copy).<extension>"
		data->new_file_name[total_string_length] = '\0';// 										sets the null terminator


		data->new_file_w_path_w_extension = malloc(strlen(data->destination) + strlen(data->new_file_name));// 	allocates memory for the new file name with path
		if(!data->new_file_w_path_w_extension){
			perror("Couldnt allocate memory for new file name with path");
			return 1;
		}

													

	} else {// 																				otherwise, there is no extension

		data->file_name = malloc(strlen(file_name_pos) + 1);// 									allocates memory for the file name (strlen + 1 for null terminator)

		strcpy(data->file_name, file_name_pos); // 												copies the file name into the allocated memory
		data->file_name[strlen(file_name_pos)] = '\0';// 										sets the null terminator

		data->new_file_name = malloc(strlen(data->file_name) + 7);// 								allocates memory for the new file name (strlen + 7 for " (copy)")

		snprintf(data->new_file_name, strlen(data->file_name) + 7, "%s_(copy)", data->file_name);// 	builds the new file name (data->new_file_name), formats with "<filename> (copy)"

		data->new_file_w_path_w_extension = malloc(strlen(data->destination) + strlen(data->new_file_name));// 	allocates memory for the new file name with path

	}


	char* destination_slash = strrchr(data->destination, '/');

	if(destination_slash == strlen(data->destination)){ //if the last character in the path is a slash
		snprintf(data->new_file_w_path_w_extension, strlen(data->destination) + strlen(data->new_file_name) + 1, "%s%s", data->destination, data->new_file_name);// simply add the two strings together (no slash) 
	} else{// otherwise, if the last character is not a slash
		snprintf(data->new_file_w_path_w_extension, strlen(data->destination) + strlen(data->new_file_name) + 2, "%s/%s", data->destination, data->new_file_name);// simply add a slash between the two strings

	}

	data->new_file_w_path_w_extension[strlen(data->new_file_w_path_w_extension)] = '\0';// 	sets the null terminator
	
	printf("new file name: %s\n", data->new_file_w_path_w_extension);


	return 0;
}