#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <time.h>



#define BLACK 1
#define WHITE 0
#define EMPTY 2
#define TRUE 1
#define FALSE 0
#define PLY 3
#define SSIZE 8
#define uchar unsigned char
#define uint unsigned int
#define delimiters " ,"
#define INT_MAX 2^32

clock_t before, after;
const uchar alphabets[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
const char DELIM[] = delimiters;
long int comm_time = 0;
long int comp_time = 0;
long int numBoards = 0;
long int maxBoards;


FILE *brdtxt, *evaltxt, *ofp;
char* whiteInitialPos;
char* blackInitialPos;
char whiteStr[26 * 26 * 3];
char blackStr[26 * 26 * 3];
char maxPlayerStr[6];
char* maxPlayerStrPtr = maxPlayerStr;
char* whiteStrPtr = whiteStr;
char* blackStrPtr = blackStr;

int width;
int height;
int ply;
int maxPlayer;

void saveRunResult(uint * bestMovesArr, int bestMovesCount);
uint c_to_n(char c);
uint * to2d(uint i, int width);
int to1d(int x, int y, int width);
char * mystrsep(char ** stringp, const char * delim);
int availableMoves(uint ** boardPtr, int * movesPtr, int player);
int countFlips(int x, int y, uint ** boardPtr, int player);
uint ** createStandardBoard();
uint ** createBoard(int width, int height, int player, char * whiteStrPtr, char * blackStrPtr);
int solve(uint ** boardPtr, int ply, uint * bestMovesArr);
int playMax(uint ** boardPtr, int ply);
int playMin(uint ** boardPtr, int ply);
int evaluate(uint ** boardPtr);
uint ** makeMove(uint ** boardPtr, uint nextMove, uint player);
void placePiece(uint ** boardPtr, uint player, int x, int y);
uint ** duplicateBoard(uint ** boardPtr);
void freeBoard(uint ** boardPtr);
void printMovesArr(uint * MovesArr, uint count, int width);
void fprintMovesArr(FILE * ofp, uint * MovesArr, uint count, int width);
void readInputs(int argc, char ** argv);

uint c_to_n(char c)
{
	uint n = -1;
	static const char * const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *p = strchr(alphabet, toupper((unsigned char)c));

	if (p)
	{
		n = p - alphabet;
	}

	return n;
}

uint* to2d(uint i, int width) {
	uint* iarr = malloc(sizeof(uint)*2);
	uint x = i%width;
	uint y = i / width;
	iarr[0] = x;
	iarr[1] = y;
	return iarr;
}

int to1d(int x, int y, int width) {
	return y*width + x;
}

char* mystrsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;
	p = (start != NULL) ? strpbrk(start, delim) : NULL;
	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}
	/*if (p == "") {
		return mystrsep(stringp, delim);
	}*/
	return start;
}

int availableMoves(uint** boardPtr, int* movesPtr, int player){
	int i = 0;
	for(int y=0; y<height; y++){
		for(int x=0; x<width; x++){
			if(countFlips(x,y,boardPtr,player)>0){
				movesPtr[i]=to1d(x,y,width);
				i++;
				//printf("%c%d is a valid move\n", alphabets[x],y);
			}
		}
	}
	return i;
}

