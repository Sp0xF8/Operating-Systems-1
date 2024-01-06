#include <stdio.h>
#include <stdbool.h>

void display_pattern(int patternSize);
int rows;

int main() {


	while (true) {
		rows = 0;

		printf("\n\n\n\n\n\n\nHello! Welcome to the pattern generator!\n\n");
		printf("Please enter the number of rows you would like to see: ");
		scanf("%d", &rows);

		printf("\n\n\n");


		//Sanity Check #1 - Ensures a pattern can be made.
		if (rows < 3) {
			printf("\n\n\n\n\nPlease enter a number greater than 2, you entered %d", rows);
			continue;
		}



		display_pattern(rows);

	}


	return 0;
}


bool is_even(int num) {
	if (num % 2 == 0)
		return true;
	else
		return false;
}


void draw_line(int stars, int width);

void display_pattern(int patternSize) {


	//function setups

	bool even = false; //defines if the sides are curved or pointed
	int midpoint = patternSize; //gets uses the midpoint as a tempstore and condensing the decliration to optimise runtime.

	if (patternSize % 2 == 0) {
		even = true;
	}
	else {
		even = false;
		midpoint -= 1; // by default, all patterns are build to be smoothed edge diamonds. if they are odd, they are given an extra layer.
	}
	midpoint /= 2;

	int width = midpoint + (midpoint-1);
	int height = (const int)even ? midpoint * 2 : (midpoint * 2) + 1;


	//Draw the rise of the diamond
	for (int p = 1; p <= midpoint; p++) {
		if(!is_even(rows)){
			printf(" ");
		}
		draw_line(p, width);
		
	}

	if (!even) {
		draw_line(midpoint + 1, width);
	}

	//Draw the fall of the diamond
	for (int p = midpoint; p >= 1; p--) {
		if(!is_even(rows)){
			printf(" ");
		}
		draw_line(p, width);
	}


}

void draw_line(int stars, int width) {
	int pattern_length = stars + (stars - 1);
	int start_point = (width - pattern_length) / 2;

	for (int i = 1; i <= start_point; i++){
		printf(" ");
	}


	//draw pattern
	for (int i = 1; i <= pattern_length; i++){
		if(is_even(i)){
			printf(" ");

		} else {
			printf("*");
		}
	}

	for (int i = 1; i <= start_point; i++){
		printf(" ");
	}

	printf("\n");

}