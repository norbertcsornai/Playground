#include <iostream>
#include <cmath>
#include <cctype>
#include <stdlib.h>
#include <ctime>
using namespace std;

void PlayGame();
bool WantToPlayAgain();
bool IsGameOver(int secretNumber, int numberOfTries, int guess);
int GetGuess(int numberOfTries);
void DisplayResult(int secretNumber, int numberOfTries);

const int IGNORE_CHARS = 256;

int main()
{
	// main function from which we play the game until 
	do
	{
		PlayGame();
	} while (WantToPlayAgain());

	return 0;
}

void PlayGame()
{
	// uper bound of the range of possible numbers to guess
	const int UPPER_BOUND = 100;

	// variable for secret number to guess -> from 0 to 100
	int secretNumber = rand() % UPPER_BOUND;

	// variable that stores the number of guesses the player has left
	int numberOfTries = ceil(log2(UPPER_BOUND));

	// variable that will store the players guess
	int guess = 0;

	cout << "The range of the number is between 0 and "<< UPPER_BOUND << endl;

	do
	{
		// get the guess from the player
		guess = GetGuess(numberOfTries);

		// if not guessed the we substract from the number of tries
		if (guess != secretNumber)
		{
			numberOfTries--;
			
			if (guess > secretNumber)
			{
				cout << "Guess was too high!" << endl;
			}
			else
			{
				cout << "Guess was too low!" << endl;
			}
		}
			

	} while (!IsGameOver(secretNumber, numberOfTries, guess));

	DisplayResult(secretNumber, numberOfTries);
}

// ask the player if play again or not. Choices are Y for yes and N for no
bool WantToPlayAgain()
{
	char input;
	bool failure;
	do
	{
		failure = false;

		cout << "Would you like to play again? (y/n) : ";
		cin >> input;

		if (cin.fail())
		{
			cin.clear();
			cin.ignore(IGNORE_CHARS, '\n');
			cout << "Input Error! Please try again." << endl;
			failure = true;
		}
		else
		{
			cin.ignore(IGNORE_CHARS, '\n');
			input = tolower(input);
		}

		// only accept y or n
		if (input != 'y' && input != 'n')
		{
			cout << "Please enter Y (Yes) or N (No)" << endl;
			failure = true;
		}

	} while (failure);

	// clear the screen
	system("CLS");
	return input == 'y';
}

// game over when number is guessed or out of tries
bool IsGameOver(int secretNumber, int numberOfTries, int guess)
{
	return secretNumber == guess || numberOfTries <= 0;
}

int GetGuess(int numberOfTries)
{
	
	int guess;
	bool failure;
	
	do
	{
		failure = false;
		cout << "Please enter your guess (number of guesses left: " << numberOfTries << "): ";
		cin >> guess;

		// check that only numbers are inputed
		if(cin.fail())
		{
			cin.clear();
			cin.ignore(IGNORE_CHARS, '\n');
			cout << "Input error! Please try again." << endl;
			failure = true;
		}

	} while (failure);
	return guess;
}

// prompt if number is  guessed or if out of guesses and game is lost
void DisplayResult(int secretNumber, int numberOfTries)
{
	if (numberOfTries > 0)
	{
		cout << "You guessed it! Number is :" << secretNumber << endl;
	}
	else
	{
		cout << "You didn't guessed it! Number is:" << secretNumber << endl;
	}

}
