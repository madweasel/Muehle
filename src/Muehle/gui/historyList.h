/*********************************************************************
	historyList.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef HISTORY_LIST_H
#define HISTORY_LIST_H

#include "wildWeasel/src/wildWeasel.h"
#include "wildWeasel/src/wwListView.h"
#include "../muehle.h"

/*---------------------------------------------------
|													|
|	labelTurnsToRemis								|
|													|
|	labelRepeatedTurns								|
|													|
|	-------------------------------------			|
|	|									|			|
|	|									|			|
|	|		listViewHistory				|			|
|	|									|			|
|	|									|			|
|	|									|			|
|	|									|			|
|	-------------------------------------			|
|													|
|	-------------------  ---------------------  	|
|	buttonHistoryBack	 buttonHistoryForward    	|
|	-------------------  ---------------------  	|
----------------------------------------------------*/

class historyList : private wildWeasel::eventFollower
{
public:
	using voidFunc = std::function<void(void)>;

protected:

	// variables
	wildWeasel::masterMind *					ww									= nullptr;					// pointer to engine
	muehle *									game								= nullptr;					// object containing the game logic
	bool										showingControls						= false;					// true if the controls are visible
	unsigned int 								logId								= 0;						// current position in the log (0 = beginning, log.size() = end)
	unsigned int 								logSize								= 0;						// total number of log entries

	// positions, metrics, sizes, dimensions
	const unsigned int							listViewRowHeight					= 20;						// height in pixel of a single row of the list view
	const int		 							listViewColWidthPlayer				= 50;						// 
	const int 									listViewColWidthMove				= 150;						// 
	const float									defPixelDist						= 15;						// default pixel distance between gui elements
	const float									labelHeight							= 25;						// label height in pixel
	const float									buttonHeight						= 30;						// button height in pixel
	const float 								labelTextSizeFactor 				= 1.0f;						// factor for the text size of the labels

	// gui elements
	wildWeasel::plainButton2D					buttonHistoryBack;												// revert the last move
	wildWeasel::plainButton2D					buttonHistoryForward;											// redo the last move
	wildWeasel::listView2D						listViewHistory; 												// list view showing each move of the current game
	wildWeasel::textLabel2D						labelTurnsToRemis;												// label showing the number of turns to a remis
	wildWeasel::textLabel2D						labelRepeatedTurns;												// label showing the number of repeated turns
	wildWeasel::font2D*							pFont								= nullptr;					// used font in the controls
	wildWeasel::alignment*						amAreaHistory						= nullptr;					// parent alignment area provided by the caller
	wildWeasel::alignment						amHistoryLabels						= { wildWeasel::alignmentTypeX::BORDER_LEFT, 	defPixelDist,		// alignmentTypeX type_xPos, xPos
																						wildWeasel::alignmentTypeY::BORDER_TOP, 	defPixelDist,		// alignmentTypeY type_yPos, yPos
																						wildWeasel::alignmentTypeX::BORDER_RIGHT, 	-defPixelDist,		// alignmentTypeX type_width, width
																						wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 	labelHeight,		// alignmentTypeY type_height, height
																						wildWeasel::alignmentTypeX::PIXEL_WIDTH,   	defPixelDist, 		// alignmentTypeX type_xDist, xDist
																						wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   defPixelDist, 		// alignmentTypeY type_yDist, yDist
																						1, wildWeasel::alignment::posMode::ROW_WISE}; 					// periodicity, posMode::ROW_WISE
	wildWeasel::alignment						amListViewHistory					= { wildWeasel::alignmentTypeX::BORDER_LEFT,	defPixelDist,		
																						wildWeasel::alignmentTypeY::BORDER_TOP,		2*labelHeight+3*defPixelDist,
																						wildWeasel::alignmentTypeX::BORDER_RIGHT, 	-defPixelDist,		
																						wildWeasel::alignmentTypeY::BORDER_BOTTOM,	-buttonHeight-2*defPixelDist}; 			
	wildWeasel::alignment						amHistoryButtons					= { wildWeasel::alignmentTypeX::BORDER_LEFT, 	defPixelDist,
																						wildWeasel::alignmentTypeY::BORDER_BOTTOM,	-buttonHeight-defPixelDist,
																						wildWeasel::alignmentTypeX::PIXEL_WIDTH, 	static_cast<float>(listViewColWidthMove + listViewColWidthPlayer + listViewRowHeight - defPixelDist) / 2.0f,
																						wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 	buttonHeight,
																						wildWeasel::alignmentTypeX::PIXEL_WIDTH,   	defPixelDist, 
																						wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   defPixelDist, 
																						2, wildWeasel::alignment::posMode::ROW_WISE};
	wildWeasel::buttonImageFiles				buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};
	const wildWeasel::color						colTextGameStatus					= wildWeasel::color::gray();						// text color

	// callback functions
	voidFunc									undoMoveFunc						= nullptr;					// function pointer to undo the last move
	voidFunc									redoMoveFunc						= nullptr;					// function pointer to redo the last move

	// Functions
	void										buttonFuncHistoryBack				(void* pUser);
	void										buttonFuncHistoryForward			(void* pUser);
	bool										createControls						();
	void										windowSizeChanged					(int xSize, int ySize) override;

public:

	// Constructor / destructor
	void										init								(wildWeasel::masterMind* ww, muehle& game, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureLine);

	// Functions
	void 										setCallBacks						(voidFunc undoMove, voidFunc redoMove);
	void										resize								(wildWeasel::alignment &amNewArea);
	bool										show   								(bool visible);
	bool 										isVisible							() { return showingControls; };
	bool 										update								();
	void 										setButtonsEnable					(bool enable);
};

#endif
