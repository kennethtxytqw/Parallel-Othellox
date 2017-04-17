#include "solver.h"
#include "board.h"

int playMax(uint** boardPtr, int ply, int* bestMovesCountPtr, uint* bestMovesArr, int alpha, int beta, int width, int height, long int maxBoards, int maxDepth, int abPruning, int maxPlayer, int* numBoardsPtr) {
	//fprintf(stderr, "Board %d x %d created with maxDepth: %d maxBoards: %d maxPlayer: %d abPruning: %d\n", width, height, maxDepth, maxBoards, maxPlayer, abPruning);
	if (ply == 0 || *numBoardsPtr >= maxBoards) {
		return evaluate(boardPtr, width, height, maxPlayer);
	}
	else {
		int value = INT_MIN;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, maxPlayer, width, height);
		if (numMoves == 0) {
			free(movesArr);
			return playMin(boardPtr, ply - 1, bestMovesCountPtr, bestMovesArr, alpha, beta, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
		}
		int temp;
		int bestMovesCount;
		
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], maxPlayer, width, height);
			*numBoardsPtr = *numBoardsPtr +1;
			temp = playMin(newBoard, ply - 1, bestMovesCountPtr, bestMovesArr, alpha, beta, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
			if (temp > value) {
				value = temp;
				bestMovesCount = 0;
			}
			if (temp >= value && ply == maxDepth) {
				bestMovesArr[bestMovesCount] = movesArr[i];
				bestMovesCount++;
				*bestMovesCountPtr = bestMovesCount;
				//fprintf(stderr,"Move %d has %d\n", movesArr[i], temp);
			}
			//fprintf(stderr, "BMC: %d\n", bestMovesCount);
			
			freeBoard(newBoard);
			if (abPruning) {
				if (value >= beta) {
					free(movesArr);
					return value;
				}
				else {
					alpha = value > alpha ? value : alpha;
				}
			}
		}
		//printf("Done Max\n");
		free(movesArr);
		return value;
	}
}

int playMin(uint** boardPtr, int ply, int* bestMovesCountPtr, uint* bestMovesArr, int alpha, int beta, int width, int height, long int maxBoards, int maxDepth, int abPruning, int maxPlayer, int* numBoardsPtr) {
	//fprintf(stderr, "Board %d x %d created with maxDepth: %d maxBoards: %d maxPlayer: %d abPruning: %d\n", width, height, maxDepth, maxBoards, maxPlayer, abPruning);
	if (ply == 0 || *numBoardsPtr >= maxBoards) {
		return evaluate(boardPtr, width, height, maxPlayer);
	}
	else {
		int value = INT_MAX;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, !maxPlayer,width, height);
		if (numMoves == 0) {
			free(movesArr);
			return playMax(boardPtr, ply - 1, bestMovesCountPtr, bestMovesArr, alpha, beta, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], !maxPlayer, width, height);
			*numBoardsPtr = *numBoardsPtr + 1;
			temp = playMax(newBoard, ply - 1, bestMovesCountPtr, bestMovesArr, alpha, beta, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
			value = temp < value ? temp : value;
			freeBoard(newBoard);
			if (abPruning) {
				if (value <= alpha) {
					free(movesArr);
					return value;
				}
				else {
					beta = value < beta ? value : beta;
				}
			}

		}
		//printf("Done Min\n");
		free(movesArr);
		return value;
	}
}

int evaluate(uint** boardPtr, int width, int height, int maxPlayer) {
	int score = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (boardPtr[i][j] == maxPlayer) {
				score++;
				if (i == 0 || width-1 == i) {
					score += 20;
				}
				if (j == 0 || height-1 == j ) {
					score += 20;
				}
			}
	
			//printf("%d-%d is %d\n", i, j, gptr->boardPtr[i][j]);
		}
	}

	return score;
}
