/*********************************************************************
	historyList.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "historyList.h"

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Initialization
//-----------------------------------------------------------------------------
void historyList::init(wildWeasel::masterMind* ww, muehle& game, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureField)
{
	// pointers
	this->ww			= ww;
	this->game			= &game;
	this->pFont			= &d3dFont2D;
	this->amAreaHistory = &newAlignment;
	
	followEvent(this, wildWeasel::eventFollower::eventType::WINDOWSIZE_CHANGED);

	// Buttons
	createControls();

	// calc position of gui elements
	resize(*amAreaHistory);
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
bool historyList::createControls()
{
	// parameters ok?
	if (ww == nullptr) return false;
	if (listViewHistory.isInitialized()) return false;
		
	// number of turns to remis
	labelTurnsToRemis.create(ww, wstring(L"Turns to Remis: 0"), pFont);
	labelTurnsToRemis.setTextColor(colTextGameStatus);
	labelTurnsToRemis.setTextState(wildWeasel::guiElemState::HIDDEN);
	labelTurnsToRemis.setTextSize(labelTextSizeFactor, labelTextSizeFactor);

	// number of repeated turns
	labelRepeatedTurns.create(ww, wstring(L"Repeated moves: 0"), pFont);
	labelRepeatedTurns.setTextColor(colTextGameStatus);
	labelRepeatedTurns.setTextState(wildWeasel::guiElemState::HIDDEN);
	labelRepeatedTurns.setTextSize(labelTextSizeFactor, labelTextSizeFactor);

	/*** create controls for database calculation ******************************************************************************************************************/
	buttonHistoryBack 	.create(ww, buttonImagesVoid, bind(&historyList::buttonFuncHistoryBack		, this, placeholders::_1), this, 0); buttonHistoryBack		.setText(L"<"	);	buttonHistoryBack	.setState(wildWeasel::guiElemState::HIDDEN);		buttonHistoryBack	.setTextState(wildWeasel::guiElemState::DRAWED);
	buttonHistoryForward.create(ww, buttonImagesVoid, bind(&historyList::buttonFuncHistoryForward	, this, placeholders::_1), this, 0); buttonHistoryForward	.setText(L">"	);	buttonHistoryForward.setState(wildWeasel::guiElemState::HIDDEN);		buttonHistoryForward.setTextState(wildWeasel::guiElemState::DRAWED);

	buttonHistoryBack	.setFont(pFont);	buttonHistoryBack	.setTextColor(wildWeasel::color::black());	buttonHistoryBack	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	buttonHistoryBack	.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	buttonHistoryBack	.setTextSize(0.5f, 0.5f);
	buttonHistoryForward.setFont(pFont);	buttonHistoryForward.setTextColor(wildWeasel::color::black());	buttonHistoryForward.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	buttonHistoryForward.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	buttonHistoryForward.setTextSize(0.5f, 0.5f);

	amListViewHistory	.create(ww->alignmentRootFrame);
	amHistoryLabels		.create(ww->alignmentRootFrame);
	amHistoryButtons	.create(ww->alignmentRootFrame);

	// create list view
	listViewHistory.create(ww, 0.0f);
	listViewHistory.setSelectionMode(wildWeasel::listView2D::selectionMode::NONE);
	listViewHistory.setMarkerColor(wildWeasel::color::lightBlue());
	listViewHistory.setColumnHeaderHeight(listViewRowHeight);
	listViewHistory.setVisibilityColumnHeader(true);
	listViewHistory.createScrollBars(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	listViewHistory.setColumnScrollBarHeight(listViewRowHeight);
	listViewHistory.setRowScrollBarWidth(listViewRowHeight);
	listViewHistory.setVisibilityColumnScrollBar(false);
	listViewHistory.setVisibilityRowScrollBar(true);
	listViewHistory.setPosition	(0, 0, true);
	listViewHistory.setAlignment(amListViewHistory);
	listViewHistory.setTextSize(0.6f, 0.6f);
	listViewHistory.alignAllItems();
	listViewHistory.setState(wildWeasel::guiElemState::HIDDEN);

    // Add columns to the list-view
	RECT rcCol;
	rcCol = {0, 0, static_cast<LONG>(listViewColWidthPlayer), 					static_cast<LONG>(listViewRowHeight)};	listViewHistory	.insertColumn_plainButton2D(0, wstring(L"Player"), pFont, listViewColWidthPlayer, 0, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0, static_cast<LONG>(listViewColWidthMove + listViewRowHeight), static_cast<LONG>(listViewRowHeight)};	listViewHistory	.insertColumn_plainButton2D(1, wstring(L"Move"	), pFont, listViewColWidthMove,   0, 0.5f, rcCol, buttonImagesVoid, 0);	
	listViewHistory.setState(wildWeasel::guiElemState::HIDDEN);
	listViewHistory.setColumnWidth(0, listViewColWidthPlayer);
	listViewHistory.setColumnWidth(1, listViewColWidthMove);

	// alignment
	buttonHistoryBack		.setAlignment(amHistoryButtons, 0);	
	buttonHistoryForward	.setAlignment(amHistoryButtons, 1);
	labelTurnsToRemis		.setAlignment(amHistoryLabels,  0);
	labelRepeatedTurns		.setAlignment(amHistoryLabels,  1);

	return true;
}

//-----------------------------------------------------------------------------
// Name: showCalculationControls()
// Desc: 
//-----------------------------------------------------------------------------
bool historyList::show(bool visible)
{
	if (!game || !ww || !pFont) return false;

	// locals
	auto	stateButton = visible ? wildWeasel::guiElemState::DRAWED  : wildWeasel::guiElemState::HIDDEN;
	auto	stateInfo   = visible ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN;

	// initialize lists
	if (!visible) {
		// delete items in listview
		listViewHistory.removeAllItems(true);
		listViewHistory.removeAllRows(true);
	}

	listViewHistory				.setVisibilityRowScrollBar(true);
	listViewHistory				.setState(stateButton);
	listViewHistory				.alignAllItems();

	buttonHistoryBack			.setState(stateButton);
	buttonHistoryForward		.setState(stateButton);

	labelTurnsToRemis			.setState(stateInfo  );
	labelRepeatedTurns			.setState(stateInfo  );
	labelTurnsToRemis			.setTextState(stateButton);
	labelRepeatedTurns			.setTextState(stateButton);

	showingControls	= visible;

	if (visible && amAreaHistory) {
		update();
		resize(*amAreaHistory);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: setCallBacks()
// Desc: Set the callbacks for undo and redo moves triggered by the buttons
//-----------------------------------------------------------------------------
void historyList::setCallBacks(voidFunc undoMove, voidFunc redoMove)
{
	this->undoMoveFunc = undoMove;
	this->redoMoveFunc = redoMove;
}

//-----------------------------------------------------------------------------
// Name: resizeCalculationArea()
// Desc:
//-----------------------------------------------------------------------------
void historyList::resize(wildWeasel::alignment& amNewArea)
{
	amAreaHistory	= &amNewArea;

	amListViewHistory		.setInsideAnotherRect(amNewArea);
	amHistoryLabels			.setInsideAnotherRect(amNewArea);
	amHistoryButtons		.setInsideAnotherRect(amNewArea);

	// align list view array and edit output box at the lower edge of the list view layer
	amListViewHistory		.top	.setRelation(wildWeasel::alignment::relDistance{amHistoryLabels		.bottom,  3 * defPixelDist + labelHeight});
	amListViewHistory		.bottom .setRelation(wildWeasel::alignment::relDistance{amHistoryButtons	.top,	 -defPixelDist});

	listViewHistory			.alignAllItems();
}		

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: Inform the gui elements about the new window size 
//-----------------------------------------------------------------------------
void historyList::windowSizeChanged(int xSize, int ySize)
{
	// TODO: Implement resizing logic for the history list controls
	// labelTurnsToRemis.setTextSize(xSize * labelTextSizeFactor, ySize * labelTextSizeFactor);
	// labelRepeatedTurns.setTextSize(xSize * labelTextSizeFactor, ySize * labelTextSizeFactor);
	// listViewHistory.setTextSize(xSize * labelTextSizeFactor, ySize * labelTextSizeFactor);
	// buttonHistoryBack.setTextSize(xSize * labelTextSizeFactor, ySize * labelTextSizeFactor);
	// buttonHistoryForward.setTextSize(xSize * labelTextSizeFactor, ySize * labelTextSizeFactor);

	// resize the alignment area
	if (showingControls && amAreaHistory) {
		resize(*amAreaHistory);
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncHistoryBack()
// Desc: 
//-----------------------------------------------------------------------------
void historyList::buttonFuncHistoryBack(void* pUser)
{
	if (undoMoveFunc) {
		undoMoveFunc();
		update();
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncHistoryForward()
// Desc: 
//-----------------------------------------------------------------------------
void historyList::buttonFuncHistoryForward(void* pUser)
{
	if (redoMoveFunc) {
		redoMoveFunc();
		update();
	}
}

//-----------------------------------------------------------------------------
// Name: update()
// Desc: 
//-----------------------------------------------------------------------------
bool historyList::update()
{
	if (!showingControls) return false;	

	// locals
	wstringstream					wssTmp;
	std::vector<muehle::logItem> 	log;

	// get the log from the game
	game->getLog(log, logId);
	logSize = static_cast<unsigned int>(log.size());
	listViewHistory.removeAllItems(true);
	listViewHistory.removeAllRows(true);

	// add a new row in the list view if not enough rows existend
	if (log.size() != listViewHistory.getNumRows())	{
		listViewHistory.insertRowsAndItems_plainButton2D(0, log.size(), listViewRowHeight, buttonImagesVoid, pFont);
		listViewHistory.alignAllItems();
	}

	// set the text of the labels
	for (size_t rowId = 0; rowId < log.size(); ++rowId)	{
		bool playerIsBlack = (log[rowId].player == fieldStruct::playerBlack);
		wssTmp.str(L""); wssTmp << (playerIsBlack ? L"Black" : L"White");
		listViewHistory.setItemText (listViewHistory.getItemIndex(rowId, 0), 0, wssTmp.str().c_str());
		wssTmp.str(L""); 
		if (log[rowId].move.from == fieldStruct::size) {
			wssTmp << "set stone to " << log[rowId].move.to;
		} else {
			wssTmp << "move from " << log[rowId].move.from << " to " << log[rowId].move.to;
		}
		if (log[rowId].move.removeStone != fieldStruct::size) {
			wssTmp << "\nremove stone from " << log[rowId].move.removeStone;
			listViewHistory.setRowHeight(rowId, 2 * listViewRowHeight);
		}
		listViewHistory.setItemText(listViewHistory.getItemIndex(rowId, 1), 0, wssTmp.str().c_str());
		listViewHistory.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
		if (rowId < logId) {
			listViewHistory.setRowColor(rowId, playerIsBlack ? wildWeasel::color::gray() : wildWeasel::color::white());
			listViewHistory.setRowTextColor(rowId, playerIsBlack ? wildWeasel::color::white() : wildWeasel::color::black());
		} else {
			listViewHistory.setRowColor(rowId, wildWeasel::color::darkGray());
			listViewHistory.setRowTextColor(rowId, wildWeasel::color::black());
		}
	}

	// show the last entry in the list view
	listViewHistory.scrollTo(wildWeasel::listView2D::scrollPosition::BOTTOM);
	
	// set the number of turns to remis
	labelTurnsToRemis.setText(wstring(L"Turns to Remis: " ) + to_wstring(game->getNumTurnsToRemis()));
	wstringstream repeatedMovesStream;
	repeatedMovesStream << fixed << setprecision(1) << game->getNumRepeatedMoves();
	labelRepeatedTurns.setText(wstring(L"Repeated moves: ") + repeatedMovesStream.str());

	// enable/disable buttons depending on the current position in the log
	setButtonsEnable(true);
	return true;
}

//-----------------------------------------------------------------------------
// Name: setButtonsEnable()
// Desc: 
//-----------------------------------------------------------------------------
void historyList::setButtonsEnable(bool enable)
{
	if (!showingControls) {
		buttonHistoryBack	.setState(wildWeasel::guiElemState::HIDDEN);
		buttonHistoryForward.setState(wildWeasel::guiElemState::HIDDEN);
		return;
	}
	auto stateButton = enable ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED;
	buttonHistoryBack	.setState(logId > 0 	  ? stateButton : wildWeasel::guiElemState::GRAYED);
	buttonHistoryForward.setState(logId < logSize ? stateButton : wildWeasel::guiElemState::GRAYED);
}
