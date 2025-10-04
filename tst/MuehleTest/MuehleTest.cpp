/**************************************************************************************************************************
	MuehleTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ai/perfectAI.h"
#include "muehle.h"

const wstring PATH_TO_COMPLETE_DATABASE = L"C:\\Coding\\Muehle\\database";

using ::testing::ElementsAre;

class muehle_Test : public testing::Test {

protected:
	perfectAI 			myAI{L"./database"};		// AI object
	muehle 				myGame{};

	void SetUp() override {
	}
};

TEST_F(muehle_Test, simpleFunctions)
{
	// locals
	fieldStruct 							field;
	std::vector<muehle::logItem>			moveLog;
	unsigned int 							moveLogId = 123;				// id of the current move in the log
	moveInfo								move;

	const unsigned int fieldStruct_size = fieldStruct::size;

	// zero & negative tests
	EXPECT_FALSE(myGame.moveStone(moveInfo{fieldStruct_size, fieldStruct_size, 0}));		// pass invalid positions
	myGame.undoLastMove();													// nothing to undo, but do not crash
	myGame.printField();													// just for coverage
	EXPECT_FALSE(myGame.putStone(fieldStruct_size, playerId::playerOne));	// try to set a stone on an invalid position
	myGame.getComputersChoice(move);										// no AI has been set yet, so no choice can be made)
	EXPECT_EQ(move.from, fieldStruct_size);									// no move has been done yet
	EXPECT_EQ(move.to, fieldStruct_size);									// ''
	EXPECT_EQ(move.removeStone, fieldStruct_size);							// ''
	myGame.getChoiceOfSpecialAI(nullptr, move);								// no AI has been set yet, so no choice can be made)
	EXPECT_EQ(move.from, fieldStruct_size);									// no move has been done yet
	EXPECT_EQ(move.to, fieldStruct_size);									// ''
	EXPECT_EQ(move.removeStone, fieldStruct_size);							// ''
	myGame.getLog(moveLog, moveLogId);										// get the log
	EXPECT_THAT(moveLog, ElementsAre());									// the log is empty
	EXPECT_THAT(moveLogId, 0);												// the log is empty
	EXPECT_EQ(field.getStone(0),  playerId::squareIsFree);					// the first field position is free
	EXPECT_EQ(field.getStone(23), playerId::squareIsFree);					// the last  field position is free
	EXPECT_TRUE(myGame.isCurrentPlayerHuman());								// no AI has been set yet, so the current player is human
	EXPECT_TRUE(myGame.isOpponentPlayerHuman());							// ''
	EXPECT_TRUE(myGame.inSettingPhase());									// in setting phase
	EXPECT_EQ(myGame.mustStoneBeRemoved(), 0);								// no stone must be removed yet
	EXPECT_EQ(myGame.getWinner(), playerId::squareIsFree);					// no winner yet
	EXPECT_EQ(myGame.getCurrentPlayer(), playerId::playerOne);				// player one starts
	EXPECT_EQ(myGame.getMovesDone(), 0);									// no move has been done yet
	EXPECT_EQ(myGame.getNumStonesSet(), 0);									// no stone has been set yet
	EXPECT_EQ(myGame.getBeginningPlayer(), playerId::playerOne);			// by default player one starts
	EXPECT_EQ(myGame.getNumStonesOfCurPlayer(), 0);							// no stone has been set yet
	EXPECT_EQ(myGame.getNumStonesOfOppPlayer(), 0);							// ''

	// positive tests
	EXPECT_TRUE(myGame.moveStone(moveInfo{0, 1, fieldStruct::size}));									// place a stone on position 1 on the field
	myGame.undoLastMove();													// undo the last move
	myGame.beginNewGame(&myAI, nullptr, playerId::playerTwo, true, true);	// start a new game
	EXPECT_TRUE (myGame.isCurrentPlayerHuman());							// current player two is AI
	EXPECT_FALSE(myGame.isOpponentPlayerHuman());							// ''
	myGame.getComputersChoice(move);										// get the choice of the AI, although it is not the AI's turn
	EXPECT_EQ(move.from, fieldStruct_size);									// no move has been done yet, since humans turn
	EXPECT_EQ(move.to, fieldStruct_size);									// ''
	EXPECT_EQ(move.removeStone, fieldStruct_size);							// ''
	myGame.setAI(playerId::playerOne, nullptr);								// set the AI for player one
	myGame.setAI(playerId::playerTwo, &myAI);								// set player two to be human
	EXPECT_FALSE(myGame.isCurrentPlayerHuman());							//  
	EXPECT_TRUE (myGame.isOpponentPlayerHuman());							// ''
	myGame.getComputersChoice(move);										// get the choice of the AI, although it is not the AI's turn
	myGame.moveStone(move);													// move the stone
	EXPECT_TRUE (myGame.isCurrentPlayerHuman());							// current player is AI 
	EXPECT_FALSE(myGame.isOpponentPlayerHuman());							// ''
	EXPECT_EQ(myGame.getNumStonesOfCurPlayer(), 0);							// no stone has been set yet
	EXPECT_EQ(myGame.getNumStonesOfOppPlayer(), 1);							// ''
	EXPECT_EQ(myGame.getBeginningPlayer(), playerId::playerTwo);			// by default player one starts
	EXPECT_EQ(myGame.getMovesDone(), 1);									// one move has been done
	EXPECT_EQ(myGame.getNumStonesSet(), 1);									// one stone has been set
	EXPECT_EQ(myGame.getCurrentPlayer(), playerId::playerOne);				// player one is next
	EXPECT_EQ(myGame.getWinner(), playerId::squareIsFree);					// no winner yet
	EXPECT_EQ(myGame.mustStoneBeRemoved(), 0);								// no stone must be removed yet
	EXPECT_TRUE(myGame.inSettingPhase());									// still in setting phase
	myGame.getLog(moveLog, moveLogId);										// get the log
	EXPECT_THAT(moveLogId, 1);												// one move has been done
	// EXPECT_THAT(moveLog, ElementsAre(muehle::logItem{pushFrom, pushTo, playerId::playerTwo}));
	myGame.printField();													// just for coverage
}

TEST_F(muehle_Test, checkIfWonGameIsAlwaysWon)
{
	// Skip test if no database is available
	if (!std::filesystem::exists(PATH_TO_COMPLETE_DATABASE + L"\\plyInfo.dat")) {
		GTEST_SKIP() << L"Skipping test: Database file 'plyInfo.dat' not found in " << PATH_TO_COMPLETE_DATABASE;
	}

	// locals
	unsigned int numberOfTries = 100;		// Try this number of games, to find out if the perfectAI always wins a won game

	// open database
	ASSERT_EQ(myAI.mm.openDatabase(PATH_TO_COMPLETE_DATABASE), true);
	if (!myAI.mm.db.isComplete()) {
		GTEST_SKIP() << L"Skipping test: Database is not complete.";
	}
	myGame.setAI(playerId::playerOne, &myAI);
	myGame.setAI(playerId::playerTwo, &myAI);
	myGame.setNumMovesToRemis(1000);
	cout << "Try to finish starting from " << numberOfTries << " won game states..." << endl;
	cout << "Games won: ";

	// loop
	for (unsigned int i = 0; i < numberOfTries; i++) {

		moveInfo 					move;
		unsigned int 				numStatesInLayer;
		fieldStruct 				field;
		bool 						gameHasFinished = true;
		miniMax::stateAdressStruct	stateAdress;
		float 						floatValue;
		miniMax::twoBit 			shortValue;
		vector<unsigned int> 		possIds;
		playerId 					playerSupposedToWin;

		// create a random field
		field.reset();

		// set any random state, which is won
		while (true) {
			// set random state address
			stateAdress.layerNumber = rand() % myAI.getNumberOfLayers();
			numStatesInLayer 		= myAI.getNumberOfKnotsInLayer(stateAdress.layerNumber);
			// layer must not be empty
			if (!numStatesInLayer) {
				continue;
			}
			stateAdress.stateNumber = rand() % numStatesInLayer;
			// set state by stateAdress
			if (!myAI.setSituation(0, stateAdress.layerNumber, stateAdress.stateNumber)) {
				continue;
			}
			// state must be in database
			if (!myAI.mm.isCurrentStateInDatabase(0)) {
				continue;
			}
			// Game must not be finished yet
			myAI.getPossibilities(0, possIds);
			if (possIds.size() == 0) {
				continue;
			}
			// state must be a won game
			myAI.mm.db.readKnotValueFromDatabase(stateAdress.layerNumber, stateAdress.stateNumber, shortValue); // does not work since private
			if (shortValue != miniMax::SKV_VALUE_GAME_WON) {
				continue;
			}
			// set chosen state, if not a finished game
			myAI.getField(stateAdress.layerNumber, stateAdress.stateNumber, 3, field, gameHasFinished);
			if (gameHasFinished) {
				continue;
			}
			myGame.setCurrentGameState(field);
			playerSupposedToWin = myGame.getCurrentPlayer();
			break;
		}
		
		// play the game
		while (!gameHasFinished) {
			myGame.getChoiceOfSpecialAI(&myAI, move);
			if (!myGame.moveStone(move)) {
				cout << endl << "Player supposed to win: " << static_cast<unsigned int>(playerSupposedToWin) << endl;
				cout << "Initial Layer: " << static_cast<unsigned int>(stateAdress.layerNumber) 
				           << ", State: " << static_cast<unsigned int>(stateAdress.stateNumber) << endl;
				myGame.printField();
				myAI.printMoveInformation(0, move.getId());
				GTEST_FAIL() << "Move failed in game " << i;
			}
			if (myGame.getWinner() != playerId::squareIsFree) {
				ASSERT_EQ(myGame.getWinner(), playerSupposedToWin);
				gameHasFinished = true;
			}
		}
		cout << ".";
		std::cout.flush();
	}
	cout << endl << "All games won!" << endl;
	myAI.mm.closeDatabase();
}

#pragma region compFileTest
namespace miniMax {
class gameInterfaceStub : public gameInterface {
public:
	gameInterfaceStub() : gameInterface() {}
	~gameInterfaceStub() {}

	unsigned int	getNumberOfLayers				()															{ return 2;											};
	unsigned int	getNumberOfKnotsInLayer			(unsigned int layerNum)										{ return layerNum == 0 ? 100 : 200;					};
	void            getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)	{ succLayers.assign({1,0});							};
	uint_1d			getPartnerLayers				(unsigned int layerNum)										{ return layerNum == 0 ? uint_1d{1} : uint_1d{0};	};
};
}	// namespace miniMax

class MiniMaxDatabase_genericFileTest : public ::testing::Test {

public:
	static const unsigned int numKnotsInLayer0 = 100;
	static const unsigned int numKnotsInLayer1 = 200;
	static const unsigned int numSkvBytes0 		= (numKnotsInLayer0 + 3) / 4;
	static const unsigned int numSkvBytes1 		= (numKnotsInLayer1 + 3) / 4;

protected:
	logger log{logger::logLevel::none, logger::logType::none, L""};
	miniMax::gameInterfaceStub game{};
	miniMax::database::databaseStatsStruct dbStats{};
	vector<miniMax::database::layerStatsStruct> layerStats{};

	void SetUp() override {
		dbStats.numLayers = 2;
		dbStats.completed = false;

		layerStats.resize(2);
		
		layerStats[0].completedAndInFile = false;
		layerStats[0].partnerLayer 		= 1;
		layerStats[0].knotsInLayer 		= numKnotsInLayer0;
		layerStats[0].numWonStates 		= 10;
		layerStats[0].numLostStates 	= 5;
		layerStats[0].numDrawnStates 	= numKnotsInLayer0 - 15;
		layerStats[0].numInvalidStates 	= 0;
		layerStats[0].succLayers 		= {1, 0};
		layerStats[0].partnerLayers 	= {1};
		layerStats[0].skv 				= vector<miniMax::twoBit>(numSkvBytes0, miniMax::SKV_VALUE_INVALID);
		layerStats[0].plyInfo 			= vector<miniMax::plyInfoVarType>(numKnotsInLayer0, miniMax::PLYINFO_VALUE_INVALID);

		layerStats[1].completedAndInFile = true;
		layerStats[1].partnerLayer 		= 0;
		layerStats[1].knotsInLayer 		= numKnotsInLayer1;
		layerStats[1].numWonStates 		= 20;
		layerStats[1].numLostStates 	= 10;
		layerStats[1].numDrawnStates 	= numKnotsInLayer1 - 30;
		layerStats[1].numInvalidStates 	= 0;
		layerStats[1].succLayers 		= {1, 0};
		layerStats[1].partnerLayers		 = {0};
		layerStats[1].skv 				= vector<miniMax::twoBit>(numSkvBytes1, miniMax::SKV_VALUE_INVALID);
		layerStats[1].plyInfo 			= vector<miniMax::plyInfoVarType>(numKnotsInLayer1, miniMax::PLYINFO_VALUE_INVALID);
	}

	void TearDown() override {
	}
};

TEST(MiniMaxDatabase_compFile, database_dat)
{
	GTEST_SKIP();

	compressor::winCompApi					comp;
	compressor::file						file{comp};
	miniMax::database::databaseStatsStruct 	dbStats;
	miniMax::database::layerStatsStruct 	layerStats;

	file.open(std::wstring(PATH_TO_COMPLETE_DATABASE + L"\\database.dat"), true);
	auto keys = file.getKeys();
	auto it = std::find(keys.begin(), keys.end(), L"dbStats");
	EXPECT_GE(keys.size(), 1);
	ASSERT_NE(it, keys.end());
	file.read(L"dbStats", 0, sizeof(dbStats), &dbStats);
	EXPECT_EQ(dbStats.numLayers, 				200);
	EXPECT_EQ(dbStats.completed, 				true);
	file.read(wstring(L"layerStats") + to_wstring(50), 0, miniMax::database::layerStatsStruct::numBytesLayerStatsHeader, &layerStats);
	EXPECT_EQ(layerStats.completedAndInFile, 	true);
	EXPECT_EQ(layerStats.partnerLayer, 			49);
	EXPECT_EQ(layerStats.knotsInLayer, 			20996880);
	EXPECT_EQ(layerStats.numWonStates, 			10557);
	EXPECT_EQ(layerStats.numLostStates, 		9);
	EXPECT_EQ(layerStats.numDrawnStates,		11320702);
	EXPECT_EQ(layerStats.numInvalidStates, 		76);
	file.close();
}

TEST_F(MiniMaxDatabase_genericFileTest, database_dat)
{
	GTEST_SKIP();
	
	miniMax::database::compFile cf(&game, log);
	miniMax::plyInfoVarType plyInfo;
	miniMax::twoBit skvInfo;

	EXPECT_TRUE(cf.openDatabase(L"."));
	EXPECT_TRUE(cf.isOpen());
	EXPECT_TRUE(cf.loadHeader(dbStats, layerStats));
	EXPECT_EQ(dbStats.numLayers, 					200);
	EXPECT_EQ(dbStats.completed, 					true);
	EXPECT_EQ(layerStats[50].completedAndInFile, 	true);
	EXPECT_EQ(layerStats[50].partnerLayer, 			49);
	EXPECT_EQ(layerStats[50].knotsInLayer, 			20996880);
	EXPECT_EQ(layerStats[50].numWonStates, 			10557);
	EXPECT_EQ(layerStats[50].numLostStates, 		9);
	EXPECT_EQ(layerStats[50].numDrawnStates,		11320702);
	EXPECT_EQ(layerStats[50].numInvalidStates, 		76);
	EXPECT_TRUE(cf.readPlyInfo(50, plyInfo, 0));
	EXPECT_TRUE(cf.readSkv(50, skvInfo, 0));
	EXPECT_EQ(skvInfo, 170);						// 10101010	= SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_DRAWN 
	EXPECT_EQ(plyInfo, miniMax::PLYINFO_VALUE_DRAWN);
	cf.closeDatabase();
	EXPECT_FALSE(cf.isOpen());
}
#pragma endregion
