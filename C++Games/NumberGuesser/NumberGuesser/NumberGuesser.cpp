#include <cctype>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace
{
struct GameSettings
{
	std::string name;
	int minValue;
	int maxValue;
	int tries;
};

void ClearInputLine()
{
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int CalculateTryLimit(int minValue, int maxValue)
{
	const int rangeSize = (maxValue - minValue) + 1; // inclusive range
	int tries = 0;
	int coveredValues = 1;

	while (coveredValues < rangeSize)
	{
		coveredValues *= 2;
		++tries;
	}

	return (tries > 0) ? tries : 1;
}

int ReadInt(const std::string& prompt)
{
	while (true)
	{
		std::string line;
		std::cout << prompt;
		if (!std::getline(std::cin, line))
		{
			std::cin.clear();
			std::cout << "Input error. Please enter a whole number.\n";
			continue;
		}

		size_t parsePos = 0;
		int value = 0;

		try
		{
			value = std::stoi(line, &parsePos);
		}
		catch (...)
		{
			std::cout << "Input error. Please enter a whole number.\n";
			continue;
		}

		while (parsePos < line.size() &&
			std::isspace(static_cast<unsigned char>(line[parsePos])))
		{
			++parsePos;
		}

		if (parsePos != line.size())
		{
			std::cout << "Input error. Please enter a whole number.\n";
			continue;
		}

		return value;
	}
}

GameSettings SelectDifficulty()
{
	while (true)
	{
		std::cout << "\nChoose a difficulty:\n";
		std::cout << "1) Easy   (0 - 50)\n";
		std::cout << "2) Medium (0 - 100)\n";
		std::cout << "3) Hard   (0 - 500)\n";

		const int choice = ReadInt("Enter 1, 2, or 3: ");

		if (choice == 1)
		{
			return {"Easy", 0, 50, CalculateTryLimit(0, 50)};
		}
		if (choice == 2)
		{
			return {"Medium", 0, 100, CalculateTryLimit(0, 100)};
		}
		if (choice == 3)
		{
			return {"Hard", 0, 500, CalculateTryLimit(0, 500)};
		}

		std::cout << "Invalid choice. Please select 1, 2, or 3.\n";
	}
}

int GetGuess(int minValue, int maxValue, int triesLeft)
{
	while (true)
	{
		const int guess = ReadInt(
			"Enter your guess (" + std::to_string(minValue) + " - " +
			std::to_string(maxValue) + "), tries left: " + std::to_string(triesLeft) + ": ");

		if (guess < minValue || guess > maxValue)
		{
			std::cout << "Out of range. Please enter a number between "
				<< minValue << " and " << maxValue << ".\n";
			continue;
		}

		return guess;
	}
}

void DisplayResult(int secretNumber, bool guessedCorrectly, int triesLeft)
{
	if (guessedCorrectly)
	{
		std::cout << "You guessed it! The number was " << secretNumber
			<< ". Tries remaining: " << triesLeft << "\n";
	}
	else
	{
		std::cout << "Out of tries. The number was " << secretNumber << ".\n";
	}
}

bool WantToPlayAgain()
{
	char input = '\0';

	while (true)
	{
		std::cout << "\nWould you like to play again? (y/n): ";
		std::cin >> input;

		if (std::cin.fail())
		{
			std::cin.clear();
			ClearInputLine();
			std::cout << "Input error. Please try again.\n";
			continue;
		}

		ClearInputLine();
		input = static_cast<char>(std::tolower(static_cast<unsigned char>(input)));

		if (input == 'y')
		{
			return true;
		}
		if (input == 'n')
		{
			return false;
		}

		std::cout << "Please enter Y (yes) or N (no).\n";
	}
}

void PlayGame(std::mt19937& rng)
{
	const GameSettings settings = SelectDifficulty();
	const std::uniform_int_distribution<int> dist(settings.minValue, settings.maxValue);
	const int secretNumber = dist(rng);

	int triesLeft = settings.tries;
	bool guessedCorrectly = false;

	std::cout << "\nDifficulty: " << settings.name << '\n';
	std::cout << "Guess the number between " << settings.minValue
		<< " and " << settings.maxValue << ".\n";
	std::cout << "You have " << triesLeft << " tries.\n";

	while (triesLeft > 0)
	{
		const int guess = GetGuess(settings.minValue, settings.maxValue, triesLeft);

		if (guess == secretNumber)
		{
			guessedCorrectly = true;
			break;
		}

		--triesLeft;

		if (triesLeft > 0)
		{
			if (guess > secretNumber)
			{
				std::cout << "Too high!\n";
			}
			else
			{
				std::cout << "Too low!\n";
			}
		}
	}

	DisplayResult(secretNumber, guessedCorrectly, triesLeft);
}
} // namespace

int main()
{
	std::random_device rd;
	std::mt19937 rng(rd());

	do
	{
		PlayGame(rng);
	} while (WantToPlayAgain());

	return 0;
}
