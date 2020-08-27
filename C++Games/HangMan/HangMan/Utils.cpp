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

char GetCharacter(const char* prompt, const char* error, const char validInput[], int validInputLength)
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

			input = std::tolower(input);

			for (int i = 0; i < validInputLength; ++i)
			{
				if (input == validInput[i])
				{
					return input;
				}
			}

			std::cout << error << std::endl;
			inputFailure = true;
		}

	} while (inputFailure);

	return ' ';
}