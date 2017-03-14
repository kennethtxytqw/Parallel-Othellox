#define black 2
#define white 1
#define empty 0
#define true 1
#define false 0

typedef struct{
	int** board;
	int player;
	int height;
	int width;
}game;

function int availableMoves(game){
	for(int y=0; y<height; y++){
		for(int x=0; x<width; x++){
			if(isValidMove(x,y,board)){

			}
		}
	}
}

function int isValidMove(int x,int y,int player,int** board){
	if(board[x][y]!=2){
		return false;
	}
	int currX;
	int currY;
	for(int i=-1;i<2;i++){
		currX = i+x;
		if(currX>0 &&currX<width){
			for(int j=-1;j<2;j++){
				currY=j+y;
				if(currY>0&&currY<height){
					if(board[currX][currY]!=player){
						return true;
					}
				}
			}
		}
	}
}