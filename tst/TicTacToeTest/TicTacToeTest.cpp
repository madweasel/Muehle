/**************************************************************************************************************************
	TicTacToeTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ticTacToe.h"

using ::testing::ElementsAre;

#pragma region gameState

TEST(ticTacToe, gameState)
{
	ticTacToe::gameState myGameState{};

	EXPECT_TRUE(myGameState.areValuesEqual(0,0,0,0));
	EXPECT_TRUE(myGameState.areValuesEqual(1,1,1,1));
	EXPECT_FALSE(myGameState.areValuesEqual(0,0,0,1));
	EXPECT_FALSE(myGameState.areValuesEqual(1,1,1,0));

	EXPECT_FALSE(myGameState.hasPlayerWon(gameId::egoPlayerId));
	EXPECT_FALSE(myGameState.hasPlayerWon(gameId::oppPlayerId));
	EXPECT_EQ(myGameState.getNumStonesOnField(), 0);

	myGameState.field[0] = gameId::egoPlayerId;
	myGameState.field[1] = gameId::egoPlayerId;
	myGameState.field[2] = gameId::egoPlayerId;
	EXPECT_TRUE (myGameState.hasPlayerWon(gameId::egoPlayerId));
	EXPECT_FALSE(myGameState.hasPlayerWon(gameId::oppPlayerId));
	EXPECT_EQ(myGameState.getNumStonesOnField(), 3);

	myGameState.invert();
	EXPECT_EQ(myGameState.field[0], gameId::oppPlayerId);
	EXPECT_EQ(myGameState.field[1], gameId::oppPlayerId);
	EXPECT_EQ(myGameState.field[2], gameId::oppPlayerId);
}

#pragma endregion

#pragma region ticTacToe

class ticTacToe_Test : public testing::Test {

protected:
	ticTacToe::stateAddressingTypeB saType{};
	ticTacToe 						myGame{saType};
	const wstring 					tmpFileDirectory 	= (std::filesystem::temp_directory_path() / "ticTacToe" / "database").c_str();

	void SetUp() override {
		if (std::filesystem::exists(tmpFileDirectory)) {
			std::filesystem::remove_all(tmpFileDirectory);
		}
		std::filesystem::create_directories(tmpFileDirectory);
	}

	void testCalcDatabase();
};

TEST_F(ticTacToe_Test, simpleFunctions) 
{
	EXPECT_EQ(ticTacToe::ipow(0, 0), 1);
	EXPECT_EQ(ticTacToe::ipow(2, 0), 1);
	EXPECT_EQ(ticTacToe::ipow(0, 2), 0);
	EXPECT_EQ(ticTacToe::ipow(2, 3), 8);
	EXPECT_EQ(ticTacToe::ipow(3, 2), 9);
	EXPECT_EQ(ticTacToe::ipow(3, 3), 27);

	// locals
	float 										floatValue;
	miniMax::twoBit 							shortValue;
 	vector<unsigned int> 						possibilityIds;
	vector<miniMax::retroAnalysis::predVars> 	predVars;
	vector<unsigned int> 						succLayers;
	vector<miniMax::stateAdressStruct>			symStates;
	unsigned int 								layerNum;
	unsigned int 								stateNumber;
	unsigned int								symOp;

	// empty field
	EXPECT_TRUE(myGame.setSituation(0, saType.emptyField_layer, saType.emptyField_state));						// set the empty field
	myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
	EXPECT_EQ(myGame.getNumberOfLayers(), 10);						// 10 layers
	EXPECT_EQ(myGame.getMaxNumPlies(), 9);							// 9 plies
	EXPECT_EQ(myGame.getMaxNumPossibilities(), 9);					// 9 possibilities
	EXPECT_EQ(myGame.hasAnyBodyWon(), false);						// nobody has won yet, since the game is just started
	EXPECT_EQ(myGame.shallRetroAnalysisBeUsed(0), false);			// no retro analysis
	myGame.getPossibilities(0, possibilityIds);		
	EXPECT_EQ(possibilityIds.size(), 9);							// still all possibilities are free
	myGame.getValueOfSituation(0, floatValue, shortValue);			// no value yet
	EXPECT_EQ(floatValue, 0.0f);		
	EXPECT_EQ(shortValue, miniMax::SKV_VALUE_GAME_DRAWN);		
	EXPECT_FALSE(myGame.lostIfUnableToMove(0));						// if no move is possible, the game is lost
	EXPECT_TRUE(myGame.isStateIntegrityOk(0));						// the state is valid
	myGame.getPredecessors(0, predVars);							// no predecessors, since the field is empty
	EXPECT_EQ(predVars.size(), 0);		
	EXPECT_EQ(myGame.getLayerNumber(0), saType.emptyField_layer);	// empty field means no master stone
	myGame.getLayerAndStateNumber(0, layerNum, stateNumber, symOp);	// should be 9, 0, 0
	EXPECT_EQ(layerNum, saType.emptyField_layer);
	EXPECT_EQ(stateNumber, saType.emptyField_state);
	EXPECT_EQ(symOp, 0);
	myGame.getSymStateNumWithDuplicates(0, symStates);				// only one state, namely the empty field
	miniMax::stateAdressStruct emptyFieldState{saType.emptyField_state,saType.emptyField_layer};
	EXPECT_THAT(symStates, ElementsAre(emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState));			// 8 times the same state

	if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeA)) {
		myGame.getSuccLayers(9, succLayers);							// 
		EXPECT_EQ(succLayers.size(), 9);								// 9 possibilities to set the master stone + the empty field
	
		// try setting invalid situations
		EXPECT_FALSE(myGame.setSituation(0, 9, 513));					// invalid state number
		EXPECT_FALSE(myGame.setSituation(0, 10, 0));					// invalid layer number
		EXPECT_FALSE(myGame.setSituation(100, 0, 0));					// invalid thread number

		// set the master on the first field
		EXPECT_FALSE(myGame.setSituation(0, 0, 0));						// set the master stone on the first field. this is invalid, since it is also the current player
		myGame.getValueOfSituation(0, floatValue, shortValue);
		myGame.printField(0, miniMax::SKV_VALUE_INVALID, 1);
		myGame.applySymOp(0, ticTacToe::symmetryVars::symOp::NONE, 		false, false);							// apply no symmetry
		myGame.applySymOp(0, ticTacToe::symmetryVars::symOp::VERTICAL, 	false, false);							// mirror vertically
		myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_INVALID);
		EXPECT_TRUE(myGame.setSituation(0, 0, 1));
		myGame.getValueOfSituation(0, floatValue, shortValue);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_GAME_DRAWN);
		myGame.printField(0, shortValue, 1);
		EXPECT_FALSE(myGame.setSituation(0, 0, 2));
		myGame.getValueOfSituation(0, floatValue, shortValue);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_INVALID);
		myGame.printField(0, shortValue, 1);

		// number of states without symmetry
		EXPECT_EQ(myGame.getNumberOfKnotsInLayer(0), ticTacToe::ipow(2, 0) * ticTacToe::ipow(3, 8));	// 0 fields before and 8 fields after the master stone
		EXPECT_EQ(myGame.getNumberOfKnotsInLayer(1), ticTacToe::ipow(2, 1) * ticTacToe::ipow(3, 7));	// 1 field  before and 7 fields after the master stone
		EXPECT_EQ(myGame.getNumberOfKnotsInLayer(9), ticTacToe::ipow(2, 9) * ticTacToe::ipow(3, 0));	// 9 fields before and 0 fields after the master stone
	} else if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeB)) {
		myGame.getSuccLayers(saType.emptyField_layer, succLayers);		// 
		EXPECT_EQ(succLayers.size(), 1);								// only one layers follows
	
		// try setting invalid situations
		EXPECT_FALSE(myGame.setSituation(0, 1, 19));					// invalid state number
		EXPECT_FALSE(myGame.setSituation(0, 10, 0));					// invalid layer number
		EXPECT_FALSE(myGame.setSituation(100, 0, 0));					// invalid thread number

		// set the master on the first field
		EXPECT_FALSE(myGame.setSituation(0, 9, 1));						// set the master stone on the first field. this is invalid, since 
		myGame.getValueOfSituation(0, floatValue, shortValue);
		myGame.printField(0, miniMax::SKV_VALUE_INVALID, 1);
		myGame.applySymOp(0, ticTacToe::symmetryVars::symOp::NONE, 		false, false);							// apply no symmetry
		myGame.applySymOp(0, ticTacToe::symmetryVars::symOp::VERTICAL, 	false, false);							// mirror vertically
		myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_INVALID);
		EXPECT_TRUE(myGame.setSituation(0, 7, 1));
		myGame.getValueOfSituation(0, floatValue, shortValue);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_GAME_DRAWN);
		myGame.printField(0, shortValue, 1);
		EXPECT_FALSE(myGame.setSituation(0, 7, 3));
		myGame.getValueOfSituation(0, floatValue, shortValue);
		EXPECT_EQ(shortValue, miniMax::SKV_VALUE_INVALID);
		myGame.printField(0, shortValue, 1);

		EXPECT_EQ(myGame.getNumberOfKnotsInLayer(9), 1);
		EXPECT_EQ(myGame.getNumberOfKnotsInLayer(0), 102);
	}
}

TEST_F(ticTacToe_Test, setSituationAndGetStateNumber) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_TRUE(myGame.mm.checker.testSetSituationAndGetStateNum(layerNumber));
	}
}

TEST_F(ticTacToe_Test, moveAndUndo) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_TRUE(myGame.mm.checker.testMoveAndUndo(layerNumber));
	}
}

TEST_F(ticTacToe_Test, getPredecessors) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_TRUE(myGame.mm.checker.testGetPredecessors(layerNumber));
	}
}

TEST_F(ticTacToe_Test, getPossibilities) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_TRUE(myGame.mm.checker.testGetPossibilities(layerNumber));
	}
}

TEST_F(ticTacToe_Test, NullptrAndZeros) 
{
	unsigned int								symOp					= 0;
	unsigned int								stateNum				= 0;
	unsigned int								layerNum				= 0;
	float										floatValue				= 0;
	miniMax::twoBit								shortValue				= 0;
	miniMax::plyInfoVarType						plyInfo					= 0;
	bool										playnerToMoveChanged	= false;
	void*										pBackup					= nullptr;
	vector<miniMax::retroAnalysis::predVars>	predVars;
	vector<unsigned int>						succLayers;
	vector<miniMax::stateAdressStruct>			symStates;
	vector<unsigned int>						possibiltyIds;

	// expected values
	unsigned int								validLayerNumber;
	unsigned int								validStateNumber;
	float 										expFloatValue;
	miniMax::twoBit 							expShortValue;
	unsigned int 								expNumberOfKnotsInLayer;
	::testing::Matcher<vector<unsigned int>> 	expPartnerLayers;
	::testing::Matcher<vector<unsigned int>> 	expSuccedingLayers;
	unsigned int 								expPredVarsSize;

	if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeA)) {
		validLayerNumber 		= 0;
		validStateNumber 		= 1;
		expNumberOfKnotsInLayer = (unsigned int) pow(3, 8);
		expPartnerLayers 		= ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9);
		expSuccedingLayers 		= ElementsAre();
		expPredVarsSize			= 0;
		expFloatValue 			= miniMax::FPKV_INV_VALUE;
		expShortValue 			= miniMax::SKV_VALUE_INVALID;
	} else if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeB)) {
		validLayerNumber 		= 7;
		validStateNumber 		= 1;
		expNumberOfKnotsInLayer = 102;
		expPartnerLayers 		= ElementsAre(layerNum);
		expSuccedingLayers 		= ElementsAre();
		expPredVarsSize			= 1;
		expFloatValue 			= miniMax::FPKV_INV_VALUE;
		expShortValue 			= miniMax::SKV_VALUE_INVALID;
	} else {
		FAIL() << "Unknown state addressing type!";
	}

	myGame.mm.setNumThreads(4);

	// robustness tests with nullptr and zeros and also invalid threadNo
	for (unsigned int threadNo = 0; threadNo <= 5; threadNo++) {

		// getter
		ASSERT_EQ(myGame.getLayerNumber(threadNo),									threadNo < 4 ? 0 : myGame.getNumberOfLayers());
		ASSERT_EQ(myGame.getNumberOfKnotsInLayer(layerNum),							expNumberOfKnotsInLayer);
		ASSERT_EQ(myGame.getNumberOfLayers(),										saType.numLayers);
		ASSERT_THAT(myGame.getPartnerLayers(layerNum), 								expPartnerLayers);
		ASSERT_EQ(myGame.getOutputInformation(layerNum),							wstring(L""));
		ASSERT_EQ(myGame.shallRetroAnalysisBeUsed(layerNum),						false);
		myGame.getPossibilities(threadNo, possibiltyIds);
		myGame.getPredecessors(threadNo, predVars);									EXPECT_EQ(predVars.size(), expPredVarsSize);
		myGame.getSuccLayers(layerNum, succLayers);									ASSERT_THAT(succLayers, expSuccedingLayers);
		myGame.getSymStateNumWithDuplicates(threadNo, symStates);					EXPECT_EQ(symStates.size(), 8);
		myGame.getLayerAndStateNumber(threadNo, layerNum, stateNum, symOp);			EXPECT_EQ(layerNum, 0);	EXPECT_EQ(stateNum, 0);	EXPECT_EQ(symOp, 0);
		myGame.getValueOfSituation(threadNo, floatValue, shortValue);				ASSERT_EQ(floatValue, expFloatValue); ASSERT_EQ(shortValue, expShortValue);

		// setter
		ASSERT_EQ(myGame.setSituation(threadNo, validLayerNumber, validStateNumber),threadNo < 4 ? true : false);
		myGame.move(threadNo, 0, playnerToMoveChanged, pBackup);
		myGame.undo(threadNo, 0, playnerToMoveChanged, pBackup);

		// output
		myGame.printField(threadNo, miniMax::SKV_VALUE_GAME_DRAWN, 0);
		myGame.printMoveInformation(threadNo, 0);
	}
}

void ticTacToe_Test::testCalcDatabase()
{
	ASSERT_EQ(myGame.mm.openDatabase(tmpFileDirectory), 											true);
	ASSERT_EQ(myGame.mm.calculateDatabase(), 														true);
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_EQ(myGame.mm.checker.testLayer(layerNumber),											true);
		EXPECT_EQ(myGame.mm.checker.testIfSymStatesHaveSameValue(layerNumber),						true);
	}
	myGame.mm.closeDatabase();

	// check some moves if they calculated correctly
	unsigned int 			choice;
	bool 					playerToMoveChanged;
	void *					pBackup;
	miniMax::stateInfo 		infoAboutChoices;
	vector<unsigned int> 	possibilityIds;

	unsigned int upperLeftLayer;
	unsigned int upperLeftState;

	if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeA)) {
		upperLeftLayer = 9;
		upperLeftState = 1;
	} else if (typeid(saType) == typeid(ticTacToe::stateAddressingTypeB)) {
		upperLeftLayer = 8;
		upperLeftState = 0;
	} else {
		FAIL() << "Unknown state addressing type!";
	}

	// ask for best move
	EXPECT_EQ(myGame.mm.openDatabase(tmpFileDirectory),												true);		// open the database
	ASSERT_EQ(myGame.setSituation(0, upperLeftLayer, upperLeftState),								true);		// set one stone into the upper left corner
	myGame.mm.getBestChoice(choice, infoAboutChoices);															// ask for the best move
	myGame.printField(0, infoAboutChoices.shortValue, 0);
	EXPECT_EQ(choice, 4);																						// the best move is now only the center

	// set a stone
	myGame.getPossibilities(0, possibilityIds);
	EXPECT_EQ(possibilityIds.size(),																8);
	myGame.move(0, possibilityIds[0], playerToMoveChanged, pBackup);											// do a dump mopve and set a stone above the center
	EXPECT_EQ(playerToMoveChanged,																	true);

	// ask again for best move
	vector<unsigned int> 	v1{3,4,6};																			// now the best move is the lower left corner, the center, or left from the center
	myGame.mm.getBestChoice(choice, infoAboutChoices);
	myGame.printField(0, infoAboutChoices.shortValue, 0);
	EXPECT_EQ(find(v1.begin(), v1.end(), choice) != v1.end(),										true);		

	myGame.mm.closeDatabase();
}

TEST_F(ticTacToe_Test, calcDatabase_alphaBeta) 
{
	// calculate the database using alpha-beta algorithm
	myGame.useRetroAnalysis = false;
	testCalcDatabase();
}

TEST_F(ticTacToe_Test, calcDatabase_retroAnalysis) 
{
	// calculate the database using retro analysis algorithm
	myGame.mm.setNumThreads(8);
	myGame.useRetroAnalysis = true;
	testCalcDatabase();
}

TEST_F(ticTacToe_Test, GameAgainstUser)
{
	GTEST_SKIP();

	// calculate the database
	EXPECT_EQ(myGame.mm.openDatabase(tmpFileDirectory), 											true);
	ASSERT_EQ(myGame.mm.calculateDatabase(), 														true);
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		EXPECT_EQ(myGame.mm.checker.testLayer(layerNumber),											true);
		EXPECT_EQ(myGame.mm.checker.testIfSymStatesHaveSameValue(layerNumber),						true);
	}
	myGame.mm.closeDatabase();

	// let user try to beat the database
	EXPECT_TRUE(myGame.mm.openDatabase(tmpFileDirectory));
	srand((unsigned int)time(NULL));
	
	// locals
	bool					humansTurn 		= rand() % 2;
	vector<unsigned int> 	possIds;
	unsigned char   		pos;
	unsigned int			choice;
	miniMax::stateInfo		infoAboutChoices;

	// set the empty field
	myGame.setSituation(0, 9, 0);

	while (true) {

		// print the field
		myGame.mm.getBestChoice(choice, infoAboutChoices);
		myGame.printField(0, infoAboutChoices.shortValue, 0);
		cout << endl << "Num plies till end: " << infoAboutChoices.plyInfo << endl;

		// get the possibilities
		myGame.getPossibilities(0, possIds);
		if (myGame.hasAnyBodyWon()) break;
		if (!possIds.size()) 		break;

		// ask user for his cross
		if (humansTurn) {
			do {
				cout << endl << "Choose a position: ";
				cin >> pos;
			} while (!myGame.setStone(pos - 48));
			EXPECT_FALSE(myGame.hasAnyBodyWon());
			humansTurn = false;
		}

		// let computer make his move
		myGame.letComputerSetStone();
		humansTurn = true;
	}
	wcout << "\n";
}

#pragma endregion
