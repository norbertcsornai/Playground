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

void printHelpBoard();
void printBoard(vector<vector<char> > board);
bool IsGameOver(vector<vector<char> > board);

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
	int turn = 0;
	bool wrongnumber;
	int box = 0;
	int Player1Guess = 0;
	int Player2Guess = 0;

	
	vector<vector<char> > board{ { '-', '-', '-' },
							   { '-', '-', '-' },
							   { '-', '-', '-' } };

	vector<int> guesses{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	

	do {
		// clear the screen
		system("CLS");

		cout << "Welcome to Tic Tac Toe. Player 1 uses X and Player 2 uses O!" << endl;

		printHelpBoard();

		printBoard(board);

		// turn 0 - Player 1, turn 1 - Player 2
		if (turn %2 == 0)
		{
			do {
				wrongnumber = false;
				// Display options 
				cout << "Player 1 enter ";
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
				// if not, display again options and start over
				if (!count(guesses.begin(), guesses.end(), Player1Guess))
				{
					cout << "Please enter again ";
					for (int i = 0; i < guesses.size(); i++)
					{
						if (i == guesses.size() - 1)
							cout << guesses[i] << " ";
						else
							cout << guesses[i] << ", ";
					}
					cout << " : " << endl;

					wrongnumber = true;
				}
			} while (wrongnumber);
			// remove Player 1 guess from vector so Player 2 can't choose the same number
			guesses.erase(remove(guesses.begin(), guesses.end(), Player1Guess), guesses.end());

			// sort the array
			sort(guesses.begin(), guesses.end());
		}
		else
		{
			do
			{
				wrongnumber = false;
				cout << "Player 2 enter ";
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

			// remove player 2 guess
			guesses.erase(remove(guesses.begin(), guesses.end(), Player2Guess), guesses.end());
			// sort array
			sort(guesses.begin(), guesses.end());
			
		}

		// Check to see if anyone won or lost
		if (turn %2 == 0)
		{
			switch (Player1Guess)
			{
			case 1:
			{
				board[0][0] = 'X';
			}
			break;

			case 2:
			{
				board[0][1] = 'X';
			}
			break;

			case 3:
			{
				board[0][2] = 'X';
			}
			break;

			case 4:
			{
				board[1][0] = 'X';
			}
			break;

			case 5:
			{
				board[1][1] = 'X';
			}
			break;

			case 6:
			{
				board[1][2] = 'X';
			}
			break;

			case 7:
			{
				board[2][0] = 'X';
			}
			break;

			case 8:
			{
				board[2][1] = 'X';
			}
			break;

			case 9:
			{
				board[2][2] = 'X';
			}
			break;
			}
		}
		else
		{
			switch (Player2Guess)
			{
			case 1:
			{
				board[0][0] = 'O';
			}
			break;

			case 2:
			{
				board[0][1] = 'O';
			}
			break;

			case 3:
			{
				board[0][2] = 'O';
			}
			break;

			case 4:
			{
				board[1][0] = 'O';
			}
			break;

			case 5:
			{
				board[1][1] = 'O';
			}
			break;

			case 6:
			{
				board[1][2] = 'O';
			}
			break;

			case 7:
			{
				board[2][0] = 'O';
			}
			break;

			case 8:
			{
				board[2][1] = 'O';
			}
			break;

			case 9:
			{
				board[2][2] = 'O';
			}
			break;
			}
		}

		// next turn
		turn++;

	} while (!IsGameOver(board) && turn<9);

	if (turn == 9)
		cout << "Cat Game!" << endl;

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

void printHelpBoard()
{
	cout << "Help Board : " << endl;
	cout << "+---+---+---+" << endl;
	cout << "| 1 | 2 | 3 |" << endl;
	cout << "+---+---+---+" << endl;
	cout << "| 4 | 5 | 6 |" << endl;
	cout << "+---+---+---+" << endl;
	cout << "| 7 | 8 | 9 |" << endl;
	cout << "+---+---+---+" << endl << endl;
}

void printBoard(vector<vector<char> > board)
{
	cout << "Game Board : " << endl;
	// Displaying the 2D vector 
	for (int i = 0; i < board.size(); i++)
	{
		cout << "+---+---+---+" << endl;
		for (int j = 0; j < board[i].size(); j++)
		{
			cout << "| " << board[i][j] << " ";
		}
		cout << "|" << endl;
	}
	cout << "+---+---+---+" << endl << endl;
}

bool IsGameOver(vector<vector<char>> board)
{
	bool bGameOver = false;

	// check for X (Player 1)
	for (int i = 0; i != 1; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}

	for (int i = 1; i != 2; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}


	for (int i = 2; i != 3; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 1 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}

	int win_cnt = 0;

	for (int i = 0; i < 3; i++)
	{
		
		if (board[i][i] == 'X')
		{
			win_cnt++;
			if (win_cnt == 3)
			{
				cout << "\n Player 1 has won the game!" << endl;
				bGameOver = true;
				return bGameOver;
			}
		}// check for diagonal 1
	}

	win_cnt = 0;

	for (int i = 0; i < 3; i++)
	{
		if (board[i][2 - i] == 'X')
		{
			win_cnt++;
			if (win_cnt == 3)
			{
				cout << "\n Player 1 has won the game!" << endl;
				bGameOver = true;
				return bGameOver;
			}
		}// check for diagonal 2		
	}
	


	// check for O (Player 2)
	for (int i = 0; i != 1; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'O')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'X')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}

	for (int i = 1; i != 2; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'O')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'O')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}


	for (int i = 2; i != 3; i++)
	{
		int win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[i][j] == 'O')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for row win

		win_cnt = 0;
		for (int j = 0; j != 3; j++)
		{
			if (board[j][i] == 'O')
			{
				win_cnt++;
				if (win_cnt == 3)
				{
					cout << "\n Player 2 has won the game!" << endl;
					bGameOver = true;
					return bGameOver;
				}
			}
		}//Check for column win
	}

	win_cnt = 0;

	for (int i = 0; i < 3; i++)
	{

		if (board[i][i] == 'O')
		{
			win_cnt++;
			if (win_cnt == 3)
			{
				cout << "\n Player 2 has won the game!" << endl;
				bGameOver = true;
				return bGameOver;
			}
		}// check for diagonal 1
	}

	win_cnt = 0;

	for (int i = 0; i < 3; i++)
	{
		if (board[i][2 - i] == 'O')
		{
			win_cnt++;
			if (win_cnt == 3)
			{
				cout << "\n Player 2 has won the game!" << endl;
				bGameOver = true;
				return bGameOver;
			}
		}// check for diagonal 2		
	}

	return bGameOver;
}
