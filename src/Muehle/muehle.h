/*********************************************************************\
	muehle.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#ifndef MUEHLE_H
#define MUEHLE_H

#include <cstdio>
#include <vector>
#include <cstdlib>

#include "fieldStruct.h"

// base class representing the AI
class muehleAI
{
public:
    // Constructor / destructor
								muehleAI()						= default;
								~muehleAI()						= default;

	// Functions
	virtual void				play							(const fieldStruct& theField, moveInfo& move) = 0;
};

// class representing the game
class muehle
{
	// typedef
	using fieldPos 		= fieldStruct::fieldPos;
	using fieldArray 	= fieldStruct::fieldArray;

public:

	struct logItem
	{
		moveInfo move			= moveInfo{};					// move which is done
		playerId player			= playerId::squareIsFree;		// player who made the move

								logItem() = default;
								logItem(const moveInfo& move, playerId player);

		static unsigned int 	getNumNormalMovesWithoutRemoval	(const std::vector<logItem>& log);
		static float 			getNumRepeatedMoves				(const std::vector<logItem>& log);
		bool 					operator==						(const logItem& other) const;
	};

	// Constants
	static const unsigned int 	MaxNumMoves 					= 10000;				// maximum number of moves which can be saved in the history
	static const unsigned int   NumRepeatedMovesToRemis 		= 3;					// number of repeated moves after which the game is remis
	
    // Constructor / destructor
								muehle							();
								~muehle							();

	// Functions				
	void						beginNewGame					(muehleAI *firstPlayerAI, muehleAI *secondPlayerAI, playerId currentPlayer, bool settingPhase, bool resetField);
	void						setAI							(playerId player, muehleAI *AI);
	bool						moveStone						(const moveInfo& move);
	void 						setNumMovesToRemis				(unsigned int numMoves);
	bool						redoLastMove					();
	bool						undoLastMove					();

	// printing	
	void						printField						() const;

	// start the game with a customized state	
	bool						setCurrentGameState				(fieldStruct& curState);
    bool            			putStone						(fieldPos pos, playerId player);
    bool            			settingPhaseHasFinished			();
	void						calcNumberOfRestingStones		(int &numWhiteStonesResting, int &numBlackStonesResting);

	// get computer choice	
	void						getComputersChoice				(moveInfo& move) const;
	void						getChoiceOfSpecialAI			(muehleAI *AI, moveInfo& move) const;

	// getter				
	void						getLog							(std::vector<logItem>& log, unsigned int& currentIndex) const;
	const fieldArray& 			getField						() const;
	bool 						wouldMillBeClosed				(const moveInfo& move) const;
	bool						isCurrentPlayerHuman			() const;
	bool						isOpponentPlayerHuman			() const;	
	bool						inSettingPhase					() const	{	return field.inSettingPhase();									}
	unsigned int				mustStoneBeRemoved				() const	{	return stoneMustBeRemoved;										}
	bool 						gameHasFinished					() const;
	playerId					getWinner						() const;
	playerId					getCurrentPlayer				() const	{	return field.getCurPlayer().id;									}
	unsigned int    			getMovesDone					() const	{	return moveLogCurrentIndex;										}
	unsigned int    			getNumStonesSet					() const	{	return field.getNumStonesSet();									}
	playerId					getBeginningPlayer				() const	{	return beginningPlayer;											}
	unsigned int				getNumStonesOfCurPlayer			() const	{	return field.getCurPlayer().numStones;							}
	unsigned int				getNumStonesOfOppPlayer			() const	{	return field.getOppPlayer().numStones;							}
	unsigned int 				getNumTurnsToRemis				() const;
	float 						getNumRepeatedMoves				() const;
	bool 						isMoveAllowed					(const moveInfo& move, bool ignoreStoneRemoval) const;

private:
	// Variables	
	bool						stoneMustBeRemoved				= false;					// true if a mill was closed and the player must remove a stone. this also indicates that the move is not completed yet.
	unsigned int				moveLogCurrentIndex				= 0;						// index pointing to the current move in the history, when the user already went back some moves
	unsigned int   				numMovesToRemis 				= 205;						// number of moves after which the game is remis. this is not the current but the initial value.
	std::vector<logItem>		moveLog;													// array containing the history of moves done
	muehleAI *					playerOneAI						= nullptr;					// class-pointer to the AI of player one
	muehleAI *					playerTwoAI						= nullptr;					// class-pointer to the AI of player two
	fieldStruct					field;														// current field
	fieldStruct					initialField;												// undo of the last move is done by setting the initial field und performing all moves saved in history.
																							// the initial field is not necessarily an empty field. it can be any state.
	playerId					beginningPlayer					= playerId::playerOne;		// playerId of the player who makes the first move
};

#endif
