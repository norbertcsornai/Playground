#include <iostream>
#include <cctype>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <algorithm>

using namespace std;

void PlayGame();
bool WantToPlayAgain();

void printHelpBoard();
void printBoard(const vector<vector<char>>& board);
bool IsGameOver(const vector<vector<char>>& board);

int GetAIMove(const vector<vector<char>>& board, const vector<int>& guesses);
void ApplyMove(vector<vector<char>>& board, int move, char symbol);
bool IsWinningMove(vector<vector<char>> board, int move, char symbol);
bool HasWinner(const vector<vector<char>>& board, char symbol);

const int IGNORE_CHARS = 256;

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	do {
		PlayGame();
	} while (WantToPlayAgain());

	return 0;
}

void PlayGame()
{
	int turn = 0;
	bool wrongnumber;
	int Player1Guess = 0;
	int aiGuess = 0;

	vector<vector<char>> board{ { '-', '-', '-' },
								{ '-', '-', '-' },
								{ '-', '-', '-' } };

	vector<int> guesses{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	do {
		system("CLS");

		cout << "Welcome to Tic Tac Toe. Player 1 uses X and AI uses O!" << endl;
		printHelpBoard();
		printBoard(board);

		// turn 0 - Player 1, turn 1 - AI
		if (turn % 2 == 0)
		{
			do {
				wrongnumber = false;

				cout << "Player 1 enter ";
				for (int i = 0; i < static_cast<int>(guesses.size()); i++)
				{
					if (i == static_cast<int>(guesses.size()) - 1)
						cout << guesses[i] << " ";
					else
						cout << guesses[i] << ", ";
				}
				cout << " : " << endl;

				cin >> Player1Guess;

				if (cin.fail())
				{
					cin.clear();
					cin.ignore(IGNORE_CHARS, '\n');
					cout << "Input Error! Please try again." << endl;
					wrongnumber = true;
				}
				else if (!count(guesses.begin(), guesses.end(), Player1Guess))
				{
					cout << "Please enter again ";
					for (int i = 0; i < static_cast<int>(guesses.size()); i++)
					{
						if (i == static_cast<int>(guesses.size()) - 1)
							cout << guesses[i] << " ";
						else
							cout << guesses[i] << ", ";
					}
					cout << " : " << endl;
					wrongnumber = true;
				}
			} while (wrongnumber);

			ApplyMove(board, Player1Guess, 'X');
			guesses.erase(remove(guesses.begin(), guesses.end(), Player1Guess), guesses.end());
			sort(guesses.begin(), guesses.end());
		}
		else
		{
			aiGuess = GetAIMove(board, guesses);
			cout << "AI chooses " << aiGuess << "." << endl;

			ApplyMove(board, aiGuess, 'O');
			guesses.erase(remove(guesses.begin(), guesses.end(), aiGuess), guesses.end());
			sort(guesses.begin(), guesses.end());
		}

		turn++;

	} while (!IsGameOver(board) && turn < 9);

	system("CLS");
	cout << "Welcome to Tic Tac Toe. Player 1 uses X and AI uses O!" << endl;
	printHelpBoard();
	printBoard(board);

	if (HasWinner(board, 'X'))
		cout << "\nPlayer 1 has won the game!" << endl;
	else if (HasWinner(board, 'O'))
		cout << "\nAI has won the game!" << endl;
	else if (turn == 9)
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
			input = static_cast<char>(tolower(input));
		}

		if (input != 'y' && input != 'n')
		{
			cout << "Please enter Y (Yes) or N (No)" << endl;
			failure = true;
		}

	} while (failure);

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

void printBoard(const vector<vector<char>>& board)
{
	cout << "Game Board : " << endl;
	for (int i = 0; i < static_cast<int>(board.size()); i++)
	{
		cout << "+---+---+---+" << endl;
		for (int j = 0; j < static_cast<int>(board[i].size()); j++)
		{
			cout << "| " << board[i][j] << " ";
		}
		cout << "|" << endl;
	}
	cout << "+---+---+---+" << endl << endl;
}

void ApplyMove(vector<vector<char>>& board, int move, char symbol)
{
	int row = (move - 1) / 3;
	int col = (move - 1) % 3;
	board[row][col] = symbol;
}

bool IsWinningMove(vector<vector<char>> board, int move, char symbol)
{
	ApplyMove(board, move, symbol);
	return HasWinner(board, symbol);
}

int GetAIMove(const vector<vector<char>>& board, const vector<int>& guesses)
{
	for (int guess : guesses)
	{
		if (IsWinningMove(board, guess, 'O'))
			return guess;
	}

	for (int guess : guesses)
	{
		if (IsWinningMove(board, guess, 'X'))
			return guess;
	}

	if (count(guesses.begin(), guesses.end(), 5))
		return 5;

	vector<int> corners{ 1, 3, 7, 9 };
	vector<int> availableCorners;
	for (int corner : corners)
	{
		if (count(guesses.begin(), guesses.end(), corner))
			availableCorners.push_back(corner);
	}
	if (!availableCorners.empty())
		return availableCorners[rand() % availableCorners.size()];

	return guesses[rand() % guesses.size()];
}

bool HasWinner(const vector<vector<char>>& board, char symbol)
{
	for (int i = 0; i < 3; i++)
	{
		if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol)
			return true;
		if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol)
			return true;
	}

	if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol)
		return true;

	if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)
		return true;

	return false;
}

bool IsGameOver(const vector<vector<char>>& board)
{
	return HasWinner(board, 'X') || HasWinner(board, 'O');
}
