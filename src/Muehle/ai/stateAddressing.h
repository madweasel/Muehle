/*********************************************************************\
	stateAddressing.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/muehle
\*********************************************************************/
#ifndef STATE_ADDRESSING_H
#define STATE_ADDRESSING_H

#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <limits>
// win api
#include <windows.h>
#include <Shlwapi.h>

#include "../fieldStruct.h"

/***************************************************************
All field states are divided into layers. Each layer number is determined by the number of white and black stones on the field.
Each square of the field belongs to one of the four groups (A,B,C,D).
	  C     D     C
	    A   B   A
	      C D C
	  D B D   D B D
		  C D C
	    A   B  A
	  C     D    C
Each layer is divided into sublayers. The sublayer number is determined by the number of white and black stones in group C and D.
Each state within each group (AB, CD) can be represented by a number (of type groupStateNumber) between 0 and 3^numSquaresGroup-1. The value is calculated by the sum of 3^i * field[i].
When symmetry and the amount white/black stones are considered, the number of states is reduced and can be indexed by the type groupIndex.
****************************************************************/

class stateAddressing
{
friend class StateAddressingTest_internal_variables_Test;
friend class StateAddressingTest_internal_functions_Test;
friend class StateAddressingTest_totalNumMissingStones_Test;

public:
	using groupStateNumber 	= unsigned int;		// number of a state within a group (without considering symmetry and the amount of white/black stones)
	using groupIndex 		= unsigned int;		// number of a state within a group (        considering symmetry and the amount of white/black stones)
	using numWhiteStones 	= unsigned int;		// number of white stones
	using numBlackStones 	= unsigned int;		// number of black stones
	using symOperationId 	= unsigned int;		// number of a symmetry operation
	using subLayerId 		= unsigned int;		// number of a sublayer within a layer
	using layerId  			= unsigned int;		// number of a layer
	using stateId 			= unsigned int;		// number of a state within a layer

	// The number of layers is calculated as follows:
	// - 10 x 10 since each color can range from 0 to 9 stones
	// - x2 since there is the setting phase and the moving phase
	static const unsigned int 			NUM_LAYERS						= 200;
	static const unsigned int 			MAX_NUM_SUB_LAYERS				= 100;
	static const unsigned int 			LAYER_INDEX_MOVING_PHASE		= 0;	
	static const unsigned int 			LAYER_INDEX_SETTING_PHASE		= 1;
	static constexpr unsigned int 		NOT_INDEXED						= 0xFFFFFFFFu;		// a constant that is used to indicate that a layer is not indexed
	static const unsigned int 			NUM_STONES_PER_PLAYER			= 9;

	// Symmetry Operations
	static constexpr symOperationId		SO_TURN_LEFT					=  0;
	static constexpr symOperationId		SO_TURN_180						=  1;
	static constexpr symOperationId		SO_TURN_RIGHT					=  2;
	static constexpr symOperationId		SO_DO_NOTHING					=  3;
	static constexpr symOperationId		SO_INVERT						=  4;
	static constexpr symOperationId		SO_MIRROR_VERT					=  5;
	static constexpr symOperationId		SO_MIRROR_HORI					=  6;
	static constexpr symOperationId		SO_MIRROR_DIAG_1				=  7;
	static constexpr symOperationId		SO_MIRROR_DIAG_2				=  8;
	static constexpr symOperationId		SO_INV_LEFT						=  9;
	static constexpr symOperationId		SO_INV_RIGHT					= 10;
	static constexpr symOperationId		SO_INV_180						= 11;
	static constexpr symOperationId		SO_INV_MIR_VERT					= 12;
	static constexpr symOperationId		SO_INV_MIR_HORI					= 13;
	static constexpr symOperationId		SO_INV_MIR_DIAG_1				= 14;
	static constexpr symOperationId		SO_INV_MIR_DIAG_2				= 15;
	static constexpr symOperationId		NUM_SYM_OPERATIONS				= 16;

private:	
	static constexpr unsigned int 		numSquaresGroupA				= 4;			// number of stonefields in group A
	static constexpr unsigned int 		numSquaresGroupB				= 4;			// ''
	static constexpr unsigned int 		numSquaresGroupC				= 8;			// ''
	static constexpr unsigned int 		numSquaresGroupD				= 8;			// ''
	static const unsigned int 			groupOrderA						= 7;			// ???
	static const unsigned int 			groupOrderB						= 3;			// ''
	static const unsigned int 			groupOrderC						= 15;			// ''
	static const unsigned int 			groupOrderD						= 7;			// ''
	static const unsigned int 			GROUP_A							= 0;			// index of the group
	static const unsigned int 			GROUP_B							= 1;			// ''
	static const unsigned int 			GROUP_C							= 2;			// ''
	static const unsigned int 			GROUP_D							= 3;			// ''
	static const unsigned int 			MAX_NUM_SITUATIONS_A			= 81;			// 3^numSquaresGroupA;
	static const unsigned int 			MAX_NUM_SITUATIONS_B			= 81;			// 3^numSquaresGroupB;
	static const unsigned int 			MAX_NUM_SITUATIONS_C			= 81*81;		// 3^numSquaresGroupC;
	static const unsigned int 			MAX_NUM_SITUATIONS_D			= 81*81;		// 3^numSquaresGroupD;

	// define the four groups	
	static constexpr unsigned int 		squareIndexGroupA[] 			= {  3,  5, 20, 18 };
	static constexpr unsigned int 		squareIndexGroupB[] 			= {  4, 13, 19, 10 };
	static constexpr unsigned int 		squareIndexGroupC[] 			= {  0,  2, 23, 21,  6,  8, 17, 15 };
	static constexpr unsigned int 		squareIndexGroupD[] 			= {  1,  7, 14, 12, 22, 16,  9, 11 };
	static constexpr unsigned int 		fieldPosIsOfGroup[] 			= { 
		GROUP_C,							GROUP_D,							GROUP_C,
					GROUP_A,        		GROUP_B,        		GROUP_A,
								GROUP_C,	GROUP_D,	GROUP_C,
		GROUP_D,	GROUP_B,	GROUP_D,				GROUP_D,	GROUP_B,	GROUP_D,
								GROUP_C,	GROUP_D,	GROUP_C,
					GROUP_A,				GROUP_B,				GROUP_A,
		GROUP_C,							GROUP_D,							GROUP_C
	};  

	#pragma region Symmetry Operations
    static constexpr unsigned int soTableTurnLeft[] = {        
		2,      14,      23,                     
		   5,   13,   20,                        
			  8,12,17,                           
		1, 4, 7,   16,19,22,                     
			  6,11,15,                           
		   3,   10,   18,                        
		0,       9,      21                      
	};                                        

	static constexpr unsigned int soTableDoNothing[] = {        
		0,       1,       2,                     
		   3,    4,    5,                        
			  6, 7, 8,                           
		9,10,11,   12,13,14,                     
			 15,16,17,                           
		  18,   19,   20,                        
		21,      22,      23                      
	};                                        

	static constexpr unsigned int soTableMirrorHori[] = {      
		21,      22,      23,                     
		   18,   19,   20,                        
			  15,16,17,                           
		9,10,11,   12,13,14,                     
			   6, 7, 8,                           
			3,    4,    5,                        
		0,       1,       2                       
	};                                        

	static constexpr unsigned int soTableTurn180[] = {    
		23,      22,      21,                
		   20,   19,   18,                   
			  17,16,15,                      
		14,13,12,   11,10, 9,                
				8, 7, 6,                      
			 5,    4,    3,                   
		 2,       1,       0                 
	};                                    

	static constexpr unsigned int soTableInvert[] = {  
		6,       7,       8,                
		   3,    4,    5,                   
			  0, 1, 2,                      
		11,10, 9,   14,13,12,                
			   21,22,23,                      
			18,   19,   20,                   
		15,      16,      17                 
	};

