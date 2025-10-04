// TicTacToe.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdlib.h>     /* srand, rand */

#include "ticTacToe.h"

ticTacToe* myGame = nullptr;

int main()
{
    std::cout << "Hello World!\n";
	ticTacToe::stateAddressingTypeA sa;
	myGame = new ticTacToe(sa);
	bool humanToMove = (rand() % 2 == 0);

	myGame->prepareCalculation();

	while (!myGame->hasAnyBodyWon())
	{
		myGame->printField(0, 3, 4);

		if (humanToMove ) {
			// ask user to type in a position from 0 to 8
			std::cout << "Please type in a position from 0 to 8: ";
			unsigned int pos;
			std::cin >> pos;
			myGame->setStone(pos);
		} else {
			myGame->letComputerSetStone();
		}
		humanToMove = !humanToMove;
	}
}
