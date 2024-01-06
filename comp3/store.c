int get_file_data(struct CopyData* data){

	char* slash = strrchr(data->path, '/'); // 																finds the last slash in the path, logically should be just before the file name
	char* file_name_pos; // 																					used to store the position of the final name
	char* extension_pos = strrchr(data->path, '.'); //														finds the position of the final period, should be the position before the extension

	if(slash){ // 																								if a slash is present
		file_name_pos = slash + 1; //   																			there is a path- so the name starts one position after the slash
	} else {//																									otherwise, no slash is present
		file_name_pos = data->path;//																				the origonal input is the filename
	}

	if(extension_pos){ // 																						if there is an extension 
		size_t name_length = extension_pos - file_name_pos;//														calculates the name length
		size_t extension_length =  strlen(extension_pos + 1);//														calculates the extension length, excluding the period


		strncpy(data->file_name, file_name_pos, name_length);//														copies from file name start to the last period 
        data->file_name[name_length] = '\0';


        strncpy(data->file_extension, extension_pos + 1, extension_length);//									    copies from the extension period to end of the extension type
        data->file_extension[extension_length] = '\0';

		size_t total_string_length = (name_length + 1) + (extension_length + 1) + 7;//								calculates the length of the two strings, plus the null terminiator & period- also adding 7 spaces for " (copy)"

		strncpy(data->name_w_extension, file_name_pos, name_length + extension_length + 2);// 						copies the origonal file name + period + extension + nullterminator

		snprintf(data->new_file_name, total_string_length, "%s (copy).%s", data->file_name, data->file_extension);//builds the new file name (data->new_file_name), formats with "<filename> (copy).<extension>"
        

		

	} else {//																									otherwise, there is no extension

		strcpy(data->file_name, file_name_pos); //																	the position is the start of argv[1], the entire string is copied into the filename.
	}


	return 0;
}