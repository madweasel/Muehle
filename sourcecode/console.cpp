#include <cstdio>
#include <iostream>
#include <windows.h>
#include "muehle.h"
#include "minMaxKI.h"
#include "randomKI.h"
#include "perfectKI.h"

using namespace std;

unsigned int	startTestFromLayer		= 0;
unsigned int	endTestAtLayer			= NUM_LAYERS-1;
#ifdef _DEBUG
	char		databaseDirectory[]		= "D:\\M�hle\\M�hle_Debug";
#elif _RELEASE_X64
	char		databaseDirectory[]		= ""; // "D:\\M�hle\\M�hle_Debug";
#endif
bool			calculateDatabase		= true;
unsigned int	compressionAlgorithmnId	= COMPRESSOR_ALG_UNCOMPRESSED;			// COMPRESSOR_ALG_EASYZLIB | COMPRESSOR_ALG_UNCOMPRESSED

void main(void)
{
	// locals
	bool			playerOneHuman		= false;
	bool			playerTwoHuman		= false;
	char			tmpChar[100];
	unsigned int	pushFrom, pushTo;
	muehle*			myGame				= new muehle();
	perfectKI*		myKI				= new perfectKI(databaseDirectory);

	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    srand(GetTickCount());

	// intro
    cout << "*************************" << endl;
    cout << "* Muehle                *" << endl;
    cout << "*************************" << endl << endl;

	myKI->setDatabasePath(databaseDirectory);

	// begin
	myGame->beginNewGame(myKI, myKI, (rand() % 2) ? fieldStruct::playerOne : fieldStruct::playerTwo);

	if (calculateDatabase) {

		// calculate
		myKI->calculateDatabase(MAX_DEPTH_OF_TREE, false, compressionAlgorithmnId);

		// test database
		cout << endl << "Begin test starting from layer: ";     startTestFromLayer;
	    cout << endl << "End test at layer: ";                  endTestAtLayer;
		myKI->testLayers(startTestFromLayer, endTestAtLayer);

	} else {

		cout << "Ist Spieler 1 ein Mensch? (j/n):"; cin >> tmpChar;	if (tmpChar[0] == 'j') playerOneHuman = true;
		cout << "Ist Spieler 2 ein Mensch? (j/n):"; cin >> tmpChar;	if (tmpChar[0] == 'j') playerTwoHuman = true;

		// play
		do
		{
			// print field
			cout << "\n\n\n\n\n\n\n\n\n\n\n";
			myGame->getComputersChoice(&pushFrom, &pushTo);
			cout << "\n\n";
			cout << "\nlast move was from " << (char)(myGame->getLastMoveFrom() + 97) << " to " << (char)(myGame->getLastMoveTo() + 97) << "\n\n";
			
			myGame->printField();
			 
			// Human
			if ((myGame->getCurrentPlayer() == fieldStruct::playerOne && playerOneHuman)
			||  (myGame->getCurrentPlayer() == fieldStruct::playerTwo && playerTwoHuman)) {
				do {
					// Show text
					if (myGame->mustStoneBeRemoved())	cout << "\n   Welchen Stein moechten Sie entfernen? [a-x]: \n\n\n";
					else if (myGame->inSettingPhase())	cout << "\n   Wohin setzen Sie? [a-x]: \n\n\n";
					else								cout << "\n   Ihr Zug? [a-x][a-x]: \n\n\n";
						
					// get input
					cin >> tmpChar;
					if ((tmpChar[0] >= 'a') && (tmpChar[0] <= 'x'))		pushFrom	= tmpChar[0] - 97;	else pushFrom	= fieldStruct::size;
					
					if (myGame->inSettingPhase()) {
						if ((tmpChar[0] >= 'a') && (tmpChar[0] <= 'x'))	pushTo		= tmpChar[0] - 97;	else pushTo		= fieldStruct::size;
					} else {
						if ((tmpChar[1] >= 'a') && (tmpChar[1] <= 'x'))	pushTo		= tmpChar[1] - 97;	else pushTo		= fieldStruct::size;
					}

					// undo
					if (tmpChar[0] == 'u' && tmpChar[1] == 'n' && tmpChar[2] == 'd' && tmpChar[3] == 'o') { 
						
						// undo moves until a human player shall move
						do  {
							myGame->undoLastMove();										
						} while (!((myGame->getCurrentPlayer() == fieldStruct::playerOne && playerOneHuman) 
							   ||  (myGame->getCurrentPlayer() == fieldStruct::playerTwo && playerTwoHuman)));
						
						// reprint field
						break;
					}

				} while (myGame->moveStone(pushFrom, pushTo) == false);
			
			// Computer
			} else {
				cout << "\n";
				myGame->moveStone(pushFrom, pushTo);
			}

		} while (myGame->getWinner() == 0);

		// end
		cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
		
		myGame->printField();

			 if (myGame->getWinner() == fieldStruct::playerOne)	cout << "\n   Spieler 1 (o) hat nach " << myGame->getMovesDone() << " Zuegen gewonnen.\n\n";
		else if (myGame->getWinner() == fieldStruct::playerTwo)	cout << "\n   Spieler 2 (x) hat nach " << myGame->getMovesDone() << " Zuegen gewonnen.\n\n";
		else if (myGame->getWinner() == fieldStruct::gameDrawn)	cout << "\n   Unentschieden!\n\n";
		else												    cout << "\n   Ein Programmfehler ist aufgetreten!\n\n";
	}

	char end;
	cin >> end;
}
