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



typedef struct {
	uint** board; //Using uchar because it is the smallest suitable data type 
	uint player;
	uint height;
	uint width;
	uint maxPlayer;
}Game;

unsigned int * to2d(unsigned int i, int width);
unsigned int c_to_n(char c);
char * mystrsep(char ** stringp, const char * delim);
int countFlips(int x, int y, Game * gptr);
Game * newGame(uint player, uint height, uint width, uint ** board);
uint ** createStandardBoard();
uint ** createBoard(int width, int height, int player, char * whiteStrPtr, char * blackStrPtr);
int playMax(Game * gptr, int ply);
int playMin(Game * gptr, int ply);
int evaluate(Game * gptr);
Game * makeMove(Game * gptr, uint nextMove);
void placePiece(Game * gptr, uint player, int x, int y);
Game * duplicateGame(Game * gptr);
void freeGame(Game * gptr);
void printMovesArr(uint * MovesArr, uint count, int width);

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
	uint* iarr = (uint*)malloc(sizeof(uint)*2);
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

int availableMoves(Game* gptr, int* movesPtr){
	int i = 0;
	for(int y=0; y<gptr->height; y++){
		for(int x=0; x<gptr->width; x++){
			if(countFlips(x,y,gptr)>0){
				movesPtr[i]=to1d(x,y,gptr->width);
				i++;
				//printf("%c%d is a valid move\n", alphabets[x],y);
			}
		}
	}
	return i;
}

int countFlips(int x, int y, Game* gptr){
	// if the box is not EMPTY then return 0 flips which is an invalid move
	if (gptr->board[x][y] != EMPTY) {
		return 0;
	}
	int flips=0;
	int counter=0;
	uint** board = gptr->board;
	
	// Count right flips
	for(int i=x+1;i<gptr->width;++i){
		if(board[i][y]==gptr->player){
			flips += counter;
			break;
		}
		if(board[i][y]==EMPTY){
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, board[i][y] , gptr->player);
		counter++;
	}
	counter=0;

	//Count left flips
	for(int i=x-1;i>=0;--i){
		if(board[i][y]==gptr->player){
			flips += counter;
			break;
		}
		if(board[i][y]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count top flips
	for(int j=y+1;j<gptr->height;j++){
		if(board[x][j]==gptr->player){
			flips += counter;
			break;
		}
		if(board[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count bottom flips
	for(int j=y-1;j>=0;j--){
		if(board[x][j]==gptr->player){
			flips += counter;
			break;
		}
		if(board[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top right flips
	for (int i = 1; i + x < gptr->width && i + y < gptr->height; i++) {
		if (board[x + i][y + i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom left flips
	for (int i = 1; x-i >=0 && y-i >=0; i++) {
		if (board[x - i][y - i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top left flips
	for (int i = 1; x-i >= 0 && i + y <gptr->height; i++) {
		if (board[x - i][y + i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom right flips
	for (int i = 1; x + i <gptr->width && y-i >= 0; i++) {
		if (board[x + i][y - i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x + i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;
	//printf("checking %d-%d has %d\n", x,y,flips);
	return flips;
}

Game* newGame(uint player, uint height, uint width, uint** board){
	Game* gptr;
	gptr = (Game*) malloc(sizeof(Game));
	gptr->board = board; //(uint**) malloc(sizeof(uint)*height*width);
	gptr->height = height;
	gptr->width = width;
	gptr->player = player;
	gptr->maxPlayer = player;
	return gptr;
} 


uint** createStandardBoard(){
	printf("Starting board creation\n");
	uint** board = (uint**) malloc(sizeof(uint*)*SSIZE);
	for(int i =0; i<SSIZE; i++){
		board[i]= (uint*) malloc(sizeof(uint)*SSIZE);
	}
	for (int i = 0; i < SSIZE; i++) {
		for (int j = 0; j < SSIZE; j++) {
			board[i][j] = 0;
		}
	}
	board[4][4]= WHITE;
	board[5][5]=WHITE;
	board[5][4]=BLACK;
	board[4][5]=BLACK;
	return board;
}

uint** createBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	printf("Starting board creation\n");
	char* token;
	//This way i make sure i create my board such that the data is contiguous
	uint** boardPtr = (uint**)malloc(sizeof(uint*)*width);
	uint* board = (uint*)malloc(sizeof(uint)*height*width);
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

int solve(Game* gptr, int ply, uint* bestMovesArr) {
	if (ply == 0) {
		printf( "ply == 0");
		return 0;
	}
	int value = -INT_MAX;
	uint* movesArr = (uint*)malloc(gptr->height*gptr->width * sizeof(uint));
	
	int numMoves = availableMoves(gptr, movesArr);
	if (numMoves == 0) {
		printf("No available moves");
		return 0;
	}
	int temp;
	int bestMovesCount;
	for (int i = 0; i < numMoves; i++) {
		Game* nextGame = makeMove(gptr, movesArr[i]);
		temp = playMin(nextGame, ply - 1);
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
		freeGame(nextGame);
	}
	free(movesArr);
	return bestMovesCount;
}

int playMax(Game* gptr, int ply) {
	if (ply == 0||numBoards==maxBoards) {
		return evaluate(gptr);
	}
	else {
		int value = -INT_MAX;
		uint* movesArr = (uint*)malloc(gptr->height*gptr->width*sizeof(uint));
		int numMoves = availableMoves(gptr, movesArr);
		if (numMoves == 0) {
			gptr->player = gptr->player++ % 2;
			return playMin(gptr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			Game* nextGame = makeMove(gptr, movesArr[i]);
			temp = playMin(nextGame, ply - 1);
			if (temp > value) {
				value = temp;
			}
			freeGame(nextGame);
		}
		//printf("Done Max\n");
		free(movesArr);
		return value;
	}
}

int playMin(Game* gptr, int ply) {
	if (ply == 0 || numBoards == maxBoards) {
		return evaluate(gptr);
	}
	else {
		int value = INT_MAX;
		uint* movesArr = (uint*)malloc(gptr->height*gptr->width * sizeof(uint));
		int numMoves = availableMoves(gptr, movesArr);
		if (numMoves == 0) {
			gptr->player = gptr->player++%2;
			return playMax(gptr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			Game* nextGame = makeMove(gptr, movesArr[i]);
			temp = playMax(nextGame, ply - 1);
			if (temp < value) {
				value = temp;
			}
			freeGame(nextGame);
		}
		//printf("Done Min\n");
		free(movesArr);
		return value;
	}
}

int evaluate(Game* gptr) {
	int score = 0;
	for (int i = 0; i < gptr->width; i++) {
		for (int j = 0; j < gptr->height; j++) {
			if (gptr->board[i][j] == gptr->maxPlayer) {
				score++;
			}
			//printf("%d-%d is %d\n", i, j, gptr->board[i][j]);
		}
	}

	return score;
}

Game* makeMove(Game* gptr, uint nextMove) {
	uint* move = to2d(nextMove, gptr->width);
	uint x = move[0];
	uint y = move[1];
	Game* ngptr = duplicateGame(gptr);
	if (gptr->player == BLACK) {
		
		placePiece(ngptr, BLACK, x, y);
		ngptr->player = WHITE;
	}
	else {
		
		placePiece(ngptr, WHITE, x, y);
		ngptr->player = BLACK;
	}
	numBoards++;
	free(move);
	return ngptr;
}

void placePiece(Game* gptr, uint player, int x, int y) {
	uint** board = gptr->board;
	int counter = 0;
	for (int i = x + 1; i<gptr->width; ++i) {
		if (board[i][y] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x+j][y] = gptr->player;
			}
			break;
		}
		if (board[i][y] == EMPTY) {
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, board[i][y] , gptr->player);
		counter++;
	}
	counter = 0;

	//left flips
	for (int i = x - 1; i >= 0; --i) {
		if (board[i][y] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x-j][y] = gptr->player;
			}
			break;
		}
		if (board[i][y] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top flips
	for (int i = y + 1; i<gptr->height; i++) {
		if (board[x][i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x][y+j] = gptr->player;
			}
			break;
		}
		if (board[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom flips
	for (int i = y - 1; i >= 0; i--) {
		if (board[x][i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x][y - j] = gptr->player;
			}
			break;
		}
		if (board[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top right flips
	for (int i = 1; i + x < gptr->width && i + y < gptr->height; i++) {
		if (board[x + i][y + i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x +j][y + j] = gptr->player;
			}
			break;
		}
		if (board[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom left flips
	for (int i = 1; x - i >= 0 && y - i >= 0; i++) {
		if (board[x - i][y - i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x  - j][y - j] = gptr->player;
			}
			break;
		}
		if (board[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top left flips
	for (int i = 1; x - i >= 0 && i + y <gptr->height; i++) {
		if (board[x - i][y + i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x  - j][y + j] = gptr->player;
			}
			break;
		}
		if (board[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom right flips
	for (int i = 1; x + i <gptr->width && y - i >= 0; i++) {
		if (board[x + i][y - i] == gptr->player) {
			for (int j = 0; j <= counter; j++) {
				board[x  + j][y - j] = gptr->player;
			}
			break;
		}
		if (board[x + i][y - i] == EMPTY) {
			break;
		}
	}
}
Game* duplicateGame(Game* gptr) {
	Game* ngptr;
	uint** boardPtr = (uint**)malloc(sizeof(uint*)*gptr->width);
	uint* board = (uint*)malloc(sizeof(uint)* gptr->height*gptr->width);
	for (int i = 0; i<gptr->width; i++) {
		boardPtr[i] = &board[i*gptr->height];
	}
	for (int i = 0; i < gptr->width; i++) {
		for (int j = 0; j < gptr->height; j++) {
			boardPtr[i][j] = gptr->board[i][j];
		}
	}
	ngptr = (Game*)malloc(sizeof(Game));
	ngptr->board = boardPtr; //(uint**) malloc(sizeof(uint)*height*width);
	ngptr->height = gptr->height;
	ngptr->width = gptr->width;
	ngptr->player = gptr->player;
	ngptr->maxPlayer = gptr->maxPlayer;
	return ngptr;
}

void freeGame(Game* gptr) {
	free(gptr->board[0]);
	free(gptr->board);
	free(gptr);
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
	if (maxPlayerStrPtr = "WHITE") {
		maxPlayer = WHITE;
	}
	else if (maxPlayerStrPtr = "BLACK") {
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
	uint** board = createBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
	printf("Board %d x %d created\n",width,height);
	
	//printf("checking if placed at %d\n", board[4][4]);
	Game* gptr = newGame(maxPlayer , width , height, board);
	//printf("Game created\n");
	uint* bestMovesArr = (uint*)malloc(gptr->height*gptr->width * sizeof(uint));
	before = clock();
	int bestMovesCount = solve(gptr, ply, bestMovesArr);
	after = clock();
	comp_time = after - before;

	saveRunResult(bestMovesArr, bestMovesCount);

	printf("Solved!\n");
	free(bestMovesArr);
	freeGame(gptr);
	return 0;
}