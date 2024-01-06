#include <iostream>


void display_pattern(int patternSize);

int main() {

	int rows;

	while(TRUE){
		rows = 0;

		std::cout << "\n\n\n\n\n\n\nHello! Welcome to the pattern generator!\n\n" << std::endl;
		std::cout << "Please enter the number of rows you would like to see: ";
		std::cin >> rows;

		std::cout << "\n\n" << std::endl;


		//Sanity Check #1 - Ensures a pattern can be made.
		if (rows < 3) {
			std::cout << "\n\n\n\n\nPlease enter a number greater than 2." << std::endl;
			continue
		}



		display_pattern(rows);

	}



}




void display_pattern(int patternSize) {


	//function setups

	bool even = false; //defines if the sides are curved or pointed
	int midpoint = patternSize; //gets uses the midpoint as a tempstore and condensing the decliration to optimise runtime.

	if (patternSize % 2 == 0) {
		even = true
	} else {
		even = false
		midpoint -= 1 // by default, all patterns are build to be smoothed edge diamonds. if they are odd, they are given an extra layer.
	}
	midpoint /= 2

	int picture[midpoint][even ? midpoint * 2, midpoint * 2 + 1]
	
	//Draw the rise of the diamond
	for (int p = 1; p <= midpoint; p++) {
		for (int k = 0; k < p)

	}
	





}