#include <stdio.h>

int main(){

   //create a new file from the name of a vriable 
    FILE *fp = fopen("test.txt", "w");
    if(fp == NULL){
        printf("Error opening file\n");
        return 1;
    }

    //get a sentence from the user
    char sentence[100];
    printf("Enter a sentence: ");
    fgets(sentence, 100, stdin);

    //write the sentence to the file
    fprintf(fp, "%s", sentence);

    //close the file

    fclose(fp);

    //open the file for reading
    fp = fopen("test.txt", "r");

    //read the file into a new sentence variable
    char sentence2[100];
    fgets(sentence2, 100, fp);

    //print the sentence
    printf("The sentence is: %s\n", sentence2);

    //close the file
    fclose(fp);

}