#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int randomNumber;
bool cheats = false;


void randomise_number(){
    randomNumber = rand() % 5000 + 1;
    if(cheats)
        printf("The number is %d\n", randomNumber);
}

int main(){
    srand(time(NULL));

    int highScore = 0;
    int rounds = 0;

    


    
    while(true)
        {
            int lives = 5;

            printf("Welcome to the number guessing game!\n");
            printf("1. Play Game\n");
            printf("2. Quit\n");
            printf("3. Play With Cheats\n");

            int choice;
            scanf("%d", &choice);

            switch (choice)
            {
            case 1:
                printf("Starting game...\n");
		        randomise_number();
                break;
                
            case 2:
                printf("Quitting game...\n");
                return 0;
                break;
            case 3:
                printf("Starting game with cheats...\n");
                cheats = true;
                randomise_number();
                break;
            }
            
            printf("\n\n\n\n\n");
            
            while(true){
                rounds++;

                printf("-===================-\n");
                printf("Round %d\n", rounds);

                printf("You have %d lives left\n", lives);
                printf("Guess a number between 1 and 5,000: ");

                int guess;
                scanf("%d", &guess);


                if(guess == randomNumber){
                    printf("You guessed correctly!\n");
                    highScore++;
                    randomise_number();
                }
                else{
                    printf("You guessed incorrectly!\n");
                    lives--;
                    
                }



                if(lives == 0){
                    printf("You ran out of lives!\n");
                    break;
                }
            }

            printf("\n\nROUND %d\n\n", rounds);
            printf("The number was %d\n", randomNumber);
            printf("Your high score is %d\n", highScore);
        }


    return 0;
}