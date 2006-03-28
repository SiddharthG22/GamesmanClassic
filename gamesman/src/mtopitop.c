

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mtopitop.c
**
** DESCRIPTION: Topitop
**
** AUTHOR:      Mike Hamada and Alex Choy
**
** DATE:        BEGIN: 02/20/2006
**	              END: ???
**
** UPDATE HIST:
**	
**	    02/20/2006 - Setup #defines & data-structs
** 			 Wrote InitializeGame(), PrintPosition()
**          02/22/2006 - Added CharToBoardPiece(), arrayHash(), and arrayUnhash()
**                       Still need to edit above functions with these new fcts
**                       Need to make arrayHash() a for loop
**	    02/26/2006 - Fixed errors that prevented game from being built
**			 Edited InitializeGame(), PrintPosition() to use new hashes
** 			 Fixed struct for board representation
** 			 Changed PrintPosition() since Extended-ASCII does not work
**			 Changed arrayHash() to use a for-loop to calculate hash
**			 Wrote Primitive() (unsure if it is finished or not)
**          02/26/2006 - Not sure if total num of positions should include player 
**                         turn (0 and 1 at msg, most significant digit)
**                       Wrote GetInitialPosition()
**                       For reference, MOVE = int, POSITION = int (from core/type.h)
**                       Wrote DoMove(), didn't test yet
**                       A move is represented using same hash/unhash as board, but
**                         only has the moved piece (if has one) and the new piece
**                       
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Topitop"; /* The name of your game */
STRING   kAuthorName          = "Alex Choy and Mike Hamada"; /* Your name(s) */
STRING   kDBName              = "topitop"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  2000000000; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
											 /* 10^9 * 2 = 2,000,000,000 */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet.";

STRING   kHelpTextInterface    =
"VALID MOVES:\n\
1.) Place 1 of your Buckets or 1 of the Small or Big Sand Piles\n\
    on any free space of the board.\n\
2.) Move 1 of your Buckets or any Small or Big Sand Pile already\n\
    placed on the board from one square to another (one space at\n\
    a time in any direction).\n\
       a.) Any piece already on the board can be moved to an \n\
           adjacent free space.\n\
       b.) A Bucket can go on top of a Small Sand Pile.\n\
       c.) A Small Sand Pile can go on top of a Big Sand Pile.\n\
       d.) A Small Sand Pile with a Bucket on its top can go on\n\
           top of a Big Sand Pile.\n\
       e.) A Bucket can go on top of a Sand Castle.\n\
    NOTE: You CANNOT place a piece on top of another piece.\n\
          Pieces must be MOVED on top of other pieces.\n\
3.) Move any combination of Sand Piles with your Buckets on top,\n\
    or any Sand Castle, to any free space.\n\
\n\
Each player takes turns making one valid move, also noting\n\
that player CANNOT reverse an opponent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.";

STRING   kHelpOnYourTurn =
"Note that a player CANNOT undo an oppontent's move that was just made.\n\
Use the LEGEND to determine which numbers to choose to\n\
correspond to either a piece to place and a board slot where to place it\n\
OR to the location of your piece or a neutral piece and the empty\n\
adjacent position you wish to move that piece to.\n\
Example: 's 1' would place a Small Sand Pile in position 1\n\
Example: '1 2' would move a piece in position 1 to position 2.";

STRING   kHelpStandardObjective =
"Be the first to have your 2 Buckets on top of 2 Sand Castles,\n\
where a Sand Castle consists of 1 Small Sand Pile put on top\n\
of 1 Big Sand Pile.";

STRING   kHelpReverseObjective =
"";	/* There exists no reverse objective */

STRING   kHelpTieOccursWhen =
"A tie never occurs.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define ROWCOUNT 3
#define COLCOUNT 3

#define BLANKPIECE '_'
#define SMALLPIECE 's'
#define LARGEPIECE 'l'
#define CASTLEPIECE 'c'
#define BLUEBUCKETPIECE 'b'
#define REDBUCKETPIECE 'r'
#define BLUESMALLPIECE 'X'
#define REDSMALLPIECE 'O'
#define BLUECASTLEPIECE 'B'
#define REDCASTLEPIECE 'R'
#define UNKNOWNPIECE '0'  // hopefully none of these b/c can't be represented by a digit from 0 - 9
#define UNKNOWNBOARDPIECE -1

#define HASHBLANK 0
#define HASHSANDPILE 1
#define HASHBLUEBUCKET 1
#define HASHREDBUCKET 2

#define BLUETURN 0
#define REDTURN 1

#define PLAYEROFFSET 1000000000
#define NUMCASTLESTOWIN 2

#define BOLD_UL_CORNER 201
#define BOLD_UR_CORNER 187
#define BOLD_LL_CORNER 200
#define BOLD_LR_CORNER 188
#define BOLD_HOR 205
#define BOLD_VERT 186
#define BOLD_HOR_DOWN 209
#define BOLD_HOR_UP 207
#define BOLD_VERT_LEFT 182
#define BOLD_VERT_RIGHT 199
#define HOR_LINE 196
#define VERT_LINE 179
#define CROSS_LINE 197

typedef enum possibleBoardPieces {
    Blank = 0, SmallSand, LargeSand, SandCastle, BlueBucket, 
    RedBucket, BlueSmall, RedSmall, BlueCastle, RedCastle
} BoardPiece;

typedef enum playerTurn {
	Blue = 0, Red
} PlayerTurn;

// need this for unhashing...
// "the" used for structs in code (use as convention??)
typedef struct boardAndTurnRep {
  char *theBoard;
  PlayerTurn theTurn;
} *BoardAndTurn;

typedef struct tripleBoardRep {
	char *boardL;	// Holds blanks (0) and large sand piles (1)
	char *boardS;	// Holds blanks (0) and small sand piles (1)
	char *boardB;	// Holds blanks (0), blue buckets (1), and red buckets (2).
} *BoardRep;

typedef struct threePieces {
	char L;
	char S;
	char B;
} *ThreePiece;

/*typedef struct structMove {
  int theFromLoc;
  int theToLoc;
  } sMove*/

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int boardSize = ROWCOUNT * COLCOUNT;
int rowWidth = COLCOUNT;
int redCastles, blueCastles, totalBoardPieces = 0;
int maxL, maxS, maxB = 0;

PlayerTurn gWhosTurn = Blue;
int gameType;
char* playerName;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR		SafeMalloc ();
extern void				SafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg));
extern POSITION         generic_hash(char *board, int player);
extern char             *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);
/* Internal */
void                    InitializeGame();
MOVELIST                *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* Game-specific */
char					BoardPieceToChar(BoardPiece piece);
BoardPiece 	        	CharToBoardPiece(char piece);
POSITION				arrayHash(BoardAndTurn board);
BoardAndTurn			arrayUnhash(POSITION hashNumber);
/*sMove                   moveUnhash(MOVE move);*/
/*BOOLEAN               OkMove(char *theBlankFG, int whosTurn, SLOT fromSlot,SLOT toSlot);
BOOLEAN                 CantMove(POSITION position);
void                    ChangeBoard();
void                    MoveToSlots(MOVE theMove, SLOT *fromSlot, SLOT *toSlot);
MOVE                    SlotsToMove (SLOT fromSlot, SLOT toSlot);
void                    InitializeAdjacency();
void                    InitializeOrder();*/


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
** 
************************************************************************/

void InitializeGame ()
{
    int i;
    int LpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4 };
    int SpiecesArray[] = { HASHBLANK, 5, 9, HASHSANDPILE, 0, 4 };
    int BpiecesArray[] = { HASHBLANK, 5, 9, HASHREDBUCKET, 0, 2, HASHBLUEBUCKET, 0, 2 };
    
    BoardAndTurn boardArray;
    boardArray = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
    boardArray->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
    
    maxL = generic_hash_init(boardSize, LpiecesArray, NULL);
    maxS = generic_hash_init(boardSize, SpiecesArray, NULL);
    maxB = generic_hash_init(boardSize, BpiecesArray, NULL);
    gNumberOfPositions = maxB + maxS * maxB + maxL * maxS * maxB;
    gWhosTurn = boardArray->theTurn = Blue;
    
    for (i = 0; i < boardSize; i++) {
    	boardArray->theBoard[i] = BLANKPIECE;
    }
    
    gInitialPosition = arrayHash(boardArray);
    SafeFree(boardArray->theBoard);
    SafeFree(boardArray);
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
    MOVELIST *moves = NULL;
    
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
    
    return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move) {
 	return 0;
 	int i;
	BoardPiece fromLoc = -1, toLoc;
	BoardPiece tempBoardPiece, tempMoveBoardPiece;
	POSITION newBoard = 0;
	BoardAndTurn board, moveBoard;  
  
	board = arrayUnhash(position);
	moveBoard = arrayUnhash(move);

 	/* not sure if we need to check if it is a valid move or not..
 	 * probably can just check the move list? or the move list should handle/filter
 	 * out the bad moves already...*/
  
	/* MOVE represented as a board...stores the new piece and the old, moved piece */
	for (i = 0; i < boardSize; i++) {
		tempMoveBoardPiece = CharToBoardPiece(moveBoard->theBoard[i]);
 		if (tempMoveBoardPiece != Blank) {
      		tempBoardPiece = CharToBoardPiece(board->theBoard[i]);
      		if (tempBoardPiece == tempMoveBoardPiece) {
      			/* this is the piece that was changed/affected */
      			fromLoc = i;
      		} else {
				/* this is where the move was made to */
				toLoc = i;
      		}
    	}
  	}
	/* add move to board */
 	if (fromLoc >= 0) {
		board->theBoard[fromLoc] = BLANKPIECE;
	}  
	board->theBoard[toLoc] = moveBoard->theBoard[toLoc];
	if (board->theTurn == Blue) {  /* blue's turn */
    	/* change the turn to 1 (red), and hash and return it */
    	board->theTurn = Red;
	} else { /* red's turn  */
    	/* change the turn to 0 (blue), and hash and return it */
    	board->theTurn = Blue;
	}
  
	newBoard = arrayHash(board);
	return newBoard;
  
	/* return 0;<-- their default return */
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

VALUE Primitive (POSITION position) {
    int i, blueCastles = 0, redCastles = 0;
    BoardAndTurn board;
    
    board = arrayUnhash(position);
    
    for (i = 0; i < boardSize; i++) {
    	if (board->theBoard[i] == BLUECASTLEPIECE) {
    		blueCastles++;
    	} else if (board->theBoard[i] == REDCASTLEPIECE) {
    		redCastles++;
    	}
    }
    
    if (((board->theTurn == Blue) && (blueCastles >= NUMCASTLESTOWIN)) ||
    	((board->theTurn == Red) && (redCastles >= NUMCASTLESTOWIN))) {
    	return win;
    } else if (((board->theTurn == Blue) && (redCastles >= NUMCASTLESTOWIN)) ||
   		((board->theTurn == Red) && (blueCastles >= NUMCASTLESTOWIN))) {
   		return lose;
    }
   	
    return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
	BoardAndTurn arrayHashedBoard;
 	int i;
  
	arrayHashedBoard = arrayUnhash(position);
	
	/***********************LINE 1**************************/
	printf("       *-*-*-*\n");
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_UL_CORNER, BOLD_HOR, BOLD_HOR_DOWN, BOLD_HOR,
									BOLD_HOR_DOWN, BOLD_HOR, BOLD_UR_CORNER);*/
	/***********************LINE 2**************************/
	printf("       |");
	for (i = 0; i < rowWidth; i++) {
		printf("%c|", BoardPieceToChar(arrayHashedBoard->theBoard[i]));
	}
	
	/*printf("       %c", BOLD_VERT);
	for (i = 0; i < rowWidth; i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]), 
										((i == (rowWidth-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("          ( 1 2 3 )\n");
	/***********************LINE 3**************************/
	printf("       *-+-+-*\n");
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);*/							
	/***********************LINE 4**************************/
	printf("BOARD: |");
	for (i = rowWidth; i < (rowWidth*2); i++) {
		printf("%c|", BoardPieceToChar(arrayHashedBoard->theBoard[i]));
	}
	
	/*printf("BOARD: %c", BOLD_VERT);
	for (i = rowWidth; i < (rowWidth*2); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]),
										((i == ((rowWidth*2)-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("  LEGEND: ( 4 5 6 )\n");
	/***********************LINE 5**************************/
	printf("       *-+-+-*\n");

	/*printf("       %c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, HOR_LINE, CROSS_LINE, HOR_LINE,
										CROSS_LINE, HOR_LINE, BOLD_VERT_LEFT);*/
	/***********************LINE 6**************************/
	printf("       |");
	for (i = rowWidth*2; i < (rowWidth*3); i++) {
		printf("%c|", BoardPieceToChar(arrayHashedBoard->theBoard[i]));
	}
	
	/*printf("       %c", BOLD_VERT);
	for (i = rowWidth*2; i < (rowWidth*3); i++) {
		printf("%c%c", BoardPieceToChar(arrayHashedBoard->theBoard[i]),
										((i == ((rowWidth*3)-1)) ? VERT_LINE : BOLD_VERT));
	}*/
	printf("          ( 7 8 9 )\n");
	/***********************LINE 7**************************/
	printf("       *-*-*-*\n");
	
	/*printf("       %c%c%c%c%c%c%c\n", BOLD_LL_CORNER, BOLD_HOR, BOLD_HOR_UP, BOLD_HOR,
									BOLD_HOR_UP, BOLD_HOR, BOLD_LR_CORNER);*/
	/***********************LINE 8, 9, 10, 11**************************/
	printf("                 Small Sand Pile = s\n");
	printf("                 Large Sand Pile = l\n");
	printf("                          Bucket = b\n");
	printf("\n%s\n\n", GetPrediction(position, playerName, usersTurn));
	SafeFree(arrayHashedBoard->theBoard);
	SafeFree(arrayHashedBoard);
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
    printf(" THIS IS PRINT COMPUTER'S MOVE!\n");
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
    printf(" THIS IS PRINT MOVE!\n");
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
    USERINPUT input;
    USERINPUT HandleDefaultTextInput();
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
		return input;
    }

    /* NOTREACHED */
    return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
    return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
    return 0;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
    printf(" THIS IS GAME SPECIFIC MENU!\n");
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{
    
}
  
  
/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
  InitializeGame();
  return 0;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
    return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/

char BoardPieceToChar(BoardPiece piece) {
	switch (piece) {
		case Blank:				return BLANKPIECE;
		case SmallSand:			return SMALLPIECE;
		case LargeSand:			return LARGEPIECE;
		case SandCastle:		return CASTLEPIECE;
		case BlueBucket:		return BLUEBUCKETPIECE;
		case RedBucket:			return REDBUCKETPIECE;
		case BlueSmall:			return BLUESMALLPIECE;
		case RedSmall:			return REDSMALLPIECE;
		case BlueCastle:		return BLUECASTLEPIECE;
		case RedCastle:			return REDCASTLEPIECE;
	}
	
	return UNKNOWNPIECE;
}

BoardPiece CharToBoardPiece(char piece) {
	switch (piece) {
	  case BLANKPIECE:			return Blank;
	  case SMALLPIECE:			return SmallSand;
	  case LARGEPIECE:			return LargeSand;
	  case CASTLEPIECE:			return SandCastle;
	  case BLUEBUCKETPIECE:		return BlueBucket;
	  case REDBUCKETPIECE:		return RedBucket;
	  case BLUESMALLPIECE:		return BlueSmall;
	  case REDSMALLPIECE: 		return RedSmall;
	  case BLUECASTLEPIECE:		return BlueCastle;
	  case REDCASTLEPIECE:		return RedCastle;
	}
	
	return UNKNOWNBOARDPIECE;
}

BoardPiece ThreePieceToBoardPiece(ThreePiece lsb) {
	if (lsb->L == HASHBLANK) {
		if (lsb->S == HASHBLANK) {
			if (lsb->B == HASHBLANK) { return Blank; }
			else if (lsb->B == HASHBLUEBUCKET) { return BlueBucket; }
			else if (lsb->B == HASHREDBUCKET) { return RedBucket; }
		} else if (lsb->S == HASHSANDPILE) {
			if (lsb->B == HASHBLANK) { return SmallSand; }
			else if (lsb->B == HASHBLUEBUCKET) { return BlueSmall; }
			else if (lsb->B == HASHREDBUCKET) { return RedSmall; }
		}
	} else if (lsb->L == HASHSANDPILE) {
		if (lsb->S == HASHBLANK) {
			if (lsb->B == HASHBLANK) { return LargeSand; }
			else if (lsb->B == HASHBLUEBUCKET) { return UNKNOWNBOARDPIECE; }
			else if (lsb->B == HASHREDBUCKET) { return UNKNOWNBOARDPIECE; }
		} else if (lsb->S == HASHSANDPILE) {
			if (lsb->B == HASHBLANK) { return SandCastle; }
			else if (lsb->B == HASHBLUEBUCKET) { return BlueCastle; }
			else if (lsb->B == HASHREDBUCKET) { return RedCastle; }
		}
	}
	
	return UNKNOWNBOARDPIECE;
}

char ThreePieceToChar(ThreePiece lsb) {
	return BoardPieceToChar(ThreePieceToBoardPiece(lsb));
}

ThreePiece BoardPieceToThreePiece(BoardPiece piece) {
	ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
	
	switch (piece) {
		case Blank:
			newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 0;
			break;
	  	case BlueBucket:
	  		newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 1;
			break;
	  	case RedBucket:
	  		newPiece->L = 0;
			newPiece->S = 0;
			newPiece->B = 2;
			break;
	  	case SmallSand:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 0;
			break;
	  	case BlueSmall:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 1;
			break;
	  	case RedSmall:
	  		newPiece->L = 0;
			newPiece->S = 1;
			newPiece->B = 2;
			break;
	  	case LargeSand:
	  		newPiece->L = 1;
			newPiece->S = 0;
			newPiece->B = 0;
			break;
	  	case SandCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 0;
			break;
	  	case BlueCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 1;
			break;
	  	case RedCastle:
	  		newPiece->L = 1;
			newPiece->S = 1;
			newPiece->B = 2;
			break;
		default:
			return NULL;
	}
	
	return newPiece;
}

ThreePiece CharToThreePiece(char piece) {
	return BoardPieceToThreePiece(CharToBoardPiece(piece));
}

/*
  arrayHash - hashes the board to a number
  Since there are 10 different pieces, this hash utilizes this fact and 
*/
POSITION arrayHash(BoardAndTurn board) {
	BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
	toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
	toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
	ThreePiece piece;
	POSITION L, S, B;
	int i;
	
	for (i = 0; i < boardSize; i++) {
		piece = CharToThreePiece(board->theBoard[i]);
		toHash->boardL = piece->L;
		toHash->boardS = piece->S;
		toHash->boardB = piece->B;
	}
	
	L = generic_hash(toHash->boardL, board.theTurn);
	S = generic_hash(toHash->boardS, board.theTurn);
	B = generic_hash(toHash->boardB, board.theTurn);
	
	return B + (S * maxB) + (L * maxS * maxB);
}

BoardAndTurn arrayUnhash(POSITION hashNumber) {
  BoardAndTurn board = (BoardAndTurn) SafeMalloc(sizeof(struct boardAndTurnRep));
  board->theBoard = (char *) SafeMalloc(boardSize * sizeof(char));
  BoardRep toHash = (BoardRep) SafeMalloc(sizeof(struct tripleBoardRep));
  toHash->boardL = (char *) SafeMalloc(boardSize * sizeof(char));
  toHash->boardS = (char *) SafeMalloc(boardSize * sizeof(char));
  toHash->boardB = (char *) SafeMalloc(boardSize * sizeof(char));
  ThreePiece newPiece = (ThreePiece) SafeMalloc(sizeof(struct threePieces));
  int i;
  
  POSITION L = hashNumber / (maxS * maxB);
  POSITION S = hashNumber % (maxS * maxB) / maxB;
  POSITION B = hashNumber / (maxB);
  
  generic_unhash(L, toHash->boardL);
  generic_unhash(S, toHash->boardS);
  generic_unhash(B, toHash->boardB);

  for (i = 0; i < boardSize; i++) {
  	newPiece->L = toHash->boardL[i];
  	newPiece->S = toHash->boardS[i];
  	newPiece->B = toHash->boardB[i];
  	board->theBoard[i] = ThreePieceToChar(newPiece);
  	board->theTurn = gWhosTurn;
  }
  
  return board;
}
/*
sMove moveUnhash(MOVE move) {
  struct goodMove m;
  
}*/

// $Log: not supported by cvs2svn $
// Revision 1.19  2006/03/20 02:21:09  mikehamada
// Finished arrayHash() and arrayUnhash()?
//
// Revision 1.18  2006/03/15 07:46:12  mikehamada
// Added HASHBLANK, HASHSANDPILE, HASHBLUEBUCKET, HASHREDBUCKET defines.
//
// Added BoardRep representation and ThreePiece representation (for a board, use a ThreePiece array!).
//
// Updated InitializeGame() to use LSB generic_hash_init (internal board will still be kept as a BoardAndTurn though!).
//
// Added ThreePieceToBoardPiece(), ThreePieceToChar(), BoardPieceToThreePiece(), CharToThreePiece() methods used for arrayHash() and arrayUnhash().
//
// Revision 1.17  2006/03/14 03:02:58  mikehamada
// Changed InitializeGame(), added BoardRep, changed BoardPiece
//
// Revision 1.16  2006/03/08 01:22:30  mikehamada
// *** empty log message ***
//
// Revision 1.15  2006/03/08 01:19:37  mikehamada
// Formatted DoMove Code
//
// Revision 1.14  2006/03/02 05:43:16  mikehamada
// *** empty log message ***
//
// Revision 1.13  2006/02/27 23:37:40  mikehamada
// *** empty log message ***
//
// Revision 1.12  2006/02/27 23:28:08  mikehamada
// Fixed Errors in DoMove and Primitive
//
// Revision 1.11  2006/02/27 00:19:08  alexchoy
// wrote DoMove and GetInitialPosition, untested
//
// Revision 1.9  2006/02/26 08:31:26  mikehamada
// Fixed errors that prevented game from being built
// Edited InitializeGame(), PrintPosition() to use new hashes
// Fixed struct for board representation
// Changed PrintPosition() since Extended-ASCII does not work
// Changed arrayHash() to use a for-loop to calculate hash
// Wrote Primitive() (unsure if it is finished or not)
//
// Revision 1.8  2006/02/25 19:20:15  mikehamada
// *** empty log message ***
//
// Revision 1.7  2006/02/25 09:33:55  mikehamada
// *** empty log message ***
//
// Revision 1.6  2006/02/25 06:32:09  mikehamada
// *** empty log message ***
//
// Revision 1.5  2006/02/24 17:34:43  mikehamada
// *** empty log message ***
//
// Revision 1.4  2006/02/23 07:19:20  mikehamada
// *** empty log message ***
//
// Revision 1.3  2006/02/22 09:49:04  alexchoy
// edited unhash and hash to make them more correct
//
// Revision 1.2  2006/02/22 09:37:24  alexchoy
// added hashing and unhashing functions
//
// Revision 1.1  2006/02/20 19:36:45  mikehamada
// First addition to repository for Topitop by Mike Hamada
// Setup #defines & data-structs
// Wrote InitializeGame() and PrintPosition()
//
// Revision 1.7  2006/01/29 09:59:47  ddgarcia
// Removed "gDatabase" reference from comment in InitializeGame
//
// Revision 1.6  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//
