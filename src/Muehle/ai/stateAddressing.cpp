/*********************************************************************\
	stateAddressing.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/muehle
\*********************************************************************/

#include "stateAddressing.h"
#include <cassert>
#ifdef _MSC_VER
	#include <limits>
	#undef max
#endif

using namespace std;

#pragma region init functions

//-----------------------------------------------------------------------------
// Name: stateAddressing()
// Desc: Initializes the state addressing. Thereby the precalculated variables are loaded from the file preCalculatedVars.dat.
//		 If the file does not exist, the precalculated variables are calculated and saved into the file.
// Args: directory - the directory where the preCalculatedVars.dat file is stored
//-----------------------------------------------------------------------------
stateAddressing::stateAddressing(std::wstring const& directory)
{
	// allocate memory
	resizeVector2D(amountSituationsCD, 			groupIndex{0}, 			NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1);
	resizeVector2D(amountSituationsAB, 			groupIndex{0}, 			NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1);
	resizeVector1D(groupIndexAB, 				groupIndex{0}, 			MAX_NUM_SITUATIONS_A * MAX_NUM_SITUATIONS_B);
	resizeVector1D(groupIndexCD, 				groupIndex{0}, 			MAX_NUM_SITUATIONS_C * MAX_NUM_SITUATIONS_D);
	resizeVector1D(symmetryOperationCD, 		symOperationId{0}, 		MAX_NUM_SITUATIONS_C * MAX_NUM_SITUATIONS_D);
	resizeVector2D(symmetryTransformationTable, 0u, 					NUM_SYM_OPERATIONS, fieldStruct::size);
	resizeVector3D(groupStateCD, 				groupStateNumber{0}, 	NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1, 1);
	resizeVector3D(groupStateAB, 				groupStateNumber{0}, 	NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1, 1);
	resizeVector1D(powerOfThree, 				0u, 					numSquaresGroupC + numSquaresGroupD);
	resizeVector2D(mOverN, 						0u, 					fieldStruct::size + 1, fieldStruct::size + 1);
	resizeVector1D(reverseSymOperation, 		symOperationId{0}, 		NUM_SYM_OPERATIONS);
	resizeVector2D(concSymOperation, 			symOperationId{0}, 		NUM_SYM_OPERATIONS, NUM_SYM_OPERATIONS);
	resizeVector3D(layerIndex, 					layerId{0}, 			2, NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1);
	resizeVector1D(layer, 						layerStruct{}, 			NUM_LAYERS);

	// locals
	cacheFile cf(directory, *this);

	// vars already stored in file?
	if (cf.readFromFile()) {
		return;
	// calculate vars and save into file
	} else {

		// calc mOverN
		init_mOverN();

		// power of three
		init_powerOfThree();

		// int symmetryOperationTable, reverseSymOperation
		init_symOperationMappings();
	    
		// init concSymOperation
		init_concSymOperation();

		// init amountSituationsAB, originalStateAB, indexAB
		init_group_AB();

		// init amountSituationsCD, originalStateCD, symmetryOperationCD and indexCD
		init_group_CD();

		// init layerIndex and layer for moving phase
		initLayerRegardingMovingPhase();

		// init layerIndex and layer for setting phase
		initLayerRegardingSettingPhase();

		// write cache to file
		cf.writeToFile();
	}
}

//-----------------------------------------------------------------------------
// Name: init_mOverN()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_mOverN()
{
	for (unsigned int m=0; m<=fieldStruct::size; m++) { for (unsigned int n=0; n<=fieldStruct::size; n++) { 
		mOverN[m][n] = (unsigned int) mOverN_Function(m, n);
	}}
}

//-----------------------------------------------------------------------------
// Name: init_powerOfThree()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_powerOfThree()
{
	powerOfThree[0] = 1;
	for (unsigned int i=1; i<numSquaresGroupC+numSquaresGroupD; i++) {
		powerOfThree[i] = 3 * powerOfThree[i-1];
	}
}

//-----------------------------------------------------------------------------
// Name: init_symOperationMappings()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_symOperationMappings()
{
	// locals
	fieldStruct::fieldPos i;

	// symmetry operation table
	for (i=0; i<fieldStruct::size; i++) {
		symmetryTransformationTable[SO_TURN_LEFT     ][i] = soTableTurnLeft   [i];
		symmetryTransformationTable[SO_TURN_180      ][i] = soTableTurn180    [i];
		symmetryTransformationTable[SO_TURN_RIGHT    ][i] = soTableTurnRight  [i];
		symmetryTransformationTable[SO_DO_NOTHING    ][i] = soTableDoNothing  [i];
		symmetryTransformationTable[SO_INVERT        ][i] = soTableInvert     [i];
		symmetryTransformationTable[SO_MIRROR_VERT   ][i] = soTableMirrorVert [i];
		symmetryTransformationTable[SO_MIRROR_HORI   ][i] = soTableMirrorHori [i];
		symmetryTransformationTable[SO_MIRROR_DIAG_1 ][i] = soTableMirrorDiag1[i];
		symmetryTransformationTable[SO_MIRROR_DIAG_2 ][i] = soTableMirrorDiag2[i];
		symmetryTransformationTable[SO_INV_LEFT      ][i] = soTableInvLeft    [i];
		symmetryTransformationTable[SO_INV_RIGHT     ][i] = soTableInvRight   [i];
		symmetryTransformationTable[SO_INV_180       ][i] = soTableInv180     [i];
		symmetryTransformationTable[SO_INV_MIR_VERT  ][i] = soTableInvMirHori [i];
		symmetryTransformationTable[SO_INV_MIR_HORI  ][i] = soTableInvMirVert [i];
		symmetryTransformationTable[SO_INV_MIR_DIAG_1][i] = soTableInvMirDiag1[i];
		symmetryTransformationTable[SO_INV_MIR_DIAG_2][i] = soTableInvMirDiag2[i];
	}

	// reverse symmetrie operation
	reverseSymOperation[SO_TURN_LEFT     ] = SO_TURN_RIGHT;
	reverseSymOperation[SO_TURN_180      ] = SO_TURN_180;
	reverseSymOperation[SO_TURN_RIGHT    ] = SO_TURN_LEFT;
	reverseSymOperation[SO_DO_NOTHING    ] = SO_DO_NOTHING;
	reverseSymOperation[SO_INVERT        ] = SO_INVERT;
	reverseSymOperation[SO_MIRROR_VERT   ] = SO_MIRROR_VERT;
	reverseSymOperation[SO_MIRROR_HORI   ] = SO_MIRROR_HORI;
	reverseSymOperation[SO_MIRROR_DIAG_1 ] = SO_MIRROR_DIAG_1;
	reverseSymOperation[SO_MIRROR_DIAG_2 ] = SO_MIRROR_DIAG_2;
	reverseSymOperation[SO_INV_LEFT      ] = SO_INV_RIGHT;
	reverseSymOperation[SO_INV_RIGHT     ] = SO_INV_LEFT;
	reverseSymOperation[SO_INV_180       ] = SO_INV_180;  
	reverseSymOperation[SO_INV_MIR_VERT  ] = SO_INV_MIR_VERT;
	reverseSymOperation[SO_INV_MIR_HORI  ] = SO_INV_MIR_HORI;
	reverseSymOperation[SO_INV_MIR_DIAG_1] = SO_INV_MIR_DIAG_1;
	reverseSymOperation[SO_INV_MIR_DIAG_2] = SO_INV_MIR_DIAG_2;  	
}

//-----------------------------------------------------------------------------
// Name: init_concSymOperation()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_concSymOperation()
{
	// locals
	fieldStruct::fieldPos   i;
	symOperationId 			symOpA, symOpB, symOpC;

	for (symOpA=0; symOpA<NUM_SYM_OPERATIONS; symOpA++) { 
		for (symOpB=0; symOpB<NUM_SYM_OPERATIONS; symOpB++) {

			// test each symmetry operation
			for (symOpC=0; symOpC<NUM_SYM_OPERATIONS; symOpC++) {

				// look if b(a(state)) == c(state)
				for (i=0; i<fieldStruct::size; i++) {
					if (symmetryTransformationTable[symOpC][i] != symmetryTransformationTable[symOpA][symmetryTransformationTable[symOpB][i]]) break;
				}   
				
				// match found?
				if (i == fieldStruct::size) {
					concSymOperation[symOpA][symOpB] = symOpC;
					break;
				}
			}

			// no match found
			assert((symOpC != NUM_SYM_OPERATIONS) && "ERROR IN SYMMETRY-OPERATIONS");
		}
	}
}

//-----------------------------------------------------------------------------
// Name: resizeOriginalState()
// Desc:
//-----------------------------------------------------------------------------
void stateAddressing::resizeGroupStateMappingArray(vector3D<unsigned int>& groupState, const vector2D<unsigned int>* pAmountSituations, unsigned int numSquaresInGroup) const
{
	// locals
	numWhiteStones 	nws;
	numBlackStones 	nbs;
	groupIndex 		amountSituations;

	for (nws=0; nws<=NUM_STONES_PER_PLAYER; nws++) { for (nbs=0; nbs<=NUM_STONES_PER_PLAYER; nbs++) {
		if (nws + nbs > numSquaresInGroup) continue;
		amountSituations = (pAmountSituations != nullptr) ? (*pAmountSituations)[nws][nbs] : mOverN[numSquaresInGroup][nws] * mOverN[numSquaresInGroup - nws][nbs];
		groupState[nws][nbs].resize(amountSituations);
	}}
}

//-----------------------------------------------------------------------------
// Name: init_group_AB()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_group_AB()
{
	// locals
	unsigned int					i;
	numWhiteStones 					nws;
	numBlackStones 					nbs;	
	groupStateNumber 				stateAB;
	fieldStruct::fieldArray 		myField;

	// reserve memory
	resizeGroupStateMappingArray(groupStateAB, nullptr, numSquaresGroupA + numSquaresGroupB);

	// mark all indexCD as not indexed
	groupIndexAB.assign(MAX_NUM_SITUATIONS_A*MAX_NUM_SITUATIONS_B, NOT_INDEXED);

	// iterate through each state within group A&B
	for (stateAB=0; stateAB<MAX_NUM_SITUATIONS_A*MAX_NUM_SITUATIONS_B; stateAB++) {

		// new state ?
		if (groupIndexAB[stateAB] != NOT_INDEXED) continue;

		// zero field
		myField.fill(playerId::squareIsFree);

		// make field
		calcFieldBasedOnGroupAB(myField, stateAB);

		// count black and white stones
		for (nws=0,i=0; i<fieldStruct::size; i++) if (myField[i] == fieldStruct::playerWhite) nws++; 
		for (nbs=0,i=0; i<fieldStruct::size; i++) if (myField[i] == fieldStruct::playerBlack) nbs++; 
		
		// condition
		if (nws + nbs > numSquaresGroupA + numSquaresGroupB) continue;

		// mark original state
		groupIndexAB[stateAB]						  = amountSituationsAB[nws][nbs];
		groupStateAB[nws][nbs][groupIndexAB[stateAB]] = stateAB;

		// state counter
		amountSituationsAB[nws][nbs]++;    
	}
}

//-----------------------------------------------------------------------------
// Name: init_group_CD()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::init_group_CD()
{
	// locals
	unsigned int					i;
	numWhiteStones 					nws;
	numBlackStones 					nbs;	
	groupStateNumber 				stateCD;
	groupStateNumber				symStateCD;
	fieldStruct::fieldArray 		myField;
	fieldStruct::fieldArray 		symField;
	vector3D<groupStateNumber>		originalStateCD_tmp;
	
	// reserve memory
	resizeVector3D(originalStateCD_tmp, groupStateNumber{0}, NUM_STONES_PER_PLAYER+1, NUM_STONES_PER_PLAYER+1, 1);
	resizeGroupStateMappingArray(originalStateCD_tmp, nullptr, numSquaresGroupC + numSquaresGroupD);

	// mark all indexCD as not indexed
	groupIndexCD.assign(MAX_NUM_SITUATIONS_C*MAX_NUM_SITUATIONS_D, NOT_INDEXED);

	// iterate through each state within group C&D
	for (stateCD=0; stateCD<MAX_NUM_SITUATIONS_C*MAX_NUM_SITUATIONS_D; stateCD++) {

		// new state ?
		if (groupIndexCD[stateCD] != NOT_INDEXED) continue;

		// zero field
		myField.fill(playerId::squareIsFree);

		// make field
		calcFieldBasedOnGroupCD(myField, stateCD);

		// count black and white stones
		for (nws=0,i=0; i<fieldStruct::size; i++) if (myField[i] == fieldStruct::playerWhite) nws++; 
		for (nbs=0,i=0; i<fieldStruct::size; i++) if (myField[i] == fieldStruct::playerBlack) nbs++; 
		
		// condition
		if (nws + nbs > numSquaresGroupC + numSquaresGroupD) continue;
		if (nws	  > NUM_STONES_PER_PLAYER) continue;
		if (nbs	  > NUM_STONES_PER_PLAYER) continue;

		// mark original state
		groupIndexCD        [stateCD]                        	= amountSituationsCD[nws][nbs];
		symmetryOperationCD [stateCD]                        	= SO_DO_NOTHING;
		originalStateCD_tmp [nws][nbs][groupIndexCD[stateCD]] 	= stateCD;

		// mark all symmetric states
		for (symOperationId symOp=0; symOp<NUM_SYM_OPERATIONS; symOp++) {
			
			applySymmetryTransfToField(symOp, false, myField, symField);

			calcGroupStateNumberCD(symField, symStateCD);


			if (stateCD != symStateCD) {
				groupIndexCD        [symStateCD] = groupIndexCD[stateCD];
				symmetryOperationCD [symStateCD] = reverseSymOperation[symOp];
			}
		}

		// state counter
		amountSituationsCD[nws][nbs]++;    
	}

	// copy from originalStateCD_tmp to originalStateCD
	resizeGroupStateMappingArray(groupStateCD, &amountSituationsCD, numSquaresGroupC + numSquaresGroupD);
	for (nws=0; nws<=NUM_STONES_PER_PLAYER; nws++) { for (nbs=0; nbs<=NUM_STONES_PER_PLAYER; nbs++) {
		if (nws + nbs > numSquaresGroupC + numSquaresGroupD) continue;
		for (i=0; i<amountSituationsCD[nws][nbs]; i++) groupStateCD[nws][nbs][i] = originalStateCD_tmp[nws][nbs][i];
	}}
}

//-----------------------------------------------------------------------------
// Name: initLayerRegardingMovingPhase()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::initLayerRegardingMovingPhase()
{
	// locals
	numWhiteStones 					nws;
	numBlackStones 					nbs;	
	unsigned int					totalNumStones;
	layerId 						layerNum;
	numWhiteStones 					wCD, wAB;	// number of white and black stones for group C&D and A&B
	numBlackStones 					bCD, bAB;

	// iterate through each layer
	for (totalNumStones=0, layerNum=0; totalNumStones<=2*NUM_STONES_PER_PLAYER; totalNumStones++) {

		// iterate through each number of white and black stones
		for (nws=0; nws<=totalNumStones; nws++) { for (nbs=0; nbs<=totalNumStones-nws; nbs++) { 

			// conditions
			if (nws>NUM_STONES_PER_PLAYER) continue;
			if (nbs>NUM_STONES_PER_PLAYER) continue;
			if (nws+nbs != totalNumStones) continue;

			// set layer properties
			layerIndex[LAYER_INDEX_MOVING_PHASE][nws][nbs]	= layerNum;
			layer[layerNum].amountWhiteStones	            = nws;
			layer[layerNum].amountBlackStones	            = nbs;
			layer[layerNum].numSubLayers                	= 0;

			// iterate through each number of white and black stones for group C&D
			subLayerId curSubLayerId 		= 0;
			groupIndex curGroupIndexOffset 	= 0;
			for (wCD=0; wCD<=layer[layerNum].amountWhiteStones; wCD++) { for (bCD=0; bCD<=layer[layerNum].amountBlackStones; bCD++) {

				// calc number of white and black stones for group A&B
				wAB = layer[layerNum].amountWhiteStones - wCD;
				bAB = layer[layerNum].amountBlackStones - bCD;

				// conditions
				if (wCD + wAB != layer[layerNum].amountWhiteStones)   continue;
				if (bCD + bAB != layer[layerNum].amountBlackStones)   continue;
				if (wAB + bAB > numSquaresGroupA + numSquaresGroupB)  continue;
				if (wCD + bCD > numSquaresGroupC + numSquaresGroupD)  continue;

				layer[layerNum].subLayer[curSubLayerId].minIndex           		= curGroupIndexOffset;
				layer[layerNum].subLayer[curSubLayerId].maxIndex           		= curGroupIndexOffset + amountSituationsAB[wAB][bAB] * amountSituationsCD[wCD][bCD] - 1;
				layer[layerNum].subLayer[curSubLayerId].numBlackStonesGroupAB  	= bAB;
				layer[layerNum].subLayer[curSubLayerId].numBlackStonesGroupCD  	= bCD;
				layer[layerNum].subLayer[curSubLayerId].numWhiteStonesGroupAB  	= wAB;
				layer[layerNum].subLayer[curSubLayerId].numWhiteStonesGroupCD  	= wCD;
				layer[layerNum].subLayerIndexAB[wAB][bAB]                      	= NOT_INDEXED;
				layer[layerNum].subLayerIndexCD[wCD][bCD]                      	= curSubLayerId;
				layer[layerNum].numSubLayers++;
				curGroupIndexOffset += amountSituationsAB[wAB][bAB] * amountSituationsCD[wCD][bCD];
				curSubLayerId++;
			}}

			// next layer
			layerNum++;
		}}
	}	
}

//-----------------------------------------------------------------------------
// Name: initLayerRegardingSettingPhase()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::initLayerRegardingSettingPhase()
{
	// locals
	numWhiteStones 					nws;				// number of white stones
	numBlackStones 					nbs;				// number of black stones
	unsigned int					totalNumStones;		// nws + nbs
	layerId 						layerNum;			// layer number
	numWhiteStones 					wCD, wAB;			// number of white stones for group C&D and A&B
	numBlackStones 					bCD, bAB;			// number of black stones for group C&D and A&B

	// iterate through each layer
	for (totalNumStones=0, layerNum=NUM_LAYERS-1; totalNumStones<=2*NUM_STONES_PER_PLAYER; totalNumStones++) {

		// iterate through each number of white and black stones
		for (nws=0; nws<=totalNumStones; nws++) { for (nbs=0; nbs<=totalNumStones-nws; nbs++) { 

			// conditions
			if (nws	>  NUM_STONES_PER_PLAYER)	continue;
			if (nbs	>  NUM_STONES_PER_PLAYER)	continue;
			if (nws+nbs != totalNumStones)		continue;

			// set layer properties
			layer[layerNum].amountWhiteStones	            = nws;
			layer[layerNum].amountBlackStones	            = nbs;
			layerIndex[LAYER_INDEX_SETTING_PHASE][nws][nbs]	= layerNum;
			layer[layerNum].numSubLayers                    = 0;
			
			// iterate through each number of white and black stones for group C&D
			subLayerId curSubLayerId 		= 0;
			groupIndex curGroupIndexOffset 	= 0;			
			for (wCD=0; wCD<=layer[layerNum].amountWhiteStones; wCD++) { for (bCD=0; bCD<=layer[layerNum].amountBlackStones; bCD++) {

				// calc number of white and black stones for group A&B
				wAB = layer[layerNum].amountWhiteStones - wCD;
				bAB = layer[layerNum].amountBlackStones - bCD;

				// conditions
				if (wCD + wAB != layer[layerNum].amountWhiteStones)   continue;
				if (bCD + bAB != layer[layerNum].amountBlackStones)   continue;
				if (wAB + bAB > numSquaresGroupA + numSquaresGroupB)  continue;
				if (wCD + bCD > numSquaresGroupC + numSquaresGroupD)  continue;

				layer[layerNum].subLayer[curSubLayerId].minIndex           		= curGroupIndexOffset;
				layer[layerNum].subLayer[curSubLayerId].maxIndex           		= curGroupIndexOffset + amountSituationsAB[wAB][bAB] * amountSituationsCD[wCD][bCD] - 1;
				layer[layerNum].subLayer[curSubLayerId].numBlackStonesGroupAB  	= bAB;
				layer[layerNum].subLayer[curSubLayerId].numBlackStonesGroupCD  	= bCD;
				layer[layerNum].subLayer[curSubLayerId].numWhiteStonesGroupAB  	= wAB;
				layer[layerNum].subLayer[curSubLayerId].numWhiteStonesGroupCD  	= wCD;
				layer[layerNum].subLayerIndexAB[wAB][bAB]                       = NOT_INDEXED;
				layer[layerNum].subLayerIndexCD[wCD][bCD]                       = curSubLayerId;
				layer[layerNum].numSubLayers++;
				curGroupIndexOffset 										   += amountSituationsAB[wAB][bAB] * amountSituationsCD[wCD][bCD];
				curSubLayerId++;				
			}}

			// next layer
			layerNum--;
		}}
	}
}

//-----------------------------------------------------------------------------
// Name: nOverN()
// Desc: Returns the number of possibilities to put n different stones in m holes
//-----------------------------------------------------------------------------
long long stateAddressing::mOverN_Function(unsigned int m, unsigned int n)
{
	// locals
	long long result	= 1;
	long long fakN		= 1;	
	unsigned int i;

	// invalid parameters ?
	if (n > m) return 0;

	// flip, since then the result value won't get so high
	if (n > m/2) n = m-n;

	// calc number of possibilities one can put n different stones in m holes
	for (i=m-n+1; i<=m; i++) result *= i;

	// calc number of possibilities one can sort n different stones
	for (i=    1; i<=n; i++) fakN *= i;

	// divide
	result /= fakN;

    return result;
}

//-----------------------------------------------------------------------------
// Name: calcFieldBasedOnGroup()
// Desc: Updates the corresponding field array based on the given state number within a group
//-----------------------------------------------------------------------------
inline void stateAddressing::calcFieldBasedOnGroup(fieldStruct::fieldArray& field, unsigned int numSquaresInGroup, groupStateNumber stateNumber, const fieldStruct::fieldPos* squareIndexGroup, unsigned int groupOrder, const vector1D<unsigned int>& powerOfThree)
{
	for (unsigned int j = 0; j < numSquaresInGroup; ++j) {
		field[squareIndexGroup[j]] = static_cast<playerId>((stateNumber / powerOfThree[groupOrder - j]) % 3);
	}
}

//-----------------------------------------------------------------------------
// Name: calcFieldBasedOnGroupAB()
// Desc:
//-----------------------------------------------------------------------------
void stateAddressing::calcFieldBasedOnGroupAB(fieldStruct::fieldArray &field, groupStateNumber stateAB) const
{
	calcFieldBasedOnGroup(field, numSquaresGroupA, stateAB, squareIndexGroupA, groupOrderA, powerOfThree);
	calcFieldBasedOnGroup(field, numSquaresGroupB, stateAB, squareIndexGroupB, groupOrderB, powerOfThree);
}

//-----------------------------------------------------------------------------
// Name: calcFieldBasedOnGroupCD()
// Desc:
//-----------------------------------------------------------------------------
void stateAddressing::calcFieldBasedOnGroupCD(fieldStruct::fieldArray &field, groupStateNumber stateAB) const
{
	calcFieldBasedOnGroup(field, numSquaresGroupC, stateAB, squareIndexGroupC, groupOrderC, powerOfThree);
	calcFieldBasedOnGroup(field, numSquaresGroupD, stateAB, squareIndexGroupD, groupOrderD, powerOfThree);
}

//-----------------------------------------------------------------------------
// Name: calcGroupStateNumberBasedOnField()
// Desc: Calculates the state number based on the field array within a group
//-----------------------------------------------------------------------------
inline void stateAddressing::calcGroupStateNumberBasedOnField(const fieldStruct::fieldArray& field, unsigned int numSquaresInGroup, groupStateNumber& stateNumber, const unsigned int* squareIndexGroup, unsigned int groupOrder, const vector1D<unsigned int>& powerOfThree)
{
	for (int j = 0; j < numSquaresInGroup; ++j) {
		stateNumber += static_cast<stateId>(field[squareIndexGroup[j]]) * powerOfThree[groupOrder - j];
	}
}

//-----------------------------------------------------------------------------
// Name: calcGroupStateNumberAB()
// Desc:
//-----------------------------------------------------------------------------
void stateAddressing::calcGroupStateNumberAB(const fieldStruct::fieldArray &field, groupStateNumber &stateNumberAB) const
{
	stateNumberAB = 0;
	calcGroupStateNumberBasedOnField(field, numSquaresGroupA, stateNumberAB, squareIndexGroupA, groupOrderA, powerOfThree);
	calcGroupStateNumberBasedOnField(field, numSquaresGroupB, stateNumberAB, squareIndexGroupB, groupOrderB, powerOfThree);
}

//-----------------------------------------------------------------------------
// Name: calcGroupStateNumberCD()
// Desc:
//-----------------------------------------------------------------------------
void stateAddressing::calcGroupStateNumberCD(const fieldStruct::fieldArray &field, groupStateNumber &stateNumberCD) const
{
	stateNumberCD = 0;
	calcGroupStateNumberBasedOnField(field, numSquaresGroupC, stateNumberCD, squareIndexGroupC, groupOrderC, powerOfThree);
	calcGroupStateNumberBasedOnField(field, numSquaresGroupD, stateNumberCD, squareIndexGroupD, groupOrderD, powerOfThree);
}

#pragma endregion

#pragma region general functions

//-----------------------------------------------------------------------------
// Name: applySymmetryTransfToField()
// Desc: Applies a symmetrie operation on a sourceField returning destField 
//-----------------------------------------------------------------------------
void stateAddressing::applySymmetryTransfToField(symOperationId symmetryOperationNumber, bool doInverseOperation, const fieldStruct::fieldArray& sourceField, fieldStruct::fieldArray& destField) const
{
	symmetryOperationNumber = doInverseOperation ? reverseSymOperation[symmetryOperationNumber] : symmetryOperationNumber;
	auto& symMap = symmetryTransformationTable[symmetryOperationNumber];
    for (fieldStruct::fieldPos i=0; i<fieldStruct::size; i++) {
        destField[i] = sourceField[symMap[i]];
    }
}

//-----------------------------------------------------------------------------
// Name: applySymmetryTransfToField()
// Desc: Applies a symmetrie operation on a sourceField returning destField 
//-----------------------------------------------------------------------------
void stateAddressing::applySymmetryTransfToField(symOperationId symmetryOperationNumber, bool doInverseOperation, const fieldStruct::millArray& sourceField, fieldStruct::millArray& destField) const
{
	symmetryOperationNumber = doInverseOperation ? reverseSymOperation[symmetryOperationNumber] : symmetryOperationNumber;
	auto& symMap = symmetryTransformationTable[symmetryOperationNumber];
    for (fieldStruct::fieldPos i=0; i<fieldStruct::size; i++) {
        destField[i] = sourceField[symMap[i]];
    }
}

//-----------------------------------------------------------------------------
// Name: applySymmetryTransfToField()
// Desc: Applies a symmetrie operation on a field 
//-----------------------------------------------------------------------------
bool stateAddressing::applySymmetryTransfToField(symOperationId symmetryOperationNumber, bool doInverseOperation, fieldStruct& field) const
{
	// checks
	if (symmetryOperationNumber >= NUM_SYM_OPERATIONS) return false;

	// apply symmetrie operation on field 
	fieldStruct::fieldArray tmpField = field.field;
	applySymmetryTransfToField(symmetryOperationNumber, doInverseOperation, tmpField, field.field); 

	// ... and mill counter if necessary
	fieldStruct::millArray tmpStonePartOfMill = field.stonePartOfMill;
	applySymmetryTransfToField(symmetryOperationNumber, doInverseOperation, tmpStonePartOfMill, field.stonePartOfMill);

	return true;
}

//-----------------------------------------------------------------------------
// Name: applySymmetryTransfToField()
// Desc: Applies a symmetrie operation on a field 
//-----------------------------------------------------------------------------
bool stateAddressing::applySymmetryTransfToField(symOperationId symmetryOperationNumber, bool doInverseOperation, fieldStruct::core& field) const
{
	// checks
	if (symmetryOperationNumber >= NUM_SYM_OPERATIONS) return false;

	// apply symmetrie operation on field 
	fieldStruct::fieldArray tmpField = field.field;
	applySymmetryTransfToField(symmetryOperationNumber, doInverseOperation, tmpField, field.field); 

	return true;
}

//-----------------------------------------------------------------------------
// Name: getLayerNumber()
// Desc: Returns the layer number for a given number of white and black stones 
//-----------------------------------------------------------------------------
stateAddressing::layerId stateAddressing::getLayerNumber(const fieldStruct::core& field) const
{
	return getLayerNumber(field.curPlayer.numStones, field.oppPlayer.numStones, field.settingPhase);
}

//-----------------------------------------------------------------------------
// Name: getLayerNumber()
// Desc: Returns the layer number for a given number of white and black stones 
// 		 It is assumed that the current player is always player white (2).
//-----------------------------------------------------------------------------
stateAddressing::layerId stateAddressing::getLayerNumber(unsigned int numStonesOfCurPlayer, unsigned int numStonesOfOppPlayer, bool isSettingPhase) const
{
	const unsigned int phaseIndex 	= isSettingPhase ? LAYER_INDEX_SETTING_PHASE : LAYER_INDEX_MOVING_PHASE;
	const numWhiteStones nws	 	= numStonesOfCurPlayer;
	const numBlackStones nbs		= numStonesOfOppPlayer;
	return layerIndex[phaseIndex][nws][nbs];
}

//-----------------------------------------------------------------------------
// Name: adaptFieldArrayToCurPlayer()
// Desc: Adapts the field array to the current player, so that current player is always player white (2)
//       This is necessary for the state addressing, since the state addressing assumes that the current player is always player white (2)
//-----------------------------------------------------------------------------
void stateAddressing::adaptFieldArrayToCurPlayer(const fieldStruct::fieldArray& srcField, fieldStruct::fieldArray& dstField, playerId curPlayer) const
{
	for(fieldStruct::fieldPos i=0; i<fieldStruct::size; i++) {
		if (srcField[i] == playerId::squareIsFree) {
			dstField[i] = playerId::squareIsFree;
		} else if (srcField[i] == curPlayer) {
			dstField[i] = fieldStruct::playerWhite;
		} else {
			dstField[i] = fieldStruct::playerBlack;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: countStonesInGroup()
// Desc: Counts the number of white and black stones in a group
//-----------------------------------------------------------------------------
void stateAddressing::countStonesInGroup(const fieldStruct::core& field, numWhiteStones& numWhiteStonesGroupAB, numBlackStones& numBlackStonesGroupAB, numWhiteStones& numWhiteStonesGroupCD, numBlackStones& numBlackStonesGroupCD) const
{
	numWhiteStonesGroupAB = 0;
	numBlackStonesGroupAB = 0;
	numWhiteStonesGroupCD = 0;
	numBlackStonesGroupCD = 0;

    for(fieldStruct::fieldPos i=0; i<fieldStruct::size; i++) {
		if (field.getStone(i) == field.getCurPlayer().id) {
			if (fieldPosIsOfGroup[i] == GROUP_A) numWhiteStonesGroupAB++;
			if (fieldPosIsOfGroup[i] == GROUP_B) numWhiteStonesGroupAB++;
            if (fieldPosIsOfGroup[i] == GROUP_C) numWhiteStonesGroupCD++;
            if (fieldPosIsOfGroup[i] == GROUP_D) numWhiteStonesGroupCD++;
        } else if (field.getStone(i) == field.getOppPlayer().id) {
			if (fieldPosIsOfGroup[i] == GROUP_A) numBlackStonesGroupAB++;
			if (fieldPosIsOfGroup[i] == GROUP_B) numBlackStonesGroupAB++;			
            if (fieldPosIsOfGroup[i] == GROUP_C) numBlackStonesGroupCD++;
            if (fieldPosIsOfGroup[i] == GROUP_D) numBlackStonesGroupCD++;
        }
	}	
}

//-----------------------------------------------------------------------------
// Name: getStateNumber()
// Desc: Returns the state number for a given field 
//-----------------------------------------------------------------------------
bool stateAddressing::getStateNumber(layerId layerNum, stateId& stateNumber, symOperationId& symOp, const fieldStruct::core& field) const
{
    // locals
	fieldStruct::fieldArray myField;
    fieldStruct::fieldArray symField;
	numWhiteStones			wCD;
	numBlackStones			bCD;
    groupStateNumber		stateAB;
	groupStateNumber		stateCD;

	// the state numbers assumes that the current player is always player white (2)
	// thus we have to convert the field to this assumption
	adaptFieldArrayToCurPlayer(field.field, myField, field.getCurPlayer().id);

	// count stones in each group
	countStonesInGroupCD(field, wCD, bCD);

    // calc stateCD
	calcGroupStateNumberCD(myField, stateCD);

    // apply symmetry operation on group A&B
    applySymmetryTransfToField(symmetryOperationCD[stateCD], false, myField, symField);

	// calc stateAB
	// Optimized: unroll loop and use pointer arithmetic for better cache locality
	stateAB = 0;
	const auto* sqA = squareIndexGroupA;
	const auto* sqB = squareIndexGroupB;
	const auto* p3 = powerOfThree.data();
	for (unsigned int i = 0; i < numSquaresGroupA; ++i) {
		stateAB += static_cast<stateId>(symField[squareIndexGroupA[i]]) * p3[groupOrderA - i];
	}
	for (unsigned int i = 0; i < numSquaresGroupB; ++i) {
		stateAB += static_cast<stateId>(symField[squareIndexGroupB[i]]) * p3[groupOrderB - i];
	}

    // calc index
	const unsigned int 	stateNumberWithInSubLayer 	= groupIndexAB[stateAB] * amountSituationsCD[wCD][bCD] + groupIndexCD[stateCD];
	const subLayerId 	subLayerIndexCD 			= layer[layerNum].subLayerIndexCD[wCD][bCD];
						stateNumber 				= (layer[layerNum].subLayer[subLayerIndexCD].minIndex + stateNumberWithInSubLayer);
    					symOp 						= symmetryOperationCD[stateCD];

	// consider offset based on totalNumMissingStones
	if (isSettingPhase(layerNum)) {
		if (!addTotalNumMissingStonesOffset(stateNumber, field)) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: countStonesInGroupCD()
// Desc: Counts the number of stones in group C and D
//-----------------------------------------------------------------------------
void stateAddressing::countStonesInGroupCD(const fieldStruct::core& field, numWhiteStones& wCD, numBlackStones& bCD) const
{
	// locals
	const playerId* fieldPtr 	= field.field.data();
	playerId		curPlayerId = field.getCurPlayer().id;
	playerId		oppPlayerId = field.getOppPlayer().id;	
	
	// reset
	wCD = 0;
	bCD = 0;

	// count stones in each group
	// Use pointers and avoid repeated lookups for better cache locality and speed
	const auto* groupPtr = fieldPosIsOfGroup;
	for (fieldStruct::fieldPos i = 0; i < fieldStruct::size; ++i) {
		const auto g = groupPtr[i];
		if (g == GROUP_C || g == GROUP_D) {
			const auto f = fieldPtr[i];
			if (f == curPlayerId) {
				++wCD;
			} else if (f == oppPlayerId) {
				++bCD;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: addTotalNumMissingStonesOffset()
// Desc: Adds the statenumber offset based on the total number of missing stones
//-----------------------------------------------------------------------------
bool stateAddressing::addTotalNumMissingStonesOffset(stateId& stateNumber, const fieldStruct::core& field) const
{
	// locals
	unsigned int nsm_curPlayer 			= field.getCurPlayer().numStonesMissing;
	unsigned int nsm_oppPlayer 			= field.getOppPlayer().numStonesMissing;
	unsigned int totalNumMissingStones 	= nsm_curPlayer + nsm_oppPlayer;

	// at maximum 2 stones can be removed from closed mills in total
	if (nsm_curPlayer > fieldStruct::numStonesPerPlayer || nsm_oppPlayer > fieldStruct::numStonesPerPlayer) {
		return false;
	}

	// add offset
	stateNumber = stateNumber * getMaxTotalNumMissingStones(field.getCurPlayer().numStones, field.getOppPlayer().numStones) + totalNumMissingStones;

	return true;
}

//-----------------------------------------------------------------------------
// Name: getMaxTotalNumMissingStones()
// Desc: Returns the maximum total number of missing stones for a given state number and layer.
//-----------------------------------------------------------------------------
unsigned int stateAddressing::getMaxTotalNumMissingStones(numWhiteStones amountWhiteStones, numBlackStones amountBlackStones)
{
	if (amountWhiteStones > fieldStruct::numStonesPerPlayer) return 0;
	if (amountBlackStones > fieldStruct::numStonesPerPlayer) return 0;
	return 2 * fieldStruct::numStonesPerPlayer - amountWhiteStones - amountBlackStones;
}

//-----------------------------------------------------------------------------
// Name: isSettingPhase()
/// Desc: Checks if the given layer is in the setting phase
//-----------------------------------------------------------------------------
bool stateAddressing::isSettingPhase(layerId layerNum) const
{
    return layerNum >= NUM_LAYERS / 2;
}

//-----------------------------------------------------------------------------
// Name: getFieldByStateNumber()
// Desc: Returns the field for a given state number and layer. Thereby the current player can be chosen.
//-----------------------------------------------------------------------------
bool stateAddressing::getFieldByStateNumber(layerId layerNum, stateId stateNumber, fieldStruct& field, playerId curPlayer) const
{
	// locals
	const bool 				settingPhase 				= isSettingPhase(layerNum);
	const layerStruct& 		curLayer 					= layer[layerNum];
	const unsigned int 		totalNumMissingStones		= getTotalNumMissingStones(stateNumber, settingPhase, curLayer.amountWhiteStones, curLayer.amountBlackStones);
    unsigned int 			stateNumberWithInSubLayer;
    groupIndex 				indexWithInGroupAB;
    groupIndex 				indexWithInGroupCD;
    groupStateNumber		stateAB, stateCD;
	subLayerId 				subLayerIndexCD;
	numWhiteStones			wAB, wCD;
	numBlackStones			bAB, bCD;
	fieldStruct::fieldArray myField;
    fieldStruct::fieldArray symField;

    // get wCD, bCD, wAB, bAB
	curLayer.getNumGroupStonesByStateNumber(stateNumber, settingPhase, wAB, bAB, wCD, bCD);

    // get index within groups
	subLayerIndexCD 		  = curLayer.subLayerIndexCD[wCD][bCD];
    stateNumberWithInSubLayer = curLayer.getStateNumberWithInSubLayer(stateNumber, settingPhase) - curLayer.subLayer[subLayerIndexCD].minIndex;
    indexWithInGroupAB        = stateNumberWithInSubLayer / amountSituationsCD[wCD][bCD];
    indexWithInGroupCD        = stateNumberWithInSubLayer % amountSituationsCD[wCD][bCD];

    // get state within groups
    stateCD = groupStateCD[wCD][bCD][indexWithInGroupCD];
    stateAB = groupStateAB[wAB][bAB][indexWithInGroupAB];

	// set myField from stateAB
	calcFieldBasedOnGroupAB(myField, stateAB);

	// apply symmetry operation on group A&B
    applySymmetryTransfToField(symmetryOperationCD[stateCD], true, myField, symField);

	// set myField from stateCD
	calcFieldBasedOnGroupCD(symField, stateCD);

	// the state numbers assumes that the current player is always player white
	// thus we have to convert this assumption to the requested player
	adaptFieldArrayToCurPlayer(symField, myField, curPlayer);

	// set field
	field.reset(curPlayer);
	return field.setSituation(myField, settingPhase, totalNumMissingStones);
}

//-----------------------------------------------------------------------------
// Name: layerStruct::getStateNumberWithInSubLayer()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int stateAddressing::layerStruct::getStateNumberWithInSubLayer(stateId stateNumber, bool settingPhase) const
{
	return settingPhase ? stateNumber / stateAddressing::getMaxTotalNumMissingStones(amountWhiteStones, amountBlackStones) : stateNumber;
}

//-----------------------------------------------------------------------------
// Name: getTotalNumMissingStones()
// Desc: Returns the total number of missing stones for a given state
//-----------------------------------------------------------------------------
unsigned int stateAddressing::getTotalNumMissingStones(stateId stateNumber, bool settingPhase, numWhiteStones amountWhiteStones, numBlackStones amountBlackStones) const
{
	return settingPhase ? stateNumber % getMaxTotalNumMissingStones(amountWhiteStones, amountBlackStones) : 0;
}

//-----------------------------------------------------------------------------
// Name: resizeOriginalState()
// Desc: 
//-----------------------------------------------------------------------------
void stateAddressing::layerStruct::getNumGroupStonesByStateNumber(stateId stateNumber, bool settingPhase, numWhiteStones &numWhiteStonesGroupAB, numBlackStones &numBlackStonesGroupAB, numWhiteStones &numWhiteStonesGroupCD, numBlackStones &numBlackStonesGroupCD) const
{
	unsigned int stateNumberWithInSubLayer = layerStruct::getStateNumberWithInSubLayer(stateNumber, settingPhase);
    for (subLayerId subLayerIndexCD=0; subLayerIndexCD<=numSubLayers; subLayerIndexCD++) {
        if (subLayer[subLayerIndexCD].minIndex <= stateNumberWithInSubLayer
         && subLayer[subLayerIndexCD].maxIndex >= stateNumberWithInSubLayer) {
            numWhiteStonesGroupCD = subLayer[subLayerIndexCD].numWhiteStonesGroupCD;
            numBlackStonesGroupCD = subLayer[subLayerIndexCD].numBlackStonesGroupCD;
            numWhiteStonesGroupAB = subLayer[subLayerIndexCD].numWhiteStonesGroupAB;
            numBlackStonesGroupAB = subLayer[subLayerIndexCD].numBlackStonesGroupAB;
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: isSymOperationInvariant()
// Desc: Returns true if a given symmetry operation would not change the field
//-----------------------------------------------------------------------------
bool stateAddressing::isSymOperationInvariant(symOperationId symmetryOperation, const fieldStruct::core& field) const
{
	if (symmetryOperation >= NUM_SYM_OPERATIONS) return false;
	if (symmetryOperation == stateAddressing::SO_DO_NOTHING) return true;
	const auto* theField = field.field.data();
	const auto* symTransTable = symmetryTransformationTable[symmetryOperation].data();
	// Unroll loop for better speed, avoid repeated lookups
	for (unsigned int i = 0; i < 8; ++i) {
		const auto c = squareIndexGroupC[i];
		const auto d = squareIndexGroupD[i];
		if (theField[c] != theField[symTransTable[c]]) return false;
		if (theField[d] != theField[symTransTable[d]]) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: getLayer()
// Desc: Returns the layer containing further information for a given number
//-----------------------------------------------------------------------------
const stateAddressing::layerStruct &stateAddressing::getLayer(layerId layerNum) const
{
	return layer[layerNum];
}

//-----------------------------------------------------------------------------
// Name: getNumberOfKnotsInLayer()
// Desc: Returns the number of knots in a given layer
//-----------------------------------------------------------------------------
unsigned int stateAddressing::getNumberOfKnotsInLayer(layerId layerNum) const
{
	// checks
	if (layerNum >= stateAddressing::NUM_LAYERS) return 0;

    // locals
    unsigned int numberOfKnots = layer[layerNum].subLayer[layer[layerNum].numSubLayers - 1].maxIndex + 1;

	// during setting phase removal of stones from closed mills lead to different states, which must be distuinguished
	if (isSettingPhase(layerNum)) {

		// since the white player always starts, there cannot be 9 white stones on the board, during setting phase
		if (layer[layerNum].amountWhiteStones >= fieldStruct::numStonesPerPlayer) {
			return 0;
		}

		// consider offset based on totalNumMissingStones
		// use uint64_t to avoid overflow		
		uint64_t knots64 = static_cast<uint64_t>(numberOfKnots) * getMaxTotalNumMissingStones(layer[layerNum].amountWhiteStones, layer[layerNum].amountBlackStones);
		if (knots64 > std::numeric_limits<unsigned int>::max()) {
			cout << "Error: Number of knots " << knots64 << " exceeds unsigned int range for layer " << layerNum << endl;
			assert(false && "Number of knots exceeds unsigned int range");
		}
		numberOfKnots = static_cast<unsigned int>(knots64);
	}

	// during moving phase, we have to check if the layer is reachable
	// return zero if layer is not reachable
	if (((layer[layerNum].amountBlackStones  < 2 || layer[layerNum].amountWhiteStones  < 2) && layerNum  < 100)
	||   (layer[layerNum].amountBlackStones == 2 && layer[layerNum].amountWhiteStones == 2  && layerNum  < 100)
    ||																				  	      (layerNum == 100))
		return 0;

    // another way
    return numberOfKnots;
}

//-----------------------------------------------------------------------------
// Name: getSymmetricStateNumbers()
// Desc: Returns the state numbers of all symmetric states for a given field
//-----------------------------------------------------------------------------
bool stateAddressing::getStateNumbersOfSymmetricStates(const fieldStruct::core& field, std::array<stateId, NUM_SYM_OPERATIONS>& stateNumbers) const
{
	// save current field
	symOperationId 				symmetryOperation;
	symOperationId 				symOpApplied;
	fieldStruct::core 			symField;
	layerId		 				layerNumber = getLayerNumber(field);
	stateId 					stateNumber;
	
	// get state number of current field
	if (!getStateNumber(layerNumber, stateNumber, symmetryOperation, field)) {
		return false;
	}
	symField = field;

	// add all symmetric states
	for (symmetryOperation=0; symmetryOperation<stateAddressing::NUM_SYM_OPERATIONS; symmetryOperation++) {

		// set state number to the state without any sym operation
		stateNumbers[symmetryOperation] = stateNumber;

		// TODO: check if this is really correct. Shouldn't it be rather:
		// if (symmetryOperation != SO_DO_NOTHING && isSymOperationInvariant(symmetryOperation, field)) {
		// 	continue;
		// }
		// only add if sym operation is invariant
		if (!isSymOperationInvariant(symmetryOperation, field)) {
			continue;
		}

		// appy symmetry operation
		applySymmetryTransfToField(symmetryOperation, false, field.field, symField.field);

		// store state number
		if (!getStateNumber(layerNumber, stateNumbers[symmetryOperation], symOpApplied, symField)) {
			return false;
		}
	}
	return true;
}

#pragma endregion

#pragma region cacheFile

//-----------------------------------------------------------------------------
// Name: cacheFile()
// Desc: Constructor
//-----------------------------------------------------------------------------
stateAddressing::cacheFile::cacheFile(std::wstring const &directory, stateAddressing &sa) :
	sa(sa)
{
	// locals
	wstringstream					ssPreCalcVarsFilePath;

	// set file path
	if (directory.size()) {
		if (!filesystem::exists(directory)) {	
			filesystem::create_directory(directory);
		}
		ssPreCalcVarsFilePath << directory << "\\"; 
	}
	ssPreCalcVarsFilePath << "preCalculatedVars.dat";
	filePath = ssPreCalcVarsFilePath.str();
	
	// Open File, which contains the precalculated vars
	hFile = CreateFile(ssPreCalcVarsFilePath.str().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

//-----------------------------------------------------------------------------
// Name: ~cacheFile()
// Desc:
//-----------------------------------------------------------------------------
stateAddressing::cacheFile::~cacheFile()
{
	// Close File
	if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
		CloseHandle(hFile);
	}
}

//-----------------------------------------------------------------------------
// Name: readFromFile()
// Desc: Reads the precalculated variables from the file preCalculatedVars.dat
//-----------------------------------------------------------------------------
bool stateAddressing::cacheFile::readFromFile()
{
	// locals
	DWORD			dwBytesRead		= 0;

	// check if file is valid
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
		return false;
	}

	// Read from file
	ReadFile(hFile, &header, sizeof (header), &dwBytesRead, NULL);

	// check if file is valid
	if (header.sizeInBytes != sizeof(header)) {
		return false;
	}

	readVector(hFile, sa.layer);
	readVector(hFile, sa.layerIndex);
	readVector(hFile, sa.amountSituationsAB);
	readVector(hFile, sa.amountSituationsCD);
	readVector(hFile, sa.groupIndexAB);
	readVector(hFile, sa.groupIndexCD);
	readVector(hFile, sa.symmetryOperationCD);
	readVector(hFile, sa.powerOfThree);
	readVector(hFile, sa.symmetryTransformationTable);
	readVector(hFile, sa.reverseSymOperation);
	readVector(hFile, sa.concSymOperation);
	readVector(hFile, sa.mOverN);
	sa.resizeGroupStateMappingArray(sa.groupStateAB, nullptr, 				    numSquaresGroupA + numSquaresGroupB);
	sa.resizeGroupStateMappingArray(sa.groupStateCD, &sa.amountSituationsCD, 	numSquaresGroupC + numSquaresGroupD);
	readVector(hFile, sa.groupStateAB);
	readVector(hFile, sa.groupStateCD);
	return true;
}

//-----------------------------------------------------------------------------
// Name: writeToFile()
// Desc: Writes the precalculated variables to the file preCalculatedVars.dat
//-----------------------------------------------------------------------------
bool stateAddressing::cacheFile::writeToFile()
{
	// locals
	DWORD 			dwBytesWritten	= 0;

	// check if file is valid
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
		return false;
	}

	// write vars into file
	header.sizeInBytes = sizeof(header);

	WriteFile(hFile, &header, header.sizeInBytes, &dwBytesWritten, NULL);
	writeVector(hFile, sa.layer);
	writeVector(hFile, sa.layerIndex);
	writeVector(hFile, sa.amountSituationsAB);
	writeVector(hFile, sa.amountSituationsCD);
	writeVector(hFile, sa.groupIndexAB);
	writeVector(hFile, sa.groupIndexCD);
	writeVector(hFile, sa.symmetryOperationCD);
	writeVector(hFile, sa.powerOfThree);
	writeVector(hFile, sa.symmetryTransformationTable);
	writeVector(hFile, sa.reverseSymOperation);
	writeVector(hFile, sa.concSymOperation);
	writeVector(hFile, sa.mOverN);
	writeVector(hFile, sa.groupStateAB);
	writeVector(hFile, sa.groupStateCD);
	return true;
}

#pragma endregion
