#include <iostream>
#include <cmath>
#include <cctype>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <algorithm>

using namespace std;

void PlayGame();
bool WantToPlayAgain();

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
	int initialNumber = 8;
	int Player1Guess = 0;
	int Player2Guess = 0;
	bool turn = 0;
	bool wrongnumber = false;

	cout << "Initial number is 8." << endl;
	vector<int> guesses { 1, 2, 3 };
	
	do {
		// turn 0 - Player 1, turn 1 - Player 2
		if (turn == 0)
		{
			do {
				wrongnumber = false;
				// Display options 
				cout << "Player 1 enter  ";
				for (int i = 0; i < guesses.size(); i++)
				{
					if (i == guesses.size() - 1)
						cout << guesses[i] << " ";
					else
						cout << guesses[i] << ", ";
				}
				cout << " : " << endl;
				
				// check that player entered a n int found in guesses vector
				cin >> Player1Guess;
				if (cin.fail())
				{
					cin.clear();
					cin.ignore(IGNORE_CHARS, '\n');
					cout << "Input Error! Please try again." << endl;
					wrongnumber = true;
				}
				// if not display again options and start over
				if (!count(guesses.begin(), guesses.end(), Player1Guess))
				{
					cout << "Please enter again ";
					for (int i = 0; i < guesses.size(); i++)
					{
						cout << guesses[i] << ", ";
					}
					cout << " : " << endl;

					wrongnumber = true;
				}		
			} while (wrongnumber);
			// remove Player 1 guess from vector so Player 2 can't choose the same number
			guesses.erase(remove(guesses.begin(), guesses.end(), Player1Guess), guesses.end());

			// put back into vector the option choosen by Player 2 on his last turn
			if (Player2Guess != 0)
			{
				guesses.push_back(Player2Guess);
				sort(guesses.begin(), guesses.end());
			}
		}
		else
		{			
			do
			{
				wrongnumber = false;
				cout << "Player 2 enter  ";
				for (int i = 0; i < guesses.size(); i++)
				{
					if (i == guesses.size() - 1)
						cout << guesses[i] << " ";
					else
						cout << guesses[i] << ", ";
				}
				cout << " : " << endl;

				cin >> Player2Guess;
				if (cin.fail())
				{
					cin.clear();
					cin.ignore(IGNORE_CHARS, '\n');
					cout << "Input Error! Please try again." << endl;
					wrongnumber = true;
				}
				if (!count(guesses.begin(), guesses.end(), Player2Guess))
				{
					cout << "Please enter again ";
					for (int i = 0; i < guesses.size(); i++)
					{
						cout << guesses[i] << ", ";
					}
					cout << " : " << endl;

					wrongnumber = true;
				}
				
			} while (wrongnumber);
			guesses.erase(remove(guesses.begin(), guesses.end(), Player2Guess), guesses.end());
			if (Player1Guess != 0)
			{
				guesses.push_back(Player1Guess);
				sort(guesses.begin(), guesses.end());
			}
		}

		// Check to see if anyone won or lost
		if (turn == 0)
		{
			initialNumber -= Player1Guess;
			cout << "Number is : " << initialNumber << endl;
			if (initialNumber == 0)
				cout << "Player 1 Winner!" << endl;
			else if (initialNumber < 0)
				cout << "Player 2 Winner!" << endl;
		}
		else
		{
			initialNumber -= Player2Guess;
			cout << "Number is : " << initialNumber << endl;
			if (initialNumber == 0)
				//winner = 2;
				cout << "Player 2 Winner!" << endl;
			else if (initialNumber < 0)
				//winner = 1;
				cout << "Player 1 Winner!" << endl;
		}
		
		// next turn
		turn = !turn;

	} while (initialNumber > 0);

}

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
