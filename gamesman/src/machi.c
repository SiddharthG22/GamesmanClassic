/************************************************************************
 **
 ** NAME:        machi.c
 **
 ** DESCRIPTION: Achi
 **
 ** AUTHOR:      Jeffrey Chiang
 **              Jennifer Lee
 **	       Jesse Phillips
 **
 ** DATE:        02/11/2003
 **
 ** UPDATE HIST:
 **
 ** 	2/11/2003 - wrote printPosition, doMove, generateMoves excluding diagonal moves 
 **	2/13/2003 - wrote get input functions, print functions, fixed errors
 **	2/20/2003 - wrote hash/unhash, whosemove, fixed domove, representation of our move
 **	2/27/2003 - wrote help strings, added nodiag, alldiag variations
 **       3/06/2003 - updated print position
 **
 **************************************************************************/

/*************************************************************************
 **
 ** Everything below here must be in every game file
 **
 **************************************************************************/

#include <stdio.h>
#include "gamesman.h"

extern STRING gValueString[];

int      gNumberOfPositions  = 39366; /*19683;*/  /* 3^9 */

POSITION gInitialPosition    =  0;
POSITION gMinimalPosition    = 0 ;
POSITION kBadPosition        = -1; /* This can never be the rep. of a position */

STRING   kGameName           = "Achi";

BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kSupportsHeuristic  = TRUE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING   kHelpGraphicInterface =
"There is currently no graphic interface\n";

STRING   kHelpTextInterface    =
"There are two types of turns in Achi: place moves, and slide moves.\n\
PLACE MOVES:\n\
First, you place your 3 pieces on the board: use the LEGEND to determine\n\
which number to choose (between 1 and 9, with 1 at the upper left and 9\n\
at the lower right) to correspond to the empty board position you desire\n\
and hit return.\n\
SLIDE MOVES:\n\
Next, once all 3 of your pieces are on the board, you make\n\
slides moves.  To do a slide move, you type two numbers instead of one:\n\
FIRST: choose a piece to move and type the number (1-9) of the space it is on.\n\
SECOND: type the number (1-9) of the space you would like to slide to\n\
        i.e. 23, 45, 69 etc.\n\
NOTE: you may only do slide moves between spots that are connected,\n\
the destination MUST be unoccupied, and you may not move to the same\n\
spot you are on (i.e. 11, 22, 33, etc are illegal slide moves).\n\
If at any point you have made a mistake, you can type u and hit return\n\
and the system will revert back to your most recent position.";

STRING   kHelpOnYourTurn =
"For the first six turns, each player puts the a piece on an empty board position.\n\
After all six pieces are on the board, you move any one of your pieces along a line to any open\n\
spot on the board in an attempt to get three in a row.";

STRING   kHelpStandardObjective =
"To get three of your markers (either X or O) in a row, either\n\
horizontally, vertically, or diagonally. 3-in-a-row WINS.";

STRING   kHelpReverseObjective =
"To force your opponent into getting three of his markers (either X or\n\
O) in a row, either horizontally, vertically, or diagonally. 3-in-a-row\n\
LOSES.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"NEVER!!!";

STRING   kHelpExample =
"         ( 1 2 3 )           : - - -\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  3    \n\n\
         ( 1 2 3 )           : - - X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move  : { 2 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - - \n\
         ( 7 8 9 )           : - - - \n\n\
Computer's move              :  6    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - - \n\n\
     Dan's move  : { 9 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X \n\
         ( 7 8 9 )           : - - O \n\n\
Computer's move              :  5    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : - - O \n\n\
     Dan's move  : { 7 } \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer's move              :  54    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X - X \n\
         ( 7 8 9 )           : O - O \n\n\
     Dan's move  : { 98 }    \n\n\
         ( 1 2 3 )           : - O X \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X - X \n\
         ( 7 8 9 )           : O O - \n\n\
Computer's move              :  35    \n\n\
         ( 1 2 3 )           : - O - \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : X X X \n\
         ( 7 8 9 )           : O - O \n\n\
Computer wins. Nice try, Dan.";


/*************************************************************************
 **
 ** Everything above here must be in every game file
 **
 **************************************************************************/

/*************************************************************************
 **
 ** Every variable declared here is only used in this file (game-specific)
 **
 **************************************************************************/

#define BOARDSIZE     9           /* 3x3 board */
#define NUMSYMMETRIES 8           /* 4 rotations, 4 flipped rotations */
#define WIDTH 3
#define POSITION_OFFSET  19683       /* 3^9 */

typedef enum possibleBoardPieces {
    Blank, o, x
} BlankOX;

char *gBlankOXString[] = { "�", "#", "$" };

/* Powers of 3 - this is the way I encode the position, as an integer */
int g3Array[] =          { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

BOOLEAN allDiag = FALSE;
BOOLEAN noDiag = FALSE;

static int gSymmetryMatrix[NUMSYMMETRIES][BOARDSIZE];

/* Proofs of correctness for the below arrays:
**
** FLIP						ROTATE
**
** 0 1 2	2 1 0		0 1 2		6 3 0		8 7 6		2 5 8
** 3 4 5  ->  	5 4 3		3 4 5	->	7 4 1  ->	5 4 3	->	1 4 7
** 6 7 8	8 7 6		6 7 8		8 5 2		2 1 0		2 1 0
*/

/* This is the array used for flipping along the N-S axis */
int gFlipNewPosition[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

/* This is the array used for rotating 90 degrees clockwise */
int gRotate90CWNewPosition[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };

/*************************************************************************
 **
 ** Here we declare the global database variables
 **
 **************************************************************************/

extern VALUE     *gDatabase;

/************************************************************************
 **
 ** NAME:        InitializeDatabases
 **
 ** DESCRIPTION: Initialize the gDatabase, a global variable.
 ** 
 ************************************************************************/

InitializeGame()
{
    GENERIC_PTR SafeMalloc();
    int i, j, temp;

/**
    gDatabase = (VALUE *) SafeMalloc (gNumberOfPositions * sizeof(VALUE));

    for(i = 0; i < gNumberOfPositions; i++)
	gDatabase[i] = undecided;
**/
}

/************************************************************************
 **
 ** NAME:        DebugMenu
 **
 ** DESCRIPTION: Menu used to debub internal problems. Does nothing if
 **              kDebugMenu == FALSE
 ** 
 ************************************************************************/

DebugMenu()
{
}

/************************************************************************
 **
 ** NAME:        GameSpecificMenu
 **
 ** DESCRIPTION: Menu used to change game-specific parmeters, such as
 **              the side of the board in an nxn Nim board, etc. Does
 **              nothing if kGameSpecificMenu == FALSE
 ** 
 ************************************************************************/

GameSpecificMenu()
{
    char GetMyChar();
    
    printf("\n");
    printf("Achi Game Specific Menu\n\n");
    printf("1) No diagonal moves \n");
    printf("2) All possible diagonal moves\n");
    printf("3) Standard diagonal moves (default)\n");
    printf("b) Back to previous menu\n\n");
    
    printf("Current option:   %s\n", allDiag ? "All diagonal moves": noDiag ? "No diagonal moves" : "Standard diagonal moves");
    printf("Select an option: ");
    
    switch(GetMyChar()) {
    case 'Q': case 'q':
	ExitStageRight();
    case '1':
	noDiag = TRUE;
	allDiag = FALSE;
	break;
    case '2':
	noDiag = FALSE;
	allDiag = TRUE;
	break;
    case '3':
	noDiag = FALSE;
	allDiag = FALSE;
	break;

	
    case 'b': case 'B':
	return;
    default:
	printf("\nSorry, I don't know that option. Try another.\n");
	HitAnyKeyToContinue();
	GameSpecificMenu();
	break;
    }
}


/************************************************************************
 **
 ** NAME:        SetTclCGameSpecificOptions
 **
 ** DESCRIPTION: Set the C game-specific options (called from Tcl)
 **              Ignore if you don't care about Tcl for now.
 ** 
 ************************************************************************/

SetTclCGameSpecificOptions(theOptions)
     int theOptions[];
{
    /* No need to have anything here, we have no extra options */
}

/************************************************************************
 **
 ** NAME:        DoMove
 **
 ** DESCRIPTION: Apply the move to the position.
 ** 
 ** INPUTS:      POSITION thePosition : The old position
 **              MOVE     theMove     : The move to apply.
 **
 ** OUTPUTS:     (POSITION) : The position that results after the move.
 **
 ** CALLS:       PositionToBlankOX(POSITION,*BlankOX)
 **              BlankOX WhosTurn(*BlankOX)
 **
 ************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
    
    BlankOX theBlankOX[BOARDSIZE];
    int from, to;
    BlankOX whosTurn;
    BOOLEAN phase1();

        
    PositionToBlankOX(thePosition,theBlankOX, &whosTurn);
    from = 0;
    to = 0;
   
    if(phase1(theBlankOX)) {
	theBlankOX[theMove-1] = whosTurn;
    }
    else {
	
	from = theMove/10;
	to = theMove%10;
	theBlankOX[to-1] = theBlankOX[from-1];
	theBlankOX[from-1] = Blank;
    }
    if(whosTurn == o)
	whosTurn = x;
    else
	whosTurn = o;
    
    return BlankOXToPosition(theBlankOX,whosTurn);
}

/************************************************************************
 **
 ** NAME:        GetInitialPosition
 **
 ** DESCRIPTION: Ask the user for an initial position for testing. Store
 **              it in the space pointed to by initialPosition;
 ** 
 ** OUTPUTS:     POSITION initialPosition : The position to fill.
 **
 ************************************************************************/

/*  we haven't changed this, but we probably should */

GetInitialPosition()
{
    POSITION BlankOXToPosition();
    BlankOX theBlankOX[BOARDSIZE], whosTurn;
    signed char c;
    int i, goodInputs = 0;


    printf("\n\n\t----- Get Initial Position -----\n");
    printf("\n\tPlease input the position to begin with.\n");
    printf("\tNote that it should be in the following format:\n\n");
    printf("O - -\nO - -            <----- EXAMPLE \n- X X\n\n");

    i = 0;
    getchar();
    while(i < BOARDSIZE && (c = getchar()) != EOF) {
	if(c == 'x' || c == 'X')
	    theBlankOX[i++] = x;
	else if(c == 'o' || c == 'O' || c == '0')
	    theBlankOX[i++] = o;
	else if(c == '-')
	    theBlankOX[i++] = Blank;
	else
	    ;   /* do nothing */
    }

    /*
      getchar();
      printf("\nNow, whose turn is it? [O/X] : ");
      scanf("%c",&c);
      if(c == 'x' || c == 'X')
      whosTurn = x;
      else
      whosTurn = o;
    */

    return(BlankOXToPosition(theBlankOX,whosTurn));
}

/************************************************************************
 **
 ** NAME:        GetComputersMove
 **
 ** DESCRIPTION: Get the next move for the computer from the gDatabase
 ** 
 ** INPUTS:      POSITION thePosition : The position in question.
 **
 ** OUTPUTS:     (MOVE) : the next move that the computer will take
 **
 ** CALLS:       POSITION GetCanonicalPosition (POSITION)
 **              MOVE     DecodeMove (POSITION,POSITION,MOVE)
 **
 ************************************************************************/
/**
MOVE GetComputersMove(thePosition)
     POSITION thePosition;
{
    MOVE theMove;
    int i, randomMove, numberMoves = 0;
    MOVELIST *ptr, *head, *GetValueEquivalentMoves();

    //if(gPossibleMoves) 
//	printf("%s could equivalently choose [ ", gPlayerName[kComputersTurn]);
    head = ptr = GetValueEquivalentMoves(thePosition);
    while(ptr != NULL) {
	numberMoves++;
	//if(gPossibleMoves) 
	  //  printf("%d ",ptr->move+1);
	ptr = ptr->next;
    }
    if(gPossibleMoves) 
	printf("]\n\n");
    randomMove = GetRandomNumber(numberMoves);
    ptr = head;
    for(i = 0; i < randomMove ; i++)
	ptr = ptr->next;
    theMove = ptr->move;
    FreeMoveList(head);
    return(theMove);
}
***/

/************************************************************************
 **
 ** NAME:        PrintComputersMove
 **
 ** DESCRIPTION: Nicely format the computers move.
 ** 
 ** INPUTS:      MOVE   *computersMove : The computer's move. 
 **              STRING  computersName : The computer's name. 
 **
 ************************************************************************/

PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
    BOOLEAN phase1;
    if(computersMove < 9)
	phase1 = TRUE;
    else
	phase1 = FALSE;
    printf("%8s's move              : %2d\n", computersName, phase1 ? computersMove: computersMove );
}

/************************************************************************
 **
 ** NAME:        Primitive
 **
 ** DESCRIPTION: Return the value of a position if it fulfills certain
 **              'primitive' constraints. Some examples of this is having
 **              three-in-a-row with TicTacToe. TicTacToe has two
 **              primitives it can immediately check for, when the board
 **              is filled but nobody has one = primitive tie. Three in
 **              a row is a primitive lose, because the player who faces
 **              this board has just lost. I.e. the player before him
 **              created the board and won. Otherwise undecided.
 ** 
 ** INPUTS:      POSITION position : The position to inspect.
 **
 ** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
 **
 ** CALLS:       BOOLEAN ThreeInARow()
 **              BOOLEAN AllFilledIn()
 **              PositionToBlankOX()
 **
 ************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
    BOOLEAN ThreeInARow(), AllFilledIn();
    BlankOX theBlankOX[BOARDSIZE];
    BlankOX whosTurn;
      
    PositionToBlankOX(position,theBlankOX, &whosTurn);

    if( ThreeInARow(theBlankOX,0,1,2) || 
	ThreeInARow(theBlankOX,3,4,5) || 
	ThreeInARow(theBlankOX,6,7,8) || 
	ThreeInARow(theBlankOX,0,3,6) || 
	ThreeInARow(theBlankOX,1,4,7) || 
	ThreeInARow(theBlankOX,2,5,8) || 
	ThreeInARow(theBlankOX,0,4,8) || 
	ThreeInARow(theBlankOX,2,4,6) )
	return(gStandardGame ? lose : win);
    else if(AllFilledIn(theBlankOX))
	return(tie);
    else
	return(undecided);
}

/************************************************************************
 **
 ** NAME:        PrintPosition
 **
 ** DESCRIPTION: Print the position in a pretty format, including the
 **              prediction of the game's outcome.
 ** 
 ** INPUTS:      POSITION position   : The position to pretty print.
 **              STRING   playerName : The name of the player.
 **              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
 **
 ** CALLS:       PositionToBlankOX()
 **              GetValueOfPosition()
 **              GetPrediction()
 **
 ************************************************************************/

PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
    int i;
    STRING GetPrediction();
    VALUE GetValueOfPosition();
    BlankOX theBlankOx[BOARDSIZE];
    BlankOX whosTurn;
    
    PositionToBlankOX(position,theBlankOx, &whosTurn);
    if(noDiag) {
	printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]] );
	printf("                               |   |   |\n");
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]] );
	printf("                               |   |   |\n");
	printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       GetPrediction(position,playerName,usersTurn));
    }
    else if(allDiag) {
	printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]] );
	printf("                               | X | X |\n");
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]] );
	printf("                               | X | X |\n");
	printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       GetPrediction(position,playerName,usersTurn));
    }

    
    else {
	printf("\n         ( 1 2 3 )           : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[0]],
	       gBlankOXString[(int)theBlankOx[1]],
	       gBlankOXString[(int)theBlankOx[2]] );
	printf("                               | \\ | / |\n");
	printf("LEGEND:  ( 4 5 6 )  TOTAL:   : %s - %s - %s\n",
	       gBlankOXString[(int)theBlankOx[3]],
	       gBlankOXString[(int)theBlankOx[4]],
	       gBlankOXString[(int)theBlankOx[5]] );
	printf("                               | / | \\ |\n");
	printf("         ( 7 8 9 )           : %s - %s - %s %s\n\n",
	       gBlankOXString[(int)theBlankOx[6]],
	       gBlankOXString[(int)theBlankOx[7]],
	       gBlankOXString[(int)theBlankOx[8]],
	       GetPrediction(position,playerName,usersTurn));
    }
}

/************************************************************************
 **
 ** NAME:        GenerateMoves
 **
 ** DESCRIPTION: Create a linked list of every move that can be reached
 **              from this position. Return a pointer to the head of the
 **              linked list.
 ** 
 ** INPUTS:      POSITION position : The position to branch off of.
 **
 ** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
 **              in the linked list of moves that can be generated.
 **
 ** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
 **
 ************************************************************************/

MOVELIST *GenerateMoves(position)
     POSITION position;
{
    MOVELIST *CreateMovelistNode(), *head = NULL;
    VALUE Primitive();
    BlankOX player;
    BlankOX theBlankOX[BOARDSIZE];
    BOOLEAN phase1();
    BlankOX whosTurn;
    MOVE aMove;
    int i;
    
    PositionToBlankOX(position, theBlankOX, &whosTurn);
    player = whosTurn;
    if (Primitive(position) == undecided) {

	if(phase1(theBlankOX)) {
	    for(i = 0 ; i < BOARDSIZE ; i++) {
		if(theBlankOX[i] == Blank)
		    head = CreateMovelistNode(i+1,head);
	    }
	}
	else {
	    for (i=0; i<BOARDSIZE; i++) {
		if (theBlankOX[i]==player) {
	
		    //move left
		    if ((i%WIDTH)-1>=0 && theBlankOX[i-1]==Blank) {
			aMove = i*10 + i - 1 + 11;	
			head = CreateMovelistNode(aMove, head);
		    }
      
		    //move right
		    if ((i%WIDTH)+1<=WIDTH-1 && theBlankOX[i+1]==Blank) {
			aMove = i*10 + i + 1 + 11;	
			head = CreateMovelistNode(aMove, head);
		    }
      
		    //move up
		    if (i-WIDTH>=0 && theBlankOX[i-WIDTH]==Blank) {
			aMove = i*10 + i - WIDTH + 11;	
			head = CreateMovelistNode(aMove, head);
		    }
      
		    //move down
		    if (i+WIDTH<=BOARDSIZE-1 && theBlankOX[i+WIDTH]==Blank) {
			aMove = i*10 + i + WIDTH + 11;	
			head = CreateMovelistNode(aMove, head);
		    }

		}
	    }


	    if(!noDiag) {
		for(i=8; i>=0; i-=2) {
		    if(i!=4) {
			if(theBlankOX[i] == whosTurn && theBlankOX[4] == Blank) {
			    aMove = i*10 + 4 + 11;
			    head = CreateMovelistNode(aMove, head);
			}
		    }
		}

		if(theBlankOX[4] == whosTurn) {
		    for(i=8; i>=0; i-=2) {
			if(i!=4) {
			    if(theBlankOX[i] == Blank && theBlankOX[4] == whosTurn) {
				aMove = 40 + i + 11;
				head = CreateMovelistNode(aMove, head);
			    }
			}
		    }
		}



		if(allDiag) {
		   
		    for(i = 7; i>=1; i-=2) {
			if(theBlankOX[i] == whosTurn && (i == 1 || i == 7)) {
			    if(theBlankOX[3] == Blank) {
				aMove = (i*10) + 11 + 3;
				head = CreateMovelistNode(aMove, head);
			    }
			    if(theBlankOX[5] == Blank) {
				aMove = (i*10) + 11 + 5;
				head = CreateMovelistNode(aMove, head);
			    }
			}
			if(theBlankOX[i] == whosTurn && (i == 3 || i == 5)) {
			    if(theBlankOX[1] == Blank) {
				aMove = (i*10) + 11 + 1;
				head = CreateMovelistNode(aMove, head);
			    }
			    if(theBlankOX[7] == Blank) {
				aMove = (i*10) + 11 + 7;
				head = CreateMovelistNode(aMove, head);
			    }
			}
		    }
		}
	    }
	}
	  
	return (head);
    }



    else {
	return(NULL);
    }
}

/************************************************************************
 **
 ** NAME:        GetAndPrintPlayersMove
 **
 ** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
 **              If so, return Undo or Abort and don't change theMove.
 **              Otherwise get the new theMove and fill the pointer up.
 ** 
 ** INPUTS:      POSITION *thePosition : The position the user is at. 
 **              MOVE *theMove         : The move to fill with user's move. 
 **              STRING playerName     : The name of the player whose turn it is
 **
 ** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
 **
 ** CALLS:       ValidMove(MOVE, POSITION)
 **              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
 **
 ************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
{
    int xpos, ypos;
    BOOLEAN ValidMove();
    char input = '0';
    BOOLEAN done = FALSE;
    USERINPUT ret, HandleDefaultTextInput();
  
    do {
	printf("%8s's move :  ", playerName);
    
	ret = HandleDefaultTextInput(thePosition, theMove, playerName);
	if(ret != Continue)
	    return(ret);
    
    }
    while (TRUE);
    return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
 **
 ** NAME:        ValidTextInput
 **
 ** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
 **              For example, if the user is allowed to select one slot
 **              from the numbers 1-9, and the user chooses 0, it's not
 **              valid, but anything from 1-9 IS, regardless if the slot
 **              is filled or not. Whether the slot is filled is left up
 **              to another routine.
 ** 
 ** INPUTS:      STRING input : The string input the user typed.
 **
 ** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
 **
 ************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
    if(input[1] == NULL)
	return(input[0] <= '9' && input[0] >= '1');
    return(input[0] <= '9' && input[0] >= '1' && input[1] <= '9' && input[1] >= '1');
}

/************************************************************************
 **
 ** NAME:        ConvertTextInputToMove
 **
 ** DESCRIPTION: Convert the string input to the internal move representation.
 ** 
 ** INPUTS:      STRING input : The string input the user typed.
 **
 ** OUTPUTS:     MOVE : The move corresponding to the user's input.
 **
 ************************************************************************/

MOVE ConvertTextInputToMove(input)
     STRING input;
{
    MOVE theMove = 0;
    if(input[1] == NULL)
	return((MOVE) input[0] - '0'); /* user input is 1-9, our rep. is 0-8 */
    else {
	theMove = ((MOVE) input[0] - '0') * 10;
	theMove = theMove + ((MOVE) input[1] - '0');
    }
    return theMove;
}

/************************************************************************
 **
 ** NAME:        PrintMove
 **
 ** DESCRIPTION: Print the move in a nice format.
 ** 
 ** INPUTS:      MOVE *theMove         : The move to print. 
 **
 ************************************************************************/

PrintMove(theMove)
     MOVE theMove;
{
    /* The plus 1 is because the user thinks it's 1-9, but MOVE is 0-8 */
    /*   if(theMove < 9)
	 printf("%d", theMove + 1);
	 else
	 printf("%d", theMove + 11); */
    printf("%d", theMove);
}

/************************************************************************
*************************************************************************
** BEGIN   FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
 **
 ** NAME:        StaticEvaluator
 **
 ** DESCRIPTION:  the Static Evaluator value
 **
 **              If the game is PARTIZAN:
 **              the value 0 => player 2's advantage
 **              the value 1 => player 1's advantage
 **              player 1 MAXIMIZES and player 2 MINIMIZES
 **
 **              If the game is IMPARTIAL
 **              the value 0 => losing position
 **              the value 1 => winning position
 **
 **              Not called if kSupportsHeuristic == FALSE
 ** 
 ** INPUTS:      POSITION thePosition : The position in question.
 **
 ** OUTPUTS:     (FUZZY) : the Fuzzy Static Evaluation value
 **
 ************************************************************************/

/**
FUZZY StaticEvaluator(thePosition)
     POSITION thePosition;
{
    return 0;
}
**/
/************************************************************************
 **
 ** NAME:        PositionToMinOrMax
 **
 ** DESCRIPTION: Given any position, this returns whether the player who
 **              has the position is a MAXIMIZER or MINIMIZER. If the
 **              game is IMPARTIAL (kPartizan == FALSE) then this procedure
 **              always returns MINIMIZER. See StaticEvaluator for the 
 **              reason. Note that for PARTIZAN games (kPartizan == TRUE):
 **              
 **              Player 1 MAXIMIZES
 **              Player 2 MINIMIZES
 **
 **              Not called if kSupportsHeuristic == FALSE
 ** 
 ** INPUTS:      POSITION thePosition : The position in question.
 **
 ** OUTPUTS:     (MINIMAX) : either minimizing or maximizing
 **
 ** CALLS:       PositionToBlankOx(POSITION,*BlankOX)
 **              BlankOX WhosTurn(*BlankOX)
 **
 ************************************************************************/
/**
MINIMAX PositionToMinOrMax(thePosition)
     POSITION thePosition;
{
    return 0;
}
**/
/************************************************************************
*************************************************************************
** END     FUZZY STATIC EVALUATION ROUTINES. DON'T WORRY ABOUT UNLESS
**         YOU'RE NOT GOING TO EXHAUSTIVELY SEARCH THIS GAME
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
** BEGIN   PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
 **
 ** NAME:        GetRawValueFromDatabase
 **
 ** DESCRIPTION: Get a pointer to the value of the position from gDatabase.
 ** 
 ** INPUTS:      POSITION position : The position to return the value of.
 **
 ** OUTPUTS:     (VALUE *) a pointer to the actual value.
 **
 ** CALLS:       POSITION GetCanonicalPosition (POSITION)
 **
 ************************************************************************/
/**
VALUE *GetRawValueFromDatabase(position)
     POSITION position;
{
    return(&gDatabase[position]);
}
**/
/************************************************************************
 **
 ** NAME:        GetNextPosition
 **
 ** DESCRIPTION: Return the next non-undecided position when called 
 **              consecutively. When done, return kBadPosition and
 **              reset internal counter so that if called again,
 **              would start from the beginning.
 ** 
 ** OUTPUTS:     (POSITION) : the next non-Undecided position
 **
 ************************************************************************/

/**
POSITION GetNextPosition()
{
    VALUE GetValueOfPosition();
    static POSITION thePosition = 0; // Cycle through every position
    POSITION returnPosition;

    while(thePosition < gNumberOfPositions &&  GetValueOfPosition(thePosition) == undecided)
	thePosition++;

    if(thePosition == gNumberOfPositions) {
	thePosition = 0;
	return(kBadPosition);
    }
    else {
	returnPosition = thePosition++;
	return(returnPosition);
    }
}
**/
/************************************************************************
*************************************************************************
** END     PROBABLY DON'T HAVE TO CHANGE THESE SUBROUTINES UNLESS YOU
**         FUNDAMENTALLY WANT TO CHANGE THE WAY YOUR GAME STORES ITS
**         POSITIONS IN THE TABLE FROM AN ARRAY TO SOMETHING ELSE
**         AND ALSO CHANGE THE DEFINITION OF A POSITION (NOW AN INT)
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/

/************************************************************************
 **
 ** NAME:        PositionToBlankOX
 **
 ** DESCRIPTION: convert an internal position to that of a BlankOX.
 ** 
 ** INPUTS:      POSITION thePos     : The position input. 
 **              BlankOX *theBlankOx : The converted BlankOX output array. 
 **
 ** CALLS:       BadElse()
 **
 ************************************************************************/

PositionToBlankOX(thePos,theBlankOX,whosTurn)
     POSITION thePos;
     BlankOX *theBlankOX, *whosTurn;
{
    int i;

    if(thePos >= POSITION_OFFSET) {
	*whosTurn = x;  /* if the last character in the array is an x */
	thePos -= POSITION_OFFSET;
    }
    else
	*whosTurn = o;
    for(i = 8; i >= 0; i--) {
	if(thePos >= ((int)x * g3Array[i])) {
	    theBlankOX[i] = x;
	    thePos -= (int)x * g3Array[i];
	}
	else if(thePos >= ((int)o * g3Array[i])) {
	    theBlankOX[i] = o;
	    thePos -= (int)o * g3Array[i];
	}
	else if(thePos >= ((int)Blank * g3Array[i])) {
	    theBlankOX[i] = Blank;
	    thePos -= (int)Blank * g3Array[i];
	}
	else
	    BadElse("PositionToBlankOX");
    }
}


/************************************************************************
 **
 ** NAME:        BlankOXToPosition
 **
 ** DESCRIPTION: convert a BlankOX to that of an internal position.
 ** 
 ** INPUTS:      BlankOX *theBlankOx : The converted BlankOX output array.
 **
 ** OUTPUTS:     POSITION: The equivalent position given the BlankOX.
 **
 ************************************************************************/


POSITION BlankOXToPosition(theBlankOX, whosTurn)
     BlankOX *theBlankOX,whosTurn;
   
{
    int i;
    POSITION position = 0;

    for(i = 0 ; i < BOARDSIZE ; i++)
	position += g3Array[i] * (int)theBlankOX[i]; /* was (int)position... */

    if(whosTurn == x)
	position += POSITION_OFFSET;   /* account for whose turn it is */
      
    return(position);
}



/************************************************************************
 **
 ** NAME:        ThreeInARow
 **
 ** DESCRIPTION: Return TRUE iff there are three-in-a-row.
 ** 
 ** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
 **              int a,b,c                     : The 3 positions to check.
 **
 ** OUTPUTS:     (BOOLEAN) TRUE iff there are three-in-a-row.
 **
 ************************************************************************/

BOOLEAN ThreeInARow(theBlankOX,a,b,c)
     BlankOX theBlankOX[];
     int a,b,c;
{
    return(	theBlankOX[a] == theBlankOX[b] && 
		theBlankOX[b] == theBlankOX[c] &&
		theBlankOX[c] != Blank );
}

/************************************************************************
 **
 ** NAME:        AllFilledIn
 **
 ** DESCRIPTION: Return TRUE iff all the blanks are filled in.
 ** 
 ** INPUTS:      BlankOX theBlankOX[BOARDSIZE] : The BlankOX array.
 **
 ** OUTPUTS:     (BOOLEAN) TRUE iff all the blanks are filled in.
 **
 ************************************************************************/

BOOLEAN AllFilledIn(theBlankOX)
     BlankOX theBlankOX[];
{
    BOOLEAN answer = TRUE;
    int i;
	
    for(i = 0; i < BOARDSIZE; i++)
	answer &= (theBlankOX[i] == o || theBlankOX[i] == x);

    return(answer);
}



BOOLEAN phase1(BlankOX *theBlankOX)
{
    int count = 0;
    int i = 0;
    while(i < BOARDSIZE)
	{
	    if(theBlankOX[i] != Blank)
		count++;
	    if(count >= 6)
		return FALSE;
	    i++;
	}
    return TRUE;
}



STRING kDBName = "achi" ;

int NumberOfOptions()
{
        return 2 ;
}

int getOption()
{
        if(gStandardGame) return 1 ;
        return 2 ;
} 

void setOption(int option)
{
        if(option == 1)
                gStandardGame = TRUE ;
        else
                gStandardGame = FALSE ;
}
int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) {}


