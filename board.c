#include "board.h"

const uint DIRECTIONS[8][2] = {
	{ 0,1 },
	{ 0,-1 },
	{ -1,0 },
	{ 1,0 },
	{ 1,1 },
	{ -1,1 },
	{ -1,-1 },
	{ 1,-1 }
};

/**
Creates board using the user defined starting positions. Take note that in this code, we start counting from 0 hence position A1 in the real world is A0 in these code, hence token[1] - '1'.
**/
uint** initBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	//fprintf(stderr,"Starting boardPtr creation\n");
	char* token;
	
	uint** boardPtr = createBoard(width, height);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			boardPtr[i][j] = EMPTY;
		}
	}
	while ((token = mystrsep(&whiteStrPtr)) != NULL) {
		//fprintf(stderr,"Placing white token at %s\n", token);
		//fprintf(stderr, "Placing white token at %d-%d\n", c_to_n(token[0]), token[1] - '1');
		boardPtr[c_to_n(token[0])][token[1] - '1'] = WHITE;
	}
	//fprintf(stderr, "STR: %s\n", blackStrPtr);
	while ((token = mystrsep(&blackStrPtr)) != NULL) {
		//fprintf(stderr,"Placing black token at %s\n", token);
		//fprintf(stderr, "Placing black token at %d-%d\n", c_to_n(token[0]), token[1] - '1');
		boardPtr[c_to_n(token[0])][token[1] - '1'] = BLACK;
	}
	return boardPtr;
}

int countFlips(int x, int y, uint** boardPtr, int player, int width, int height) {
	// if the box is not EMPTY then return 0 flips which is an invalid move
	if (boardPtr[x][y] != EMPTY) {
		return 0;
	}
	int flips = 0;
	int counter = 0;
	int temp_x;
	int temp_y;

	for (int dir = 0; dir < 8; dir++) {
		temp_x = x + DIRECTIONS[dir][0];
		temp_y = y + DIRECTIONS[dir][1];

		while (temp_x < width && temp_x >= 0 && temp_y < height && temp_y >= 0) {

			if (boardPtr[temp_x][temp_y] == player) {
				flips += counter;
				counter = 0;
				break;
			}
			if (boardPtr[temp_x][temp_y] == EMPTY) {
				counter = 0;
				break;
			}
			counter++;
			temp_x += DIRECTIONS[dir][0];
			temp_y += DIRECTIONS[dir][1];
		}
		counter = 0;
	}

	return flips;
}

void placePiece(uint** boardPtr, uint player, int x, int y, int width, int height) {
	int flipsMade = 0;
	int counter = 0;
	int temp_x = x;
	int temp_y = y;
	for (int dir = 0; dir < 8; dir++) {
		temp_x = x + DIRECTIONS[dir][0];
		temp_y = y + DIRECTIONS[dir][1];
		// fprintf(stderr, "x is %d and y is %d\n",DIRECTIONS[dir][0],DIRECTIONS[dir][1]);
		// fprintf(stderr, "temp_x  is %d and temp_y is %d\n",temp_x ,temp_y);
		while (temp_x < width && temp_x >= 0 && temp_y < height && temp_y >= 0) {
			if (boardPtr[temp_x][temp_y] == player) {
				flipsMade += counter;
				for (int flipsMadeForThisDir = 0; flipsMadeForThisDir <= counter + 1; flipsMadeForThisDir++) {
					boardPtr[temp_x][temp_y] = player;
					temp_x -= DIRECTIONS[dir][0];
					temp_y -= DIRECTIONS[dir][1];
				}
				counter = 0;
				break;
			}
			if (boardPtr[temp_x][temp_y] == EMPTY) {
				counter = 0;
				break;
			}
			counter++;
			temp_x += DIRECTIONS[dir][0];
			temp_y += DIRECTIONS[dir][1];
		}
		counter = 0;
	}
}

uint** makeMove(uint** boardPtr, uint nextMove, uint player, int width, int height) {
	uint* move = to2d(nextMove, width);
	uint x = move[0];
	uint y = move[1];
	uint** newBoardPtr = duplicateBoard(boardPtr, width, height);

	placePiece(newBoardPtr, player, x, y, width, height);

	free(move);
	//printBoard(boardPtr,width,height);
	//fprintf(stderr, "Becomes this by making move %u-%u\n", to2d(nextMove, width)[0], to2d(nextMove, width)[1]);
	//printBoard(newBoardPtr,width, height);
	return newBoardPtr;
}

uint** createBoard(int width, int height) {
	// This way i make sure i create my board such that the data is contiguous
	uint** newBoardPtr = malloc(sizeof(uint*)*width);
	uint* newBoard = malloc(sizeof(uint)*height*width);
	for (int i = 0; i < width; i++) {
		newBoardPtr[i] = &newBoard[i*height];
	}
	return newBoardPtr;
}

uint** duplicateBoard(uint** boardPtr, int width, int height) {
	uint** newboardPtr = createBoard(width, height);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			newboardPtr[i][j] = boardPtr[i][j];
		}
	}
	return newboardPtr;
}

void freeBoard(uint** boardPtr) {
	free(boardPtr[0]);
	free(boardPtr);
}

int availableMoves(uint** boardPtr, int* movesPtr, int player, int width, int height) {
	int i = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (countFlips(x, y, boardPtr, player, width, height) > 0) {
				movesPtr[i] = to1d(x, y, width);
				i++;
				//fprintf(stderr,"%d-%d is a valid move\n", x,y);
			}
		}
	}
	return i;
}