int countFlips(int x, int y, uint** boardPtr, int player){
	// if the box is not EMPTY then return 0 flips which is an invalid move
	if (boardPtr[x][y] != EMPTY) {
		return 0;
	}
	int flips=0;
	int counter=0;
	
	// Count right flips
	for(int i=x+1;i<width;++i){
		if(boardPtr[i][y]==player){
			flips += counter;
			break;
		}
		if(boardPtr[i][y]==EMPTY){
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, boardPtr[i][y] , gptr->player);
		counter++;
	}
	counter=0;

	//Count left flips
	for(int i=x-1;i>=0;--i){
		if(boardPtr[i][y]==player){
			flips += counter;
			break;
		}
		if(boardPtr[i][y]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count top flips
	for(int j=y+1;j<height;j++){
		if(boardPtr[x][j]==player){
			flips += counter;
			break;
		}
		if(boardPtr[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count bottom flips
	for(int j=y-1;j>=0;j--){
		if(boardPtr[x][j]==player){
			flips += counter;
			break;
		}
		if(boardPtr[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top right flips
	for (int i = 1; i + x < width && i + y < height; i++) {
		if (boardPtr[x + i][y + i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom left flips
	for (int i = 1; x-i >=0 && y-i >=0; i++) {
		if (boardPtr[x - i][y - i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top left flips
	for (int i = 1; x-i >= 0 && i + y <height; i++) {
		if (boardPtr[x - i][y + i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom right flips
	for (int i = 1; x + i <width && y-i >= 0; i++) {
		if (boardPtr[x + i][y - i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x + i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;
	//printf("checking %d-%d has %d\n", x,y,flips);
	return flips;
}


uint** createStandardBoard(){
	printf("Starting boardPtr creation\n");
	uint** boardPtr = malloc(sizeof(uint*)*SSIZE);
	for(int i =0; i<SSIZE; i++){
		boardPtr[i]= malloc(sizeof(uint)*SSIZE);
	}
	for (int i = 0; i < SSIZE; i++) {
		for (int j = 0; j < SSIZE; j++) {
			boardPtr[i][j] = 0;
		}
	}
	boardPtr[4][4]= WHITE;
	boardPtr[5][5]=WHITE;
	boardPtr[5][4]=BLACK;
	boardPtr[4][5]=BLACK;
	return boardPtr;
}

uint** createBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	printf("Starting boardPtr creation\n");
	char* token;
	//This way i make sure i create my board such that the data is contiguous
	uint** boardPtr = malloc(sizeof(uint*)*width);
	uint* board = malloc(sizeof(uint)*height*width);
	for (int i = 0; i<width; i++) {
		boardPtr[i] = &board[i*height];
	}
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			boardPtr[i][j] = EMPTY;
		}
	}
	while ((token = mystrsep(&whiteStrPtr, DELIM)) != NULL) {
		printf("Placing white token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]),token[1]-'0');
		boardPtr[c_to_n(token[0])][token[1]-'0'] = WHITE;
	}
	while ((token = mystrsep(&blackStrPtr, DELIM)) != NULL) {
		printf("Placing black token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]), token[1]-'0');
		boardPtr[c_to_n(token[0])][token[1]-'0'] = BLACK;
	}
	return boardPtr;
}

int solve(uint** boardPtr, int ply, uint* bestMovesArr) {
	if (ply == 0) {
		printf( "ply == 0");
		return 0;
	}
	int value = -INT_MAX;
	uint* movesArr = malloc(height*width * sizeof(uint));
	
	int numMoves = availableMoves(boardPtr, movesArr,maxPlayer);
	if (numMoves == 0) {
		printf("No available moves");
		return 0;
	}
	int temp;
	int bestMovesCount;
	for (int i = 0; i < numMoves; i++) {
		uint** newBoard = makeMove(boardPtr, movesArr[i], maxPlayer);
		temp = playMin(newBoard, ply - 1);
		if (temp > value) {
			value = temp;
			bestMovesCount = 0;
			bestMovesArr[bestMovesCount] = movesArr[i];
			bestMovesCount++;
			printf("Move %d has %d\n", movesArr[i], temp);
		}else if (temp == value) {
			value = temp;
			bestMovesArr[bestMovesCount] = movesArr[i];
			bestMovesCount++;
			printf("Move %d has %d\n", movesArr[i], temp);
		}
		freeBoard(newBoard);
	}
	free(movesArr);
	return bestMovesCount;
}

int playMax(uint** boardPtr, int ply) {
	if (ply == 0||numBoards==maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		int value = -INT_MAX;
		uint* movesArr = malloc(height*width*sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr,maxPlayer);
		if (numMoves == 0) {
			return playMin(boardPtr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i],maxPlayer);
			temp = playMin(newBoard, ply - 1);
			if (temp > value) {
				value = temp;
			}
			freeBoard(newBoard);
		}
		//printf("Done Max\n");
		free(movesArr);
		return value;
	}
}

int playMin(uint** boardPtr, int ply) {
	if (ply == 0 || numBoards == maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		int value = INT_MAX;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, !maxPlayer);
		if (numMoves == 0) {
			return playMax(boardPtr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i],!maxPlayer);
			temp = playMax(newBoard, ply - 1);
			if (temp < value) {
				value = temp;
			}
			freeBoard(newBoard);
		}
		//printf("Done Min\n");
		free(movesArr);
		return value;
	}
}

int evaluate(uint** boardPtr) {
	int score = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (boardPtr[i][j] == maxPlayer) {
				score++;
			}
			//printf("%d-%d is %d\n", i, j, gptr->boardPtr[i][j]);
		}
	}

	return score;
}

uint** makeMove(uint** boardPtr, uint nextMove, uint player) {
	uint* move = to2d(nextMove, width);
	uint x = move[0];
	uint y = move[1];
	uint** newBoardPtr = duplicateBoard(boardPtr);
	
	placePiece(newBoardPtr, player, x, y);
	numBoards++;
	free(move);
	return newBoardPtr;
}

void placePiece(uint** boardPtr, uint player, int x, int y) {
	int counter = 0;
	for (int i = x + 1; i<width; ++i) {
		if (boardPtr[i][y] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x+j][y] = player;
			}
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, boardPtr[i][y] , gptr->player);
		counter++;
	}
	counter = 0;

	//left flips
	for (int i = x - 1; i >= 0; --i) {
		if (boardPtr[i][y] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x-j][y] = player;
			}
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top flips
	for (int i = y + 1; i<height; i++) {
		if (boardPtr[x][i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x][y+j] = player;
			}
			break;
		}
		if (boardPtr[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom flips
	for (int i = y - 1; i >= 0; i--) {
		if (boardPtr[x][i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x][y - j] = player;
			}
			break;
		}
		if (boardPtr[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top right flips
	for (int i = 1; i + x < width && i + y < height; i++) {
		if (boardPtr[x + i][y + i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x +j][y + j] = player;
			}
			break;
		}
		if (boardPtr[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom left flips
	for (int i = 1; x - i >= 0 && y - i >= 0; i++) {
		if (boardPtr[x - i][y - i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x  - j][y - j] = player;
			}
			break;
		}
		if (boardPtr[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top left flips
	for (int i = 1; x - i >= 0 && i + y <height; i++) {
		if (boardPtr[x - i][y + i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x  - j][y + j] = player;
			}
			break;
		}
		if (boardPtr[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom right flips
	for (int i = 1; x + i <width && y - i >= 0; i++) {
		if (boardPtr[x + i][y - i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x  + j][y - j] = player;
			}
			break;
		}
		if (boardPtr[x + i][y - i] == EMPTY) {
			break;
		}
	}
}
uint** duplicateBoard(uint** boardPtr) {
	uint** newboardPtr = malloc(sizeof(uint*)*width);
	uint* newboard = malloc(sizeof(uint)*height*width);
	for (int i = 0; i<width; i++) {
		newboardPtr[i] = &newboard[i*height];
	}
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

void printMovesArr(uint* MovesArr, uint count, int width) {
	printf("Best Moves: { ");
	for (int i = 0; i < count;i++) {
		if (i > 0) {
			printf(", ");
		}
		uint* move = to2d(MovesArr[i],width);
		printf("%c%d", alphabets[move[0]], move[1]);
	}
	printf(" }\n");
}

void fprintMovesArr(FILE* ofp, uint* MovesArr, uint count, int width) {
	fprintf(ofp,"Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(ofp,", ");
		}
		uint* move = to2d(MovesArr[i], width);
		fprintf(ofp,"%c%d", alphabets[move[0]], move[1]);
	}
	fprintf(ofp," }\n");
}

void readInputs(int argc, char**argv) {
	//checking for arguments
	printf("The argument supplied to %s are", argv[0]);
	for (int i = 1; i < argc - 1; i++) {
		printf(" %s ", argv[i]);
		if (i < argc - 1) {
			printf("and");
		}
	}
	printf(". Check that the first two arguments are intitial board config and evaluation parameters respectively.\n");
	if (argc < 3) {
		printf("Too little arguments supplied, check README.\n");
		exit(1);
	}

	//Opening file inputs
	
	char *mode = "r";
	char outputFilename[30];
	char* opfp = outputFilename;

	brdtxt = fopen(argv[1], mode);
	evaltxt = fopen(argv[2], mode);

	if (brdtxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[1]);
		exit(1);
	}

	fscanf(brdtxt, "Size: %d,%d\n", &width, &height);
	fscanf(brdtxt, "White: { %s }\n", whiteStr);
	fscanf(brdtxt, "Black: { %s }\n", blackStr);

	if (evaltxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[2]);
		exit(1);
	}
	fscanf(evaltxt, "MaxDepth: %d\n", &ply);
	fscanf(evaltxt, "MaxBoards: %ld\n", &maxBoards);
	fscanf(evaltxt, "MaxPlayer: %s\n", maxPlayerStr);
	sprintf(opfp,"outSerial%dx%d.txt",width,height);
	ofp = fopen(outputFilename, "a");

	if (ofp == NULL) {
		fprintf(stderr, "Can't open output file %s!\n",
			outputFilename);
		exit(1);
	}

	if (strcmp(maxPlayerStrPtr, "WHITE")==0) {
		maxPlayer = WHITE;
	}
	else if (strcmp(maxPlayerStrPtr, "BLACK")==0) {
		maxPlayer = BLACK;
	}
	else {
		fprintf(stderr, "Can't tell who is the MaxPlayer: %s!\n", maxPlayerStr);
	}
	fseek(brdtxt, 0, SEEK_SET);
	fseek(evaltxt, 0, SEEK_SET);
	fprintf(ofp, "########################################################################################################\n");
	char ch;
	while ((ch = fgetc(brdtxt)) != EOF)		fputc(ch, ofp);
	fputc('\n', ofp);
	while ((ch = fgetc(evaltxt)) != EOF)	fputc(ch, ofp);
	fputc('\n', ofp);
	fclose(evaltxt);
	fclose(brdtxt);
}

void saveRunResult(uint* bestMovesArr, int bestMovesCount) {
	fprintMovesArr(ofp, bestMovesArr, bestMovesCount, width);
	printf("Number of boards assessed: %ld\n", numBoards);
	fprintf(ofp, "Number of boards assessed: %ld\n", numBoards);

	if (maxPlayer == BLACK) {

		fprintf(ofp, "Max Player: BLACK\n");
	}
	else {
		fprintf(ofp, "Max Player: WHITE\n");
	}
	printf("computation_time = %ld clock ticks\n", comp_time);
	fprintf(ofp, "computation_time = %ld clock ticks\n", comp_time);

	printf("computation_time = %ld mins %ld secs %ld msecs\n", comp_time / CLOCKS_PER_SEC / 60, comp_time / CLOCKS_PER_SEC % 60, comp_time * 1000 / CLOCKS_PER_SEC % 1000);
	fprintf(ofp, "computation_time = %ld mins %ld secs %ld msecs\n", comp_time / CLOCKS_PER_SEC / 60, comp_time / CLOCKS_PER_SEC % 60, comp_time * 1000 / CLOCKS_PER_SEC % 1000);
	fclose(ofp);
}

int main(int argc, char **argv) {
	readInputs(argc, argv);
	printf("Started\n");
	uint** boardPtr = createBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
	printf("Board %d x %d created\n",width,height);
	
	//printf("Game created\n");
	uint* bestMovesArr = malloc(height*width * sizeof(uint));
	before = clock();
	int bestMovesCount = solve(boardPtr, ply, bestMovesArr);
	after = clock();
	comp_time = after - before;

	saveRunResult(bestMovesArr, bestMovesCount);

	printf("Solved!\n");
	free(bestMovesArr);
	freeBoard(boardPtr);
	return 0;
}