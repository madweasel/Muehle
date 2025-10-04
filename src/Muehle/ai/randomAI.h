/*********************************************************************\
	randomAI.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef RANDOM_AI_H
#define RANDOM_AI_H

#include <cstdlib>
#include <ctime>
#include "../muehle.h"

/*** Klassen *********************************************************/

class randomAI : public muehleAI
{
public:
    // Constructor / destructor
    randomAI();
    ~randomAI();

	// Functions
	void 			play								(const fieldStruct& theField, moveInfo& move) override;
};

#endif // RANDOM_AI_H

