/**************************************************************************************************************************
	stateAddressingTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ai/stateAddressing.h"

using groupStateNumber 	= stateAddressing::groupStateNumber;
using groupIndex 		= stateAddressing::groupIndex;
using numWhiteStones 	= stateAddressing::numWhiteStones;
using numBlackStones 	= stateAddressing::numBlackStones;
using symOperationId 	= stateAddressing::symOperationId;
using subLayerId		= stateAddressing::subLayerId;
using stateId			= stateAddressing::stateId;
using layerId			= stateAddressing::layerId;

class StateAddressingTest : public ::testing::Test {
protected:

	// Constants to simplify the test
	const playerId x 	= playerId::playerOne;
	const playerId o 	= playerId::playerTwo;
	const playerId _ 	= playerId::squareIsFree;
	using FIELD  		= fieldStruct::fieldArray;

	// For example a field can be set like this:
	// FIELD({  _,    _,    _,
	//  		  o,  _,  _,
	//  		    _,_,_,
	//  	    _,_,_,  x,_,_,
	//  		    _,_,x,
	//  		  _,  _,  _,
	//  	    o,    _,    _});

	// variables
	fieldStruct field;
	const std::wstring tmpFileDirectory = (std::filesystem::temp_directory_path() / "Muehle" / "stateAddressing").c_str();

	void SetUp() override {
		field.reset(x);
	}
};

TEST_F(StateAddressingTest, test_with_empty_cache_file)
{
	// test with empty cache file
	if (std::filesystem::exists(tmpFileDirectory)) {
		std::filesystem::remove_all(tmpFileDirectory);
	}
	std::filesystem::create_directories(tmpFileDirectory);
	stateAddressing sa(tmpFileDirectory);
	
	EXPECT_EQ(sa.getLayerNumber(0, 0, true), 199);
}

TEST_F(StateAddressingTest, test_getStateNumber_getFieldByStateNumber_manual)
{
	stateAddressing sa(tmpFileDirectory);
	fieldStruct 	field;
	fieldStruct 	fieldTmp;
	field.setSituation({
	  o,    _,    _,
		_,  _,  o,
		  _,_,_,
	  _,_,_,  _,o,_,
		  x,x,x,
		_,  x,  _,
	  _,    _,    _}, false, 0);
	layerId 		layerNumber = sa.getLayerNumber(field);
	symOperationId 	symOp;
	stateId 		stateNumber, stateNumberTmp;
	sa.getStateNumber(layerNumber, stateNumber, symOp, field);			// get the state number of the field
	EXPECT_EQ(layerNumber, 32);
	EXPECT_EQ(stateNumber, 400268);
	EXPECT_EQ(symOp, stateAddressing::SO_TURN_LEFT);
	sa.getFieldByStateNumber(layerNumber, stateNumber, fieldTmp, x);	// get again the field from the state number
	sa.applySymmetryTransfToField(symOp, true, fieldTmp);				// apply the symmetry operation
	sa.getStateNumber(layerNumber, stateNumberTmp, symOp, fieldTmp);	// get the state number of the field again
	EXPECT_EQ(field, 		fieldTmp);									// the field must be the same, after applying the symmetry operation
	EXPECT_EQ(stateNumber, 	stateNumberTmp);							// the state number must be the same in both cases
}

TEST_F(StateAddressingTest, internal_variables)
{
	stateAddressing sa(tmpFileDirectory);

	// check that each index occurs exactly one time in the arrays squareIndexGroupA, squareIndexGroupB, squareIndexGroupC, squareIndexGroupD
	{
		std::vector<unsigned int> indices;
		for (const auto& index : sa.squareIndexGroupA) { indices.push_back(index); }
		for (const auto& index : sa.squareIndexGroupB) { indices.push_back(index); }
		for (const auto& index : sa.squareIndexGroupC) { indices.push_back(index); }
		for (const auto& index : sa.squareIndexGroupD) { indices.push_back(index); }
		std::sort(indices.begin(), indices.end());
		auto it = std::unique(indices.begin(), indices.end());
		EXPECT_EQ(it, indices.end());
	}

	// check that the number of squares belonging to each group listed in fieldPosIdOfGroup matches the numSquaresGroup
	{
		unsigned int numSquaresGroupA = 0;
		unsigned int numSquaresGroupB = 0;
		unsigned int numSquaresGroupC = 0;
		unsigned int numSquaresGroupD = 0;
		for (const auto& group : sa.fieldPosIsOfGroup) {
			if (group == sa.GROUP_A) numSquaresGroupA++;
			if (group == sa.GROUP_B) numSquaresGroupB++;
			if (group == sa.GROUP_C) numSquaresGroupC++;
			if (group == sa.GROUP_D) numSquaresGroupD++;
		}
		EXPECT_EQ(numSquaresGroupA, sa.numSquaresGroupA);
		EXPECT_EQ(numSquaresGroupB, sa.numSquaresGroupB);
		EXPECT_EQ(numSquaresGroupC, sa.numSquaresGroupC);
		EXPECT_EQ(numSquaresGroupD, sa.numSquaresGroupD);
	}

	// check that amountSituationsAB[nws][nbs] is equal or smaller then MAX_NUM_SITUATIONS_A * MAX_NUM_SITUATIONS_B
	{
		for (numWhiteStones nws = 0; nws < sa.NUM_STONES_PER_PLAYER+1; nws++) {
			for (numBlackStones nbs = 0; nbs < sa.NUM_STONES_PER_PLAYER+1; nbs++) {
				EXPECT_LE(sa.amountSituationsAB[nws][nbs], sa.MAX_NUM_SITUATIONS_A * sa.MAX_NUM_SITUATIONS_B);
				EXPECT_LE(sa.amountSituationsCD[nws][nbs], sa.MAX_NUM_SITUATIONS_C * sa.MAX_NUM_SITUATIONS_D);
			}
		}
	}

	// check that groupIndex[] and groupState[][][] machtes each other
	{
		for (numWhiteStones nws = 0; nws < sa.NUM_STONES_PER_PLAYER+1; nws++) {
			for (numBlackStones nbs = 0; nbs < sa.NUM_STONES_PER_PLAYER+1; nbs++) {
				for (groupIndex index = 0; index < sa.MAX_NUM_SITUATIONS_A * sa.MAX_NUM_SITUATIONS_B; index++) {
					if (index >= sa.amountSituationsAB[nws][nbs]) break;
					groupStateNumber stateNumber = sa.groupStateAB[nws][nbs][index];
					EXPECT_EQ(sa.groupIndexAB[stateNumber], index);
				}
				for (groupIndex index = 0; index < sa.MAX_NUM_SITUATIONS_C * sa.MAX_NUM_SITUATIONS_D; index++) {
					if (index >= sa.amountSituationsCD[nws][nbs]) break;
					groupStateNumber stateNumber = sa.groupStateCD[nws][nbs][index];
					EXPECT_EQ(sa.groupIndexCD[stateNumber], index);
				}
			}
		}
	}	

	// check symmetryOperationCD[]
	{
		// every current considered state as a corresponding base state, which is symmetric
		groupStateNumber 		curStateNumber;		// the state number for the current state
		groupIndex 				curGroupIndex;		// the group index for the current state
		fieldStruct::fieldArray curField;			// the field for the current state
		groupStateNumber		baseStateNumber;	// the state number for the base state, listed in groupIndexCD
		groupIndex 				baseGroupIndex;		// the group index for the base state
		fieldStruct::fieldArray baseField;			// the field for the base state
		symOperationId 			symOp;				// symmetry operation for the current state, to get to the base state
		
		for (curStateNumber = 0; curStateNumber < sa.MAX_NUM_SITUATIONS_C * sa.MAX_NUM_SITUATIONS_D; curStateNumber++) {
			symOp 		  = sa.symmetryOperationCD[curStateNumber];
			curGroupIndex = sa.groupIndexCD[curStateNumber];
			// get curField from curStateNumber
			sa.calcFieldBasedOnGroupCD(curField, curStateNumber);
			// apply symmetry operation to get to the base state
			sa.applySymmetryTransfToField(symOp, false, curField, baseField);
			// get stateNumber from baseField
			sa.calcGroupStateNumberCD(baseField, baseStateNumber);
			// check if the base group index is the same as the current group index
			baseGroupIndex = sa.groupIndexCD[baseStateNumber];
			EXPECT_EQ(curGroupIndex, baseGroupIndex);
		}
	}

	// check powerOfThree
	{
		for (unsigned int i = 0; i < 16; i++) {
			EXPECT_EQ(sa.powerOfThree[i], std::pow(3, i));
		}
	}

	// check mOverN
	{
		EXPECT_EQ(sa.mOverN[0][0], 1);
		EXPECT_EQ(sa.mOverN[1][0], 1);
		EXPECT_EQ(sa.mOverN[1][1], 1);
		EXPECT_EQ(sa.mOverN[2][1], 2);
		EXPECT_EQ(sa.mOverN[5][3], 10);
		EXPECT_EQ(sa.mOverN[5][2], 10);
	}

	// check reverseSymOperation
	{
		EXPECT_EQ(sa.reverseSymOperation[sa.SO_DO_NOTHING], sa.SO_DO_NOTHING);
		EXPECT_EQ(sa.reverseSymOperation[sa.SO_TURN_LEFT], sa.SO_TURN_RIGHT);
		EXPECT_EQ(sa.reverseSymOperation[sa.SO_TURN_180], sa.SO_TURN_180);
	}

	// check concSymOperation
	{
		EXPECT_EQ(sa.concSymOperation[sa.SO_DO_NOTHING][sa.SO_DO_NOTHING], sa.SO_DO_NOTHING);
		EXPECT_EQ(sa.concSymOperation[sa.SO_TURN_LEFT][sa.SO_TURN_RIGHT], sa.SO_DO_NOTHING);
		EXPECT_EQ(sa.concSymOperation[sa.SO_TURN_RIGHT][sa.SO_TURN_LEFT], sa.SO_DO_NOTHING);
		EXPECT_EQ(sa.concSymOperation[sa.SO_TURN_LEFT][sa.SO_TURN_LEFT], sa.SO_TURN_180);
		EXPECT_EQ(sa.concSymOperation[sa.SO_TURN_180][sa.SO_TURN_LEFT], sa.SO_TURN_RIGHT);
		EXPECT_EQ(sa.concSymOperation[sa.SO_TURN_LEFT][sa.SO_TURN_180], sa.SO_TURN_RIGHT);
	}

	// check that each index occurs exactly one time in layerIndex[][][]
	{
		std::vector<unsigned int> indices;
		for (numWhiteStones nws = 0; nws < sa.NUM_STONES_PER_PLAYER+1; nws++) {
			for (numBlackStones nbs = 0; nbs < sa.NUM_STONES_PER_PLAYER+1; nbs++) {
				for (unsigned int phase = 0; phase < 2; phase++) {
					indices.push_back(sa.layerIndex[phase][nws][nbs]);
				}
			}
		}
		std::sort(indices.begin(), indices.end());
		auto it = std::unique(indices.begin(), indices.end());
		EXPECT_EQ(it, indices.end());
	}

	// check layer[]
	{
		for (numWhiteStones nwsAB = 0; nwsAB < sa.NUM_STONES_PER_PLAYER; nwsAB++) {
		for (numBlackStones nbsAB = 0; nbsAB < sa.NUM_STONES_PER_PLAYER; nbsAB++) {
		for (numWhiteStones nwsCD = 0; nwsCD < sa.NUM_STONES_PER_PLAYER; nwsCD++) {
		for (numBlackStones nbsCD = 0; nbsCD < sa.NUM_STONES_PER_PLAYER; nbsCD++) {
			for (unsigned int phase = 0; phase < 2; phase++) {
				numWhiteStones nws = nwsAB + nwsCD;
				numBlackStones nbs = nbsAB + nbsCD;
				if (nws > sa.NUM_STONES_PER_PLAYER) continue;
				if (nbs > sa.NUM_STONES_PER_PLAYER) continue;
				const layerId& layerNum = sa.layerIndex[phase][nws][nbs];
				const stateAddressing::layerStruct& layer = sa.layer[layerNum];
				EXPECT_EQ(sa.getLayerNumber(nws, nbs, phase == sa.LAYER_INDEX_SETTING_PHASE), layerNum);
				EXPECT_EQ(layer.amountWhiteStones, nws);
				EXPECT_EQ(layer.amountBlackStones, nbs);
				if (nwsCD + nwsAB != layer.amountWhiteStones)   				continue;
				if (nbsCD + nbsAB != layer.amountBlackStones)   				continue;
				if (nwsAB + nbsAB > sa.numSquaresGroupA + sa.numSquaresGroupB)  continue;
				if (nwsCD + nbsCD > sa.numSquaresGroupC + sa.numSquaresGroupD)  continue;
				{
					const subLayerId subLayerIndex = layer.subLayerIndexCD[nwsCD][nbsCD];
					const auto& subLayer = layer.subLayer[subLayerIndex];
					EXPECT_EQ(subLayer.numWhiteStonesGroupAB, nwsAB);
					EXPECT_EQ(subLayer.numBlackStonesGroupAB, nbsAB);
					EXPECT_EQ(subLayer.numWhiteStonesGroupCD, nwsCD);
					EXPECT_EQ(subLayer.numBlackStonesGroupCD, nbsCD);
				}
				groupIndex curGroupIndex = 0;
				for (subLayerId subLayerIndex=0; subLayerIndex<layer.numSubLayers; subLayerIndex++) {
					const auto& subLayer = layer.subLayer[subLayerIndex];
					EXPECT_EQ(layer.subLayerIndexCD[subLayer.numWhiteStonesGroupCD][subLayer.numBlackStonesGroupCD], subLayerIndex);
					EXPECT_EQ(subLayer.minIndex, curGroupIndex);
					// EXPECT_EQ(subLayer.maxIndex, curGroupIndex + sa.amountSituationsAB[nwsAB][nbsAB] * sa.amountSituationsCD[nwsCD][nbsCD] - 1);
					EXPECT_GE(subLayer.maxIndex, subLayer.minIndex);
					curGroupIndex = subLayer.maxIndex + 1;
				}
			}
		}}}}
	}
}

TEST_F(StateAddressingTest, internal_functions)
{
	stateAddressing sa(tmpFileDirectory);

	fieldStruct field;
	field.setSituation({
	  o,    o,    x,
		_,  _,  x,
		  o,_,o,
	  _,_,_,  o,x,x,
		  _,o,o,
		_,  _,  _,
	  x,    x,    x}, true, 0);

	// check applySymmetryTransfToField()
	{
		fieldStruct fieldTmp = field;
		fieldStruct fieldTmp2;
		fieldTmp2.setSituation({x,    x,    x,
								  _,  _,  _,
								    o,o,_,
								x,x,o,  _,_,_,
								    o,_,o,
								  x,  _,  _,
								x,    o,    o}, true, 0);
		EXPECT_EQ(sa.applySymmetryTransfToField(stateAddressing::SO_TURN_180, false, fieldTmp), true);
		EXPECT_EQ(fieldTmp, fieldTmp2);
	}

	// check countStonesInGroup()
	{
		numWhiteStones nwsAB, nwsCD;	// x
		numBlackStones nbsAB, nbsCD;	// o
		sa.countStonesInGroup(field, nwsAB, nbsAB, nwsCD, nbsCD);
		EXPECT_EQ(nwsAB, 2);
		EXPECT_EQ(nwsCD, 5);
		EXPECT_EQ(nbsAB, 0);
		EXPECT_EQ(nbsCD, 7);
	}

	// check countStonesInGroupCD()
	{
		numWhiteStones nwsCD;	// x
		numBlackStones nbsCD;	// o
		sa.countStonesInGroupCD(field, nwsCD, nbsCD);
		EXPECT_EQ(nwsCD, 5);
		EXPECT_EQ(nbsCD, 7);
	}

	// check if calcFieldBasedOnGroup() and calcGroupStateNumberBasedOnField() are inverse functions
	{
		fieldStruct::fieldArray fieldTmp;
		groupStateNumber returnedStateNumber = 0;
		for (groupStateNumber stateNumberAB = 0; stateNumberAB < sa.MAX_NUM_SITUATIONS_A * sa.MAX_NUM_SITUATIONS_B; stateNumberAB++) {
			sa.calcFieldBasedOnGroupAB(fieldTmp, stateNumberAB);
			sa.calcGroupStateNumberAB(fieldTmp, returnedStateNumber);
			EXPECT_EQ(stateNumberAB, returnedStateNumber);
		}
		for (groupStateNumber stateNumberCD = 0; stateNumberCD < sa.MAX_NUM_SITUATIONS_C * sa.MAX_NUM_SITUATIONS_D; stateNumberCD++) {
			sa.calcFieldBasedOnGroupCD(fieldTmp, stateNumberCD);
			sa.calcGroupStateNumberCD(fieldTmp, returnedStateNumber);
			EXPECT_EQ(stateNumberCD, returnedStateNumber);
		}
	}
}

TEST_F(StateAddressingTest, test_with_filled_cache_file)
{
	// locals
	stateAddressing sa(tmpFileDirectory);
	unsigned int 	layerNumber;
	unsigned int 	stateNumber;
	unsigned int	symOp;
	fieldStruct		fieldTmp;
	std::array<unsigned int, stateAddressing::NUM_SYM_OPERATIONS> stateNumbers;

	// test if unsigned int is sufficient for stateNumber addressing
	for (unsigned int curLayer = 0; curLayer < stateAddressing::NUM_LAYERS; curLayer++) {
		sa.getNumberOfKnotsInLayer(curLayer);
	}

	// basic tests
	EXPECT_EQ(sa.getLayer(199).amountBlackStones, 0);
	EXPECT_EQ(sa.getLayer(199).amountWhiteStones, 0);
	EXPECT_EQ(sa.getNumberOfKnotsInLayer(199), 18);											// 18 states only during the setting phase for an empty field
	EXPECT_EQ(sa.getLayerNumber(0, 0, true), 199);
	EXPECT_EQ(sa.getStateNumber(0, stateNumber, symOp, field), true);
	EXPECT_EQ(stateNumber, 0);
	EXPECT_EQ(symOp, 3);
	EXPECT_EQ(sa.getStateNumbersOfSymmetricStates(field, stateNumbers), true);
	for (const auto& stateNum : stateNumbers) {
		EXPECT_EQ(stateNum, 0);
	}
	EXPECT_EQ(sa.getFieldByStateNumber(0, 0, fieldTmp, x), false);
	EXPECT_FALSE(field.setSituation({}, false, 0));
	EXPECT_EQ(fieldTmp, field);
	EXPECT_EQ(sa.applySymmetryTransfToField(0, true, field), true);
	EXPECT_EQ(fieldTmp, field);
	
	// pick an arbitrary field in setting phase, and do some consistency checks
	EXPECT_TRUE(field.setSituation({o,    o,    x,
									  _,  _,  x,
									    o,_,o,
									_,_,_,  o,x,x,
									    _,o,o,
									  _,  _,  _,
									x,    x,    x}, true, 3));
	layerNumber = sa.getLayerNumber(field);													// Get the layer number from this arbitrary field and remember for later
	EXPECT_EQ(layerNumber, 112);															// 112 is the layer number for this field
	EXPECT_EQ(sa.getLayerNumber(7, 7, true), layerNumber);									// 7 black and 7 white stones are on the field
	EXPECT_EQ(sa.getStateNumber(layerNumber, stateNumber, symOp, field), true);				// Get the state number from the field
	EXPECT_EQ(stateNumber, 1315916871);														// 1315916871 is the state number for this field
	EXPECT_EQ(symOp, stateAddressing::SO_INV_RIGHT);										// SO_INV_RIGHT is the symmetry operation for this field
	EXPECT_EQ(sa.getFieldByStateNumber(112, 1315916871, fieldTmp, x), true);				// Get the field from this state number
	EXPECT_EQ(sa.applySymmetryTransfToField(symOp, true, fieldTmp), true);					// Apply the symmetry operation to the field
	EXPECT_EQ(fieldTmp, field);																// The field must be the same as the original field
	EXPECT_EQ(sa.getLayer(layerNumber).amountBlackStones, 7);								// x is player one with black color
	EXPECT_EQ(sa.getLayer(layerNumber).amountWhiteStones, 7);								// o is player two with white color
	EXPECT_EQ(sa.getStateNumbersOfSymmetricStates(field, stateNumbers), true);				// Get the state numbers of all symmetric states
	EXPECT_EQ(stateNumbers, (std::array<unsigned int, stateAddressing::NUM_SYM_OPERATIONS>{	// The state numbers must be the same as the original state number
		1315916871, 1315916871, 1315916871, 1315916871, 
		1315916871, 1315916871, 1315916871, 1315916871, 
		1315916871, 1315916871, 1315916871, 1315916871, 
		1315916871, 1315916871, 1315916871, 1315916871}));
	EXPECT_EQ(sa.applySymmetryTransfToField(stateAddressing::SO_TURN_180, false, field), true);
	EXPECT_TRUE(fieldTmp.setSituation({x,    x,    x,										// Do a symmetry operation to the field
										 _,  _,  _,
										   o,o,_,
									   x,x,o,  _,_,_,
										   o,_,o,
										 x,  _,  _,
									   x,    o,    o}, true, 3));
	EXPECT_EQ(fieldTmp, field);																// Expect the rotated field
	EXPECT_EQ(sa.getStateNumber(layerNumber, stateNumber, symOp, field), true);				// Get the state number from the rotated field
	EXPECT_EQ(stateNumber, 1315916871);														// The state number must be still the same as the original state number
	EXPECT_EQ(symOp, stateAddressing::SO_INV_LEFT);											// The symmetry operation must be as from above, but rotated by 180 degrees
}

TEST_F(StateAddressingTest, test_getStateNumber_setField_consistency)
{
	// locals
	stateAddressing sa(tmpFileDirectory);
	unsigned int 	layerNumber;
	unsigned int 	stateNumber, stateNumber2;
	unsigned int	symOp2;
	fieldStruct		fieldTmp;
	playerId 		curPlayer, oppPlayer;
	const unsigned int numRndStatesToTest = 10;
	unsigned int 	testCounter = 0;

	// test some random state number of each layer
	for (unsigned int layerNumber = 0; layerNumber < stateAddressing::NUM_LAYERS; layerNumber++) {
		if (!sa.getNumberOfKnotsInLayer(layerNumber)) continue;
		for (testCounter=0; testCounter<numRndStatesToTest; testCounter++) {
			if (testCounter > sa.getNumberOfKnotsInLayer(layerNumber)) break;
			stateNumber = rand() % sa.getNumberOfKnotsInLayer(layerNumber);
			
			// test each player
			for (curPlayer = playerId::playerOne; curPlayer <= playerId::playerTwo; curPlayer = static_cast<playerId>((int) curPlayer + 1)) {
				oppPlayer 		= (curPlayer == playerId::playerTwo) ? playerId::playerOne : playerId::playerTwo;
				stateNumber2 	= 0xdeadbeef;
				fieldTmp.reset(oppPlayer);
				if (!sa.getFieldByStateNumber(layerNumber, stateNumber, fieldTmp, curPlayer)) continue;
				if (!sa.getStateNumber(layerNumber, stateNumber2, symOp2, fieldTmp)) {
					std::cout << "Layer: " << layerNumber << ", State: " << stateNumber << std::endl;
					fieldTmp.print();
					GTEST_FAIL() << "Either getFieldByStateNumber() or getStateNumber() is faulty";
				}
				EXPECT_TRUE(sa.getStateNumber(layerNumber, stateNumber2, symOp2, fieldTmp));
				EXPECT_EQ(stateNumber, 	stateNumber2);
				EXPECT_EQ(symOp2, 		stateAddressing::SO_DO_NOTHING);
				EXPECT_EQ(curPlayer, 	fieldTmp.getCurPlayer().id);
			}
		}
	}
}

TEST_F(StateAddressingTest, totalNumMissingStones)
{
	// locals
	stateAddressing sa(tmpFileDirectory);
	fieldStruct 	fieldTmp;
	stateId 		stateNumber;
	symOperationId 	symOp;

	fieldTmp.reset(o);

	// check addTotalNumMissingStonesOffset()
	{
		{
			stateNumber 						= 0;
			fieldTmp.curPlayer.numStonesMissing	= 0;
			fieldTmp.oppPlayer.numStonesMissing	= 0;
			EXPECT_TRUE(sa.addTotalNumMissingStonesOffset(stateNumber, fieldTmp));
			EXPECT_EQ(stateNumber, 0);
		}
		{
			stateNumber 						= 1;
			fieldTmp.curPlayer.numStones		= 3;
			fieldTmp.oppPlayer.numStones		= 1;
			fieldTmp.curPlayer.numStonesMissing = 2;
			fieldTmp.oppPlayer.numStonesMissing = 0;
			EXPECT_TRUE(sa.addTotalNumMissingStonesOffset(stateNumber, fieldTmp));
			EXPECT_EQ(stateNumber, 1 * sa.getMaxTotalNumMissingStones(3, 1) + 2);
		}
	}

	// check getNumberOfKnotsInLayer()
	{
		stateAddressing::numWhiteStones nws 					= 7;
		stateAddressing::numBlackStones nbs 					= 5;
		stateAddressing::layerId 		layerNumSettingPhase 	= sa.getLayerNumber(nws, nbs, true);
		stateAddressing::layerId 		layerNumMovingPhase 	= sa.getLayerNumber(nws, nbs, false);
		EXPECT_EQ(sa.getNumberOfKnotsInLayer(layerNumSettingPhase), sa.getNumberOfKnotsInLayer(layerNumMovingPhase) * sa.getMaxTotalNumMissingStones(nws,nbs));
	}
}