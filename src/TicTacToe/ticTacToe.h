/*********************************************************************\
	ticTacToe.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#ifndef TICTACTOE_H
#define TICTACTOE_H

#include "miniMax/src/miniMax.h"

// std
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

// current player  = x
// opponent player = o

// field positions
// 0|1|2
// 3|4|5
// 6|7|8

namespace gameId
{
	static const char				egoPlayerId						=  2;	// must be two
	static const char				oppPlayerId						=  1;	// must be one
}

class ticTacToe : public miniMax::gameInterface
{

public:
	class symmetryVars;

	using uint_1d = vector<unsigned int>;
	using uint_2d = vector<vector<unsigned int>>;

	// used to address the states in the database. there are two types of addressing.
	class stateAddressing
	{
	public:
		virtual unsigned int		getNumberOfKnotsInLayer			(unsigned int layerNum) = 0;
		virtual uint_1d 			getPartnerLayers				(unsigned int layerNum) = 0;
		virtual void  				getSuccLayers					(unsigned int layerNum, vector<unsigned int>& succLayers) = 0;
		virtual void 				getField						(unsigned int layerNum, unsigned int stateNumber, uint_1d &field) = 0;
		virtual unsigned int        getStateNumber					(unsigned int layerNum, const uint_1d& field) = 0;
		virtual unsigned int        getLayerNumber					(const uint_1d& field) = 0;
	};
		
	// --- type A --------------------------------------------------------------------------------
	// The first stone of the current player (x) searching from top left to bottom right is relevant for the layer number.
	// The first stones of the opponent player (o) are not relevant for the layer number.
	// # can be o,-
	// ? can be x,o,-
	// layer 0	 layer 1	 layer 2	 layer 3	...  layer 9
	// x|?|?	 #|x|?		 #|#|x		 #|#|#			 #|#|#
	// ?|?|?	 ?|?|?		 ?|?|?		 x|?|?			 #|#|#
	// ?|?|?	 ?|?|? 		 ?|?|?		 ?|?|?			 #|#|#
	// -------------------------------------------------------------------------------------------	
	class stateAddressingTypeA : public stateAddressing
	{
	public:
		static constexpr unsigned int	numLayers						= 10;	// number of layers
		static constexpr unsigned int 	emptyField_layer				= 9;	// layer number of the empty field
		static constexpr unsigned int 	emptyField_state				= 0;	// state number of the empty field

		unsigned int				getNumberOfKnotsInLayer			(unsigned int layerNum) override;
		uint_1d 					getPartnerLayers				(unsigned int layerNum) override;
		void  						getSuccLayers					(unsigned int layerNum, vector<unsigned int>& succLayers) override;
		void 						getField						(unsigned int layerNum, unsigned int stateNumber, uint_1d &field) override;
		unsigned int        		getStateNumber					(unsigned int layerNum, const uint_1d& field) override;
		unsigned int        		getLayerNumber					(const uint_1d& field) override;
	};
	
	// --- type B --------------------------------------------------------------------------------
	// The number of stones corresponds to the layer number.
	// The state number is calculated by the following formula:
	// stateIndex  = field[0] * 3^0 + field[1] * 3^1 + field[2] * 3^2 + field[3] * 3^3 + field[4] * 3^4
	//             + field[5] * 3^5 + field[6] * 3^6 + field[7] * 3^7 + field[8] * 3^8
	// stateNumber = mappingStateIndexToNumber[layerNumber][stateIndex]
	// -------------------------------------------------------------------------------------------	
	class stateAddressingTypeB : public stateAddressing
	{
	public:
		static constexpr unsigned int	numLayers						= 10;	// number of layers
		static constexpr unsigned int 	emptyField_layer				= 9;	// layer number of the empty field
		static constexpr unsigned int 	emptyField_state				= 0;	// state number of the empty field
		static constexpr unsigned int 	maxStateIndex 					= 3*3*3*3*3*3*3*3*3;	// maximum state index (3^9)

		unsigned int				getNumberOfKnotsInLayer			(unsigned int layerNum) override;
		uint_1d 					getPartnerLayers				(unsigned int layerNum) override;
		void  						getSuccLayers					(unsigned int layerNum, vector<unsigned int>& succLayers) override;
		void 						getField						(unsigned int layerNum, unsigned int stateNumber, uint_1d &field) override;
		unsigned int        		getStateNumber					(unsigned int layerNum, const uint_1d& field) override;
		unsigned int        		getLayerNumber					(const uint_1d& field) override;
		
									stateAddressingTypeB			();
		void 						getFieldFromStateIndex			(unsigned int stateIdex, uint_1d &field);
		unsigned int        		getStateIndex					(const uint_1d& field);

		uint_2d					    mappingStateIndexToNumber;		// [layerNumber][stateIndex]		- mapping of state index to state number
		uint_2d					    mappingStateNumberToIndex;		// [layerNumber][stateNumber]		- mapping of state number to state index
		uint_1d						numberOfKnotsInLayer;			// [layerNumber]					- number of knots in the layer
	};

	class gameState
	{
	public:
		
		// constants
		static const char			squareChar[3];												// characters for the field to print
		static const char			squareIsFree					= 0;						// must be zero
		static const unsigned char	size							= 9;						// field size
		static const char			curPlayerId						= gameId::egoPlayerId;		// the perspective of the game is always from the current ego player. on each move the field is inverted.

		// variables
		uint_1d						field							= {0,0,0,0,0,0,0,0,0};		// array with the stones

        // functions
        void						invert							();
		unsigned int 				getNumStonesOnField				();	
		void						print							(unsigned int indentSpaces);
		bool						hasPlayerWon					(char playerid);
		static bool					areValuesEqual					(char a, char b, char c, char d);
        bool 						isStonePartOfWinningAllLines	(unsigned int pos, char player);
		static unsigned int 		countStonesOnField				(const uint_1d& field, char playerid);
	};

	struct symmetryVars
	{
		bool						considerSymmetry				= true;
		const unsigned int			numSymOperations				= 8;
		enum						symOp							{ NONE=0, VERTICAL=1, HORIZONTAL=2, LEFT_DIAG=3, RIGHT_DIAG=4, LEFT=5, HALF_TURN=6, RIGHT=7};
		uint_2d						symOpMap;						// [symOp][posId]							- Symmetry operation map for applying symmetry operations on the field
		uint_2d						symOpWithSymmetry;				// [layerNumber][stateNumberNoSymmetry]		- Symmetry operation id
		uint_2d						stateNumWithSymmetry;			// [layerNumber][stateNumberNoSymmetry]		- State number with symmetry
		uint_2d						stateNumNoSymmetry;				// [layerNumber][stateNumberWithSymmetry]	- State number without symmetry - reverse mapping of stateNumWithSymmetry
		uint_1d						numStatesWithSymmetry;			// [layerNumber]							- Total number of states when symmetry is considered

									symmetryVars					(unsigned int numLayers, stateAddressing& sa);
		void 						applySymOp						(uint_1d &field, bool doInverseOperation, unsigned int symmetryOperationNumber);
    };

    class threadVars
	{
	public:
		gameState					curState;						// current game state for each thread
		uint_1d						moveHistory;					// history of the moves
	};

	// constructor
									ticTacToe						(stateAddressing& sa);

	// for unit testing
	bool							hasAnyBodyWon					(unsigned int threadNo = 0);
	bool							setStone						(unsigned int pos, unsigned int threadNo = 0);
	void							letComputerSetStone				();

	// helper functions
	static int						ipow							(int base, int exp);
	void							getLayerAndStateNumber			(gameState& gs, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp, const symmetryVars& symVars);

	// init
	void							prepareCalculation				()																													override;	

	// getter
    bool							shallRetroAnalysisBeUsed    	(unsigned int layerNum)																								override;
	void							getPossibilities				(unsigned int threadNo, vector<unsigned int>& possibilityIds)														override;	
	unsigned int					getMaxNumPossibilities			()																													override;
	unsigned int					getNumberOfLayers				()																													override;
	unsigned int 					getMaxNumPlies					()																													override;
	unsigned int					getNumberOfKnotsInLayer			(unsigned int layerNum)																								override;
    void							getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)															override;
	uint_1d							getPartnerLayers				(unsigned int layerNum)																								override;
	void							getValueOfSituation				(unsigned int threadNo, float& floatValue, miniMax::twoBit& shortValue)												override;	
	void							getLayerAndStateNumber			(unsigned int threadNo, unsigned int &layerNum, unsigned int &stateNumber, unsigned int&symOp)						override;	
	unsigned int					getLayerNumber					(unsigned int threadNo)																								override;
	void							getSymStateNumWithDuplicates	(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)												override;
    void							getPredecessors             	(unsigned int threadNo, vector<miniMax::retroAnalysis::predVars>& predVars)											override;
	bool							isStateIntegrityOk				(unsigned int threadNo)																								override;
	bool							lostIfUnableToMove				(unsigned int threadNo)																								override;

	// setter
	void							applySymOp						(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)	override;
	bool							setSituation					(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)											override;
	void							move							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)						override;
	void							undo							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)						override;
	
	// output
	void							printField						(unsigned int threadNo, miniMax::twoBit value, unsigned int indentSpaces)											override;
	void							printMoveInformation			(unsigned int threadNo, unsigned int idPossibility)																	override;
	wstring							getOutputInformation			(unsigned int layerNum)																								override;

	// variables
	miniMax::miniMax				mm								{(miniMax::gameInterface*) this, 19};
	bool 							useRetroAnalysis				= false;
	vector<threadVars>				tv;								// thread specific variables
	symmetryVars					symVars;						// symmetry variables
	stateAddressing& 				sa;								// state addressing
};

#endif
