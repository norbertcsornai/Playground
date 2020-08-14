#include "Utils.h"
#include <iostream>
#include <cctype>

using namespace std;

char GetCharacter(const char* prompt, const char* error)
{
	const int IGNORE_CHARS = 256;
	char input;
	bool inputFailure;

	do {

		inputFailure = false;
		std::cout << prompt;

		std::cin >> input;

		if (std::cin.fail())
		{
			std::cin.ignore(IGNORE_CHARS, '\n');
			std::cin.clear();
			std::cout << error << std::endl;
			inputFailure = true;
		}
		else
		{
			std::cin.ignore(IGNORE_CHARS, '\n');
			if (std::isalpha(input))
			{
				input = std::tolower(input);
			}
			else
			{
				std::cout << error << std::endl;
				inputFailure = true;
			}
		}

	} while (inputFailure);

	return input;
}

char GetCharacter(const char* prompt, const char* error, const char validInput[], int validInputLength, CharacterCaseType charCase)
{
	const int IGNORE_CHARS = 256;
	char input;
	bool inputFailure = false;

	do {

		std::cout << prompt;
		std::cin >> input;

		if (std::cin.fail())
		{
			std::cin.ignore(IGNORE_CHARS, '\n');
			std::cin.clear();
			std::cout << error << std::endl;
			inputFailure = true;
		}
		else
		{
			std::cin.ignore(IGNORE_CHARS, '\n');

			if (isalpha(input))
			{
				if (charCase == CC_UPPER_CASE)
				{
					//input = std::tolower(input);
					input = std::toupper(input);
				}
				else if (charCase == CC_LOWER_CASE)
				{
					//input = std::toupper(input);
					input = std::tolower(input);
				}

				for (int i = 0; i < validInputLength; ++i)
				{
					if (input == validInput[i])
					{
						return input;
					}
				}
			}

			std::cout << error << std::endl;
			inputFailure = true;
		}

	} while (inputFailure);

	return ' ';
}

int GetInteger(const char* prompt, const char* error, const int validInput[], int validInputlenght)
{
	int input;
	bool inputfailure;

	do
	{
		inputfailure = false;

		cout << prompt;
		cin >> input;
		const int IGNORE_CHARS = 256;
		if (cin.fail())
		{
			cin.clear();
			cin.ignore(IGNORE_CHARS, '\n');
			cout << error << endl;
			inputfailure = true;
		}
		else
		{
			for (int i = 0; validInputlenght; i++)
			{
				if (input == validInput[i])
				{
					return input;
				}
			}

			cout << error << endl;
			inputfailure = true;
		}

	} while (inputfailure);
}

void WaitForKeyPress()
{
	system("pause");
}