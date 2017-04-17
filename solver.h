#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "board.h"
int playMax(uint** boardPtr, int ply, int* bestMovesCount, uint* bestMovesArr, int alpha, int beta, int width, int height, long int maxBoards, int maxDepth, int ALPHA_BETA_PRUNING, int maxPlayer, int* numBoardsPtr);
int playMin(uint** boardPtr, int ply, int* bestMovesCount, uint* bestMovesArr, int alpha, int beta, int width, int height, long int maxBoards, int maxDepth, int ALPHA_BETA_PRUNING, int maxPlayer, int* numBoardsPtr);
int evaluate(uint ** boardPtr,int width, int height, int maxPlayer);

#endif