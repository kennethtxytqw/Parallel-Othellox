#include "util.h"
FILE *brdtxt, *evaltxt, *ofp;

const uint UP[2] = { 0,1 };
const uint DOWN[2] = { 0,-1 };
const uint LEFT[2] = { -1,0 };
const uint RIGHT[2] = { 1,0 };
const uint UPRIGHT[2] = { 1,1 };
const uint UPLEFT[2] = { -1,1 };
const uint DOWNLEFT[2] = { -1,-1 };
const uint DOWNRIGHT[2] = { 1,-1 };

const uchar alphabets[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

const char DELIM[] = delimiters;

char maxPlayerStr[6];
char* maxPlayerStrPtr = maxPlayerStr;

void readInputs(int argc, char**argv, int* widthPtr, int* heightPtr, int* maxDepthPtr, int* maxBoardsPtr, int* maxPlayerPtr, int* abPruningPtr, char* whiteStrPtr, char* blackStrPtr, int* timeoutPtr, char* TYPE) {
	//checking for arguments
	//fprintf(stderr,"The argument supplied to %s are", argv[0]);
	for (int i = 1; i < argc - 1; i++) {
		//fprintf(stderr," %s ", argv[i]);
		if (i < argc - 1) {
			//fprintf(stderr,"and");
		}
	}
	//fprintf(stderr,". Check that the first two arguments are intitial board config and evaluation parameters respectively.\n");
	if (argc < 3) {
		fprintf(stderr, "Too little arguments supplied, check README.\n");
		exit(1);
	}

	//Opening file inputs

	char *mode = "r";
	char outputFilename[30];
	char* opfp = outputFilename;
	char abPruning[10];

	brdtxt = fopen(argv[1], mode);
	evaltxt = fopen(argv[2], mode);

	if (brdtxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[1]);
		exit(1);
	}

	fscanf(brdtxt, "Size: %d,%d\n", &(*widthPtr), &(*heightPtr));
	fscanf(brdtxt, "White: { %s }\n", &(*whiteStrPtr));
	fscanf(brdtxt, "Black: { %s }\n", &(*blackStrPtr));
	fscanf(brdtxt, "Color: %s\n", maxPlayerStr);
	fscanf(brdtxt, "Timeout: %d\n", &(*timeoutPtr));
	if (evaltxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[2]);
		exit(1);
	}
	fscanf(evaltxt, "MaxDepth: %d\n", &(*maxDepthPtr));
	fscanf(evaltxt, "MaxBoards: %d\n", &(*maxBoardsPtr));
	fscanf(evaltxt, "abPruning: %s\n", abPruning);
	sprintf(opfp, "out%s%dx%d.txt", TYPE, *widthPtr, *heightPtr);
	ofp = fopen(outputFilename, "a");

	if (ofp == NULL) {
		fprintf(stderr, "Can't open output file %s!\n",
			outputFilename);
		exit(1);
	}

	if (strcmp(maxPlayerStrPtr, "WHITE") == 0) {
		*maxPlayerPtr = WHITE;
	}
	else if (strcmp(maxPlayerStrPtr, "BLACK") == 0) {
		*maxPlayerPtr = BLACK;
	}
	else {
		fprintf(stderr, "Can't tell who is the MaxPlayer: %s!\n", maxPlayerStr);
	}

	if (strcmp(abPruning, "TRUE") == 0) {
		*abPruningPtr = 1;
	}
	else {
		*abPruningPtr = 0;
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

void saveRunResult(uint* bestMovesArr, int bestMovesCount, int width, int numBoards, long long comp_time, long long comm_time, long long total_time, int numP) {
	fprintMovesArr(ofp, bestMovesArr, bestMovesCount, width);
	fprintf(ofp, "Number of processor: %d\n", numP);
	fprintf(stderr, "Number of boards assessed: %d\n", numBoards);
	fprintf(ofp, "Number of boards assessed: %d\n", numBoards);

	fprintf(stderr,"comm_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "communication_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "avg communication_time = %lld msecs\n", comm_time / numP * 1000 / 1000000000);

	fprintf(stderr,"comp_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "avg computation_time = %lld msecs\n", comp_time / numP * 1000 / 1000000000);
	fprintf(ofp, "computation_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	fprintf(stderr, "total_time = %lld mins %lld secs %lld msecs\n", total_time / 1000000000 / 60, total_time / 1000000000 % 60, total_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "total_time = %lld mins %lld secs %lld msecs\n", total_time / 1000000000 / 60, total_time / 1000000000 % 60, total_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "total_time = %lld msecs\n", total_time * 1000 / 1000000000);

	fclose(ofp);
}

void printBoard(uint** boardPtr, int width, int height) {
	char* str = malloc(sizeof(char)*(height*(width + 1)+1));
	int count = 0;
	for (int i = height - 1; i >= 0; i--) {
		for (int j = 0; j < width; j++) {
			if (boardPtr[j][i] == BLACK) {
				str[count] = 'B';
			}
			else if (boardPtr[j][i] == WHITE) {
				str[count] = 'W';
			}
			else if (boardPtr[j][i] == 3) {
				str[count] = 'D';
			}
			else {
				str[count] = '-';
			}
			count++;
		}
		str[count] = '\n';
		count++;
	}
	str[count] = '\0';
	fprintf(stderr, "%s",str);
	free(str);
}

void printMovesArr(uint* MovesArr, uint count, int width) {
	fprintf(stdout, "Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(stdout, ", ");
		}
		uint* move = to2d(MovesArr[i], width);
		// move[1] + 1 because in the real world the board start from 1 and not 0
		fprintf(stdout, "%c%d", alphabets[move[0]], 1 + move[1]);
	}
	fprintf(stdout, " }\n");
}

void fprintMovesArr(FILE* ofp, uint* MovesArr, uint count, int width) {
	fprintf(ofp, "Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(ofp, ", ");
		}
		uint* move = to2d(MovesArr[i], width);
		fprintf(ofp, "%c%d", alphabets[move[0]], move[1] + 1);
	}
	fprintf(ofp, " }\n");
}

char* mystrsep(char** stringp)
{
	char* start = *stringp;
	char* p;
	p = (start != NULL) ? strpbrk(start, DELIM) : NULL;
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
	uint* iarr = malloc(sizeof(uint) * 2);
	uint x = i%width;
	uint y = i / width;
	iarr[0] = x;
	iarr[1] = y;
	return iarr;
}

int to1d(int x, int y, int width) {
	return y*width + x;
}

long long wall_clock_time()
{
#ifdef LINUX
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}