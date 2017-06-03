#include "board.h"
#include "solver.h"
#include <pthread.h>

#define TYPE "Serial"

long long before, after;

long long comm_time = 0;
long long comp_time = 0;

char whiteStr[26 * 26 * 3];
char blackStr[26 * 26 * 3];
int width = -1;
int* widthPtr = &width;
int height = -1;
int* heightPtr = &height;
int maxDepth = -1;
int* maxDepthPtr = &maxDepth;
int maxPlayer = -1;
int* maxPlayerPtr = &maxPlayer;
int abPruning = 0;
int* abPruningPtr = &abPruning;
int numBoards = 0;
int* numBoardsPtr = &numBoards;
int maxBoards =-1;
int* maxBoardsPtr = &maxBoards;
char* whiteStrPtr = whiteStr;
char* blackStrPtr = blackStr;
int timeout = 0;
int* timeoutPtr = &timeout;
uint bestMovesArr[26];
int bestMovesCount = 0;
int* bestMovesCountPtr = &bestMovesCount;

int main(int argc, char **argv) {
	readInputs(argc, argv,widthPtr, heightPtr, maxDepthPtr, maxBoardsPtr, maxPlayerPtr, abPruningPtr, whiteStrPtr, blackStrPtr, timeoutPtr, TYPE);
	//fprintf(stderr,"Started\n");
	uint** boardPtr = initBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
	//fprintf(stderr,"Board %d x %d created with maxDepth: %d maxBoards: %d maxPlayer: %d abPruning: %d\n", width, height, maxDepth, maxBoards,maxPlayer,abPruning);

	//fprintf(stderr,"Game created\n");
	
	before = wall_clock_time();
	playMax(boardPtr, maxDepth, bestMovesCountPtr, bestMovesArr, INT_MIN, INT_MAX, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
	after = wall_clock_time();
	comp_time = after - before;
	saveRunResult(bestMovesArr, bestMovesCount, width, numBoards, comp_time, comm_time,comp_time+comm_time,1);
	printMovesArr(bestMovesArr, bestMovesCount, width);
	//fprintf(stderr,"Solved with %d best moves and %d numBoards\n", *bestMovesCountPtr,numBoards);
	//free(bestMovesArr);
	freeBoard(boardPtr);
	return 0;
}
