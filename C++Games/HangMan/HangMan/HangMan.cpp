#include <iostream>
#include <cmath>
#include <cctype>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <string.h>

#include "Utils.h"

using namespace std;

void PlayGame();
bool WantToPlayAgain();
int GetSecretPhrase(char secretPhrase[], int maxLenght);

char * MakeHiddenPhrase(const char * secretPhrase, int secretPhraseLenght);
void DrawBoard(int numberOfGuessesLeft, const char * noptrHiddenPhrase);

char GetGuess();
void UpdateBoard(char guess, char * noptrHiddenPhrase, const char secretPhrase[], int secretPhraseLenght, int & numberOfGuessesLeft);

bool IsGameOver(int numberOfGuessesLeft, const char * noptrHiddenPhrase, int secretPhraseLenght);
void DisplayResult(const char * secretPhrase, int numberOfGuessesLeft);

const char* INPUT_ERROR_STRING = "Input error! Please try again.";

const int IGNORE_CHARS = 256;

int main()
{
	do {
		PlayGame();
	} while (WantToPlayAgain());

	return 0;
}

void PlayGame()
{
	const int MAX_LENGHT_OF_SECRET_PHRASE = 256;
	const int MAX_NUMBER_OF_GUESSES = 6;

	char secretPhrase[MAX_LENGHT_OF_SECRET_PHRASE];
	char* optrHiddenPhrase = nullptr;
	int numberOfGuessesLeft = MAX_NUMBER_OF_GUESSES;

	int secretPhraseLenght = GetSecretPhrase(secretPhrase, MAX_LENGHT_OF_SECRET_PHRASE);

	optrHiddenPhrase = MakeHiddenPhrase(secretPhrase, secretPhraseLenght);

	DrawBoard(numberOfGuessesLeft, optrHiddenPhrase);

	char guess;

	do 
	{
		guess = GetGuess();
		UpdateBoard(guess, optrHiddenPhrase, secretPhrase, secretPhraseLenght, numberOfGuessesLeft);
		DrawBoard(numberOfGuessesLeft, optrHiddenPhrase);
	} while (!IsGameOver(numberOfGuessesLeft, optrHiddenPhrase, secretPhraseLenght));

	DisplayResult(secretPhrase, numberOfGuessesLeft);

	delete [] optrHiddenPhrase;

}

bool WantToPlayAgain()
{
	const char validInputs[] = { 'y', 'n' };
	char response = GetCharacter("Would you like to play again? (y/n) ", INPUT_ERROR_STRING, validInputs, 2);

	if(response == 'y')
		// clear the screen
		system("CLS");

	return response == 'y';
}

int GetSecretPhrase(char secretPhrase[], int maxLenght)
{
	vector<string> Pool = { "London", "New York", "Yokohama", "Miami", "Los Angeles", "Berlin", "Rome", "Venice", "Hamburg", "Paris" };
	
	srand((unsigned)time(0));
	int result = 1 + (rand() % 10);

	
	strcpy_s(secretPhrase, 256, Pool[result].c_str());

	return Pool[result].length();
}

char* MakeHiddenPhrase(const char* secretPhrase, int secretPhraseLenght)
{
	char* hiddenPhrase = new char[secretPhraseLenght + 1];

	for (int i = 0; i < secretPhraseLenght; i++)
	{
		if (secretPhrase[i] != ' ')
		{
			hiddenPhrase[i] = '-';
		}
		else 
		{
			hiddenPhrase[i] = ' ';
		}
	}

	hiddenPhrase[secretPhraseLenght] = '\0';

	return hiddenPhrase;
}

void DrawBoard(int numberOfGuessesLeft, const char* noptrHiddenPhrase)
{
	cout << "Guess the city!" << endl << endl;

	switch (numberOfGuessesLeft)
	{
	case 0:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |  /|\\" << endl;
		cout << " |  / \\" << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 1:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |  /|\\" << endl;
		cout << " |  /" << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 2:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |  /|\\" << endl;
		cout << " | " << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 3:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |  /|" << endl;
		cout << " |  " << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 4:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |   |" << endl;
		cout << " |  " << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 5:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |   O" << endl;
		cout << " |  " << endl;
		cout << " |  " << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;

	case 6:
	{
		cout << " +---+" << endl;
		cout << " |   |" << endl;
		cout << " |  " << endl;
		cout << " |  " << endl;
		cout << " |  " << endl;
		cout << "-+-" << endl << endl;
		cout << "Number of guesses left: " << numberOfGuessesLeft << endl << endl;
	}
	break;
	default:
		break;
	}

	cout << "Secret Phrase: " << noptrHiddenPhrase << endl << endl;
}

char GetGuess()
{
	return GetCharacter("Please input your guess: ", INPUT_ERROR_STRING);
}



void UpdateBoard(char guess, char* noptrHiddenPhrase, const char secretPhrase[], int secretPhraseLenght, int& numberOfGuessesLeft)
{
	bool found = false;

	for (int i = 0; i < secretPhraseLenght; i++)
	{
		if (tolower(secretPhrase[i]) == tolower(guess))
		{
			noptrHiddenPhrase[i] = secretPhrase[i];
			found = true;
		}
	}
	if (!found)
	{
		numberOfGuessesLeft--;
	}

	// clear the screen
	system("CLS");
}

bool IsGameOver(int numberOfGuessesLeft, const char* noptrHiddenPhrase, int secretPhraseLenght)
{
	bool hasDash = false;

	for (int i = 0; i < secretPhraseLenght; i++)
	{
		if (noptrHiddenPhrase[i] == '-')
		{
			hasDash = true;
			break;
		}
	}

	return numberOfGuessesLeft == 0 || !hasDash;
}

void DisplayResult(const char* secretPhrase, int numberOfGuessesLeft)
{
	if (numberOfGuessesLeft > 0)
	{
		cout << "You got it! The word was: " << secretPhrase << endl;
	}
	else
	{
		cout << "You didn't get it ... the phrase was:" << secretPhrase << endl;
	}
}