	static constexpr unsigned int soTableInvMirHori[] = {  
		15,      16,      17,                
		   18,   19,   20,                   
			  21,22,23,                      
		11,10, 9,   14,13,12,                
				0, 1, 2,                      
			 3,    4,    5,                   
		 6,       7,       8                 
	}; 

	static constexpr unsigned int soTableInvMirVert[] = {  
		8,       7,       6,                
		   5,    4,    3,                   
			  2, 1, 0,                      
		12,13,14,    9,10,11,                
			   23,22,21,                      
			20,   19,   18,                   
		17,      16,      15                 
	}; 

	static constexpr unsigned int soTableInvMirDiag1[] = {  
		17,      12,       8,                
		   20,   13,    5,                   
			  23,14, 2,                      
		16,19,22,    1, 4, 7,                
			  21, 9, 0,                      
		   18,   10,    3,                   
		15,      11,       6                 
	}; 

	static constexpr unsigned int soTableInvMirDiag2[] = {  
		6,      11,      15,                
		   3,   10,   18,                   
			  0, 9,21,                      
		7, 4, 1,   22,19,16,                
			  2,14,23,                      
		   5,   13,   20,                   
		8,      12,      17                 
	}; 

	static constexpr unsigned int soTableInvLeft[] = {  
		8,      12,      17,                
		   5,   13,   20,                   
			  2,14,23,                      
		7, 4, 1,   22,19,16,                
			  0, 9,21,                      
		   3,   10,   18,                   
		6,      11,      15                 
	}; 

	static constexpr unsigned int soTableInvRight[] = {  
		15,      11,       6,                
		   18,   10,    3,                   
			  21, 9, 0,                      
		16,19,22,    1, 4, 7,                
			  23,14, 2,                      
		   20,   13,    5,                   
		17,      12,       8                 
	}; 

	static constexpr unsigned int soTableInv180[] = {  
		17,      16,      15,                
		   20,   19,   18,                   
			  23,22,21,                      
		12,13,14,    9,10,11,                
				2, 1, 0,                      
			 5,    4,    3,                   
		 8,       7,       6                 
	}; 

	static constexpr unsigned int soTableMirrorDiag1[] = {
		0,       9,      21,                
		   3,   10,   18,                   
			  6,11,15,                      
		1, 4, 7,   16,19,22,                
			  8,12,17,                      
		   5,   13,   20,                   
		2,      14,      23                 
	};                                    

	static constexpr unsigned int soTableTurnRight[] = {
		21,       9,       0,
		   18,   10,    3,
			  15,11, 6,
		22,19,16,    7, 4, 1,
			  17,12, 8,
		   20,   13,    5,
		23,      14,       2
	};

	static constexpr unsigned int soTableMirrorVert[] = {
		2,       1,       0,
		   5,    4,    3,
			  8, 7, 6,
		14,13,12,   11,10, 9,
			  17,16,15,
		   20,   19,   18,
		23,      22,      21
	}; 

	static constexpr unsigned int soTableMirrorDiag2[] = {
		23,      14,       2,
		   20,   13,    5,
			  17,12, 8,
		22,19,16,    7, 4, 1,
			  15,11, 6,
		   18,   10,    3,
		21,       9,       0
	};
	#pragma endregion

	// structs
    struct subLayerStruct																											// each layer is devided into sublayers, based on the number of white/black stones in group C and D
    {   												
        groupIndex 			minIndex;																								// index of the first state of this sublayer in the database
        groupIndex 			maxIndex;																								// index of the last  state of this sublayer in the database
        numWhiteStones 		numWhiteStonesGroupCD;																					// number of white stones in group C and D
		numBlackStones 		numBlackStonesGroupCD;																					// number of black stones in group C and D
        numWhiteStones 		numWhiteStonesGroupAB;																					// number of white stones in group A and B
		numBlackStones 		numBlackStonesGroupAB;																					// number of black stones in group A and B
    };

	struct layerStruct																												// layer
	{												
		numWhiteStones		amountWhiteStones;																						// number of white stones
		numBlackStones		amountBlackStones;																						// number of black stones
        subLayerId			numSubLayers;																							// number of sublayers
        subLayerId			subLayerIndexAB[NUM_STONES_PER_PLAYER+1][NUM_STONES_PER_PLAYER+1];										// unused
        subLayerId			subLayerIndexCD[NUM_STONES_PER_PLAYER+1][NUM_STONES_PER_PLAYER+1];										// mapping [number of white stones in group CD][number of black stones in group CD] to index within subLayer[]
        subLayerStruct		subLayer[MAX_NUM_SUB_LAYERS];																			// sublayers

        unsigned int 		getStateNumberWithInSubLayer		(stateId stateNumber, bool settingPhase) const;
        void 				getNumGroupStonesByStateNumber		(stateId stateNumber, bool settingPhase, numWhiteStones &numWhiteStonesGroupAB, numBlackStones &numBlackStonesGroupAB, numWhiteStones &numWhiteStonesGroupCD, numBlackStones &numBlackStonesGroupCD) const;
    };
    
    // 1d, 2d, 3d vector types
	template<typename T>
	using vector1D = std::vector<T>;

	template<typename T>
	using vector2D = std::vector<std::vector<T>>;

	template<typename T>
	using vector3D = std::vector<std::vector<std::vector<T>>>;

	template <typename T>
	static void resizeVector1D(vector1D<T>& vec, T value, size_t size) {
		vec.resize(size, value);
	}

	template <typename T>
	static void resizeVector2D(vector2D<T>& vec, T value, size_t x, size_t y) {
		vec.resize(x, vector1D<T>(y, value));
	}

	template <typename T>
	static void resizeVector3D(vector3D<T>& vec, T value, size_t x, size_t y, size_t z) {
		vec.resize(x, vector2D<T>(y, vector1D<T>(z, value)));
	}

	// Since the calculation of the variables takes some time, they are cached in the file preCalcedVars.dat
	class cacheFile
	{
	private:
		struct fileHeaderStruct
		{
			unsigned int	sizeInBytes						= 0;
		};

		template <typename T>
		static bool writeVector(HANDLE hFile, const vector1D<T>& vec)
		{
			DWORD dwBytesWritten = 0;
			BOOL result = WriteFile(hFile, vec.data(), sizeof(T) * vec.size(), &dwBytesWritten, NULL);
			return result && dwBytesWritten == sizeof(T) * vec.size();
		}

		template <typename T>
		static bool writeVector(HANDLE hFile, const vector2D<T>& vec)
		{
			for (const auto& subVec : vec) {
				DWORD dwBytesWritten = 0;
				BOOL result = WriteFile(hFile, subVec.data(), sizeof(T) * subVec.size(), &dwBytesWritten, NULL);
				if (!result || dwBytesWritten != sizeof(T) * subVec.size()) {
					return false;
				}
			}
			return true;
		}
		
		template <typename T>
		static bool writeVector(HANDLE hFile, const vector3D<T>& vec)
		{
			for (const auto& subVec2D : vec) {
				for (const auto& subVec : subVec2D) {
					DWORD dwBytesWritten = 0;
					BOOL result = WriteFile(hFile, subVec.data(), sizeof(T) * subVec.size(), &dwBytesWritten, NULL);
					if (!result || dwBytesWritten != sizeof(T) * subVec.size()) {
						return false;
					}
				}
			}
			return true;
		}

		template <typename T>
		static void readVector(HANDLE hFile, vector1D<T>& vec)
		{
			DWORD dwBytesRead = 0;
			ReadFile(hFile, vec.data(), sizeof(T) * vec.size(), &dwBytesRead, NULL);
		}
		
		template <typename T>
		static void readVector(HANDLE hFile, vector2D<T>& vec)
		{
			DWORD dwBytesRead = 0;
			for (auto& subVec : vec) {
				ReadFile(hFile, subVec.data(), sizeof(T) * subVec.size(), &dwBytesRead, NULL);
			}
		}
		
		template <typename T>
		static bool readVector(HANDLE hFile, vector3D<T>& vec)
		{
			DWORD dwBytesRead = 0;
			for (auto& subVec2D : vec) {
				for (auto& subVec : subVec2D) {
					BOOL result = ReadFile(hFile, subVec.data(), sizeof(T) * subVec.size(), &dwBytesRead, NULL);
					if (!result || dwBytesRead != sizeof(T) * subVec.size())
						return false;
				}
			}
			return true;
		}

		// internal variables
		HANDLE 					hFile							= INVALID_HANDLE_VALUE;
		std::wstring			filePath;
		fileHeaderStruct		header;
		stateAddressing&		sa;
			
	public:	
								cacheFile						(std::wstring const& directory, stateAddressing& sa);
								~cacheFile						();
		
		bool 					readFromFile					();
		bool 					writeToFile						();
	};

	// mathematical functions
    static long long   			mOverN_Function				    (unsigned int m, unsigned int n);

    // symmetry functions	
    void						applySymmetryTransfToField  	(symOperationId symmetryOperationNumber, bool doInverseOperation, const fieldStruct::fieldArray& sourceField, fieldStruct::fieldArray& destField) const;
    void						applySymmetryTransfToField  	(symOperationId symmetryOperationNumber, bool doInverseOperation, const fieldStruct::millArray&  sourceField, fieldStruct::millArray&  destField) const;

	// helper functions	
	void 						adaptFieldArrayToCurPlayer		(const fieldStruct::fieldArray & srcField, fieldStruct::fieldArray & dstField, playerId curPlayer) const;
	void 						countStonesInGroup				(const fieldStruct::core& field, numWhiteStones& numWhiteStonesGroupAB, numBlackStones& numBlackStonesGroupAB, numWhiteStones& numWhiteStonesGroupCD, numBlackStones& numBlackStonesGroupCD) const;
	void 						countStonesInGroupCD			(const fieldStruct::core& field, numWhiteStones& numWhiteStonesGroupCD, numBlackStones& numBlackStonesGroupCD) const;
	bool 						addTotalNumMissingStonesOffset	(stateId & stateNumber, const fieldStruct::core& field) const;
	bool 						isSettingPhase					(layerId layerNum) const;
	static unsigned int 		getMaxTotalNumMissingStones		(numWhiteStones amountWhiteStones, numBlackStones amountBlackStones);
    unsigned int 				getTotalNumMissingStones		(stateId stateNumber, bool settingPhase, numWhiteStones amountWhiteStones, numBlackStones amountBlackStones) const;

	// init functions	
	void 						init_mOverN						();
	void 						init_powerOfThree				();
	void 						init_symOperationMappings		();
	void 						init_concSymOperation			();
    void 						init_group_AB					();
    void 						init_group_CD					();
	void 						initLayerRegardingSettingPhase	();
	void 						initLayerRegardingMovingPhase	();
	static inline void			calcFieldBasedOnGroup			(fieldStruct::fieldArray& field, unsigned int numSquaresInGroup, groupStateNumber state, const unsigned int* squareIndexGroup, unsigned int groupOrder, const vector1D<unsigned int>& powerOfThree);
	void						calcFieldBasedOnGroupAB			(fieldStruct::fieldArray& field, groupStateNumber stateAB) const;
	void						calcFieldBasedOnGroupCD			(fieldStruct::fieldArray& field, groupStateNumber stateCD) const;
    static inline void 			calcGroupStateNumberBasedOnField(const fieldStruct::fieldArray &field, unsigned int numSquaresInGroup, groupStateNumber &stateNumber, const unsigned int *squareIndexGroup, unsigned int groupOrder, const vector1D<unsigned int>& powerOfThree);
	void 						calcGroupStateNumberAB			(const fieldStruct::fieldArray &field, groupStateNumber &stateNumberAB) const;
	void 						calcGroupStateNumberCD			(const fieldStruct::fieldArray &field, groupStateNumber &stateNumberCD) const;
	void 						resizeGroupStateMappingArray	(vector3D<unsigned int> &originalState, const vector2D<unsigned int> *pAmountSituations, unsigned int numSquaresInGroup) const;

	// internal variables
	vector1D<groupIndex> 		groupIndexAB;					// mapping [groupStateNumber] to groupIndex within group AB
	vector1D<groupIndex> 		groupIndexCD;					// mapping [groupStateNumber] to groupIndex within group CD
	vector3D<groupStateNumber> 	groupStateAB;					// mapping [number of white stones][number of black stones][groupIndex] to groupStateNumber with in group AB
	vector3D<groupStateNumber> 	groupStateCD;					// mapping [number of white stones][number of black stones][groupIndex] to groupStateNumber with in group CD
	vector2D<groupIndex> 		amountSituationsAB;				// mapping [number of white stones][number of black stones] to number of situations for group A and B (considering symmetry operations). this corresponds to the maximum groupIndex within group AB
	vector2D<groupIndex> 		amountSituationsCD;				// mapping [number of white stones][number of black stones] to number of situations for group C and D (considering symmetry operations). this corresponds to the maximum groupIndex within group CD
	vector1D<symOperationId> 	symmetryOperationCD;			// index of symmetry operation used to get from the symmetric state to one listed in groupIndexCD
	vector1D<unsigned int> 		powerOfThree;					// 3^0, 3^1, 3^2, ...
	vector2D<unsigned int> 		mOverN;							// mapping [m][n] to m over n
	vector1D<symOperationId> 	reverseSymOperation;			// index of the reverse symmetry operation: [symmetry operation] -> reverse symmetry operation
	vector2D<unsigned int> 		symmetryTransformationTable;	// matrix used for application of the symmetry operations to the field: [symmetry operation][field position]
	vector3D<unsigned int> 		layerIndex;						// mapping [moving/setting phase][number of white stones][number of black stones] to layer index
	vector1D<layerStruct> 		layer;							// information about the layers
	
public:
	vector2D<symOperationId> 	concSymOperation;				// symmetry operation, which is identical to applying those two concatenated symmetry operations: [symmetry operation 1][symmetry operation 2] -> resulting symmetry operation

    // constructor
    							stateAddressing					(std::wstring const& directory);
	
    // getter	
	const layerStruct&			getLayer                        (layerId layerNum) const;
    unsigned int            	getNumberOfKnotsInLayer         (layerId layerNum) const;
    unsigned int 				getLayerNumber					(unsigned int numStonesOfCurPlayer, unsigned int numStonesOfOppPlayer, bool isSettingPhase) const;
    unsigned int 				getLayerNumber					(const fieldStruct::core& field) const;
    bool                    	getStateNumber                  (layerId layerNum, stateId& stateNumber, symOperationId& symOp, const fieldStruct::core& field) const;
    bool 						getFieldByStateNumber			(layerId layerNum, stateId stateNumber, fieldStruct& field, playerId curPlayer) const;

    // symmetry functions	
	bool						applySymmetryTransfToField  	(symOperationId symmetryOperationNumber, bool doInverseOperation, fieldStruct& field) const;
	bool						applySymmetryTransfToField  	(symOperationId symmetryOperationNumber, bool doInverseOperation, fieldStruct::core& field) const;
    bool 						getStateNumbersOfSymmetricStates(const fieldStruct::core& field, std::array<stateId, stateAddressing::NUM_SYM_OPERATIONS>& stateNumbers) const;
	bool						isSymOperationInvariant         (symOperationId symmetryOperation, const fieldStruct::core& field) const;
};

#endif // STATE_ADDRESSING_H
