/*********************************************************************
	randomAI.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "randomAI.h"

//-----------------------------------------------------------------------------
// Name: randomAI()
// Desc: randomAI class constructor
//-----------------------------------------------------------------------------
randomAI::randomAI()
{
	// Init
	srand( (unsigned)time( NULL ) );
}

//-----------------------------------------------------------------------------
// Name: ~randomAI()
// Desc: randomAI class destructor
//-----------------------------------------------------------------------------
randomAI::~randomAI()
{
	// Locals

}

//-----------------------------------------------------------------------------
// Name: play()
// Desc: 
//-----------------------------------------------------------------------------
void randomAI::play(const fieldStruct& theField, moveInfo& move)
{
	std::vector<unsigned int> possibilityIds;
	theField.getPossibilities(possibilityIds);

	// Select a random possibility
	if (!possibilityIds.empty()) {
		unsigned int randomIndex = rand() % possibilityIds.size();
		move = moveInfo::getMoveInfo(possibilityIds[randomIndex]);
	}
}
