#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ncurses.h>
#include <string.h>

#define ROWS 20
#define COLS 15
#define TRUE 1
#define FALSE 0

char Table[ROWS][COLS] = {0};
int score = 0;
int high_score = 0;
char GameOn = TRUE;

int decrease = 1000;
FILE *fp;
int highscore;
int user_input;

typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

const Shape ShapesArray[7]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                               
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                                
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                                
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                               
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                               
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} 
};

Shape CopyShape(Shape shape){
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
    int i, j;
    for(i = 0; i < new_shape.width; i++){
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for(j=0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
		}
    }
    return new_shape;
}

void DeleteShape(Shape shape){
    int i;
    for(i = 0; i < shape.width; i++){
		free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape){
	char **array = shape.array;
	int i, j;
	for(i = 0; i < shape.width;i++) {
		for(j = 0; j < shape.width ;j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){
				if(array[i][j])
					return FALSE;
				
			}
			else if(Table[shape.row+i][shape.col+j] && array[i][j])
				return FALSE;
		}
	}
	return TRUE;
}

void SetNewRandomShape(){ 
	Shape new_shape = CopyShape(ShapesArray[rand()%7]);

    new_shape.col = rand()%(COLS-new_shape.width+1);
    new_shape.row = 0;
    DeleteShape(current);
	current = new_shape;
	if(!CheckPosition(current)){
		GameOn = FALSE;
	}
}

void RotateShape(Shape shape){
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for(i = 0; i < width ; i++){
		for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable(){
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore(suseconds_t timer){
	int i, j, sum, count=0;
	for(i=0;i<ROWS;i++){
		sum = 0;
		for(j=0;j< COLS;j++) {
			sum+=Table[i][j];
		}
		if(sum==COLS){
			count++;
			int l, k;
			for(k = i;k >=1;k--)
				for(l=0;l<COLS;l++)
					Table[k][l]=Table[k-1][l];
			for(l=0;l<COLS;l++)
				Table[k][l]=0;
			timer-=decrease--;
		}
	}
	if(user_input==1)score += 100*count;
	else if(user_input==2)score += 200*count;
	else if(user_input==3)score += 300*count;
}

void PrintTable(){
	char Buffer[ROWS][COLS] = {0};
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}
	clear();
	for(i=0; i<COLS-9; i++)
		printw(" ");
	printw("    TETRIS\n");
	
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			
			printw("%c ", (Table[i][j] + Buffer[i][j])? '#': '.');
			
		}
		if(i==10)printw("    s to move down");
		else if(i==9)printw("    a to move left");
		else if(i==8)printw("    d to move right");
		else if(i==7)printw("    w to rotate");
		else if(i==13)printw("    q to quit");
		printw("\n");
	}
        fp = fopen("score.txt","r");
        fscanf(fp,"%d",&highscore);
	printw("\nScore: %d\n", score);
	printw("High Score: %d\n", highscore);
        fclose(fp);
}

void ManipulateCurrent(int action,suseconds_t timer){
	Shape temp = CopyShape(current);
	switch(action){
		case 's':
			temp.row++;
			if(CheckPosition(temp))
				current.row++;
			else {
				WriteToTable();
				RemoveFullRowsAndUpdateScore(timer);
                SetNewRandomShape();
			}
			break;
		case 'd':
			temp.col++; 
			if(CheckPosition(temp))
				current.col++;
			break;
		case 'a':
			temp.col--;
			if(CheckPosition(temp))
				current.col--;
			break;
		case 'w':
			RotateShape(temp);
			if(CheckPosition(temp))
				RotateShape(current);
			break;
		case 'q':
			GameOn=FALSE;
			break;
		
	}
	DeleteShape(temp);
	PrintTable();
}
struct timeval before_now, now;
int hasToUpdate(suseconds_t timer){
	return ((suseconds_t)(now.tv_sec*1000000 + now.tv_usec) -((suseconds_t)before_now.tv_sec*1000000 + before_now.tv_usec)) > timer;
}

void art()
{
puts( 
 "\n"
 "  _______ ______ _______ _____     _____  _____  \n"
 " |__   __|  ____|__   __|  __ \\  |_   _|/ ____| \n"
 "    | |  | |__     | |  | |__) |   | |  | (___   \n"
 "    | |  |  __|    | |  |  _  /    | |  \\___ \\  \n"
 "    | |  | |____   | |  | | \\ \\   _| |_ ____) | \n"
 "    |_|  |______|  |_|  |_|  \\_\\ |_____|_____/  \n"
 "                                              \n"); 
}

int main() {
	score = 0;
	int c; 
	suseconds_t timer;
        art();
	printf("\nSELECT DIFFICULTY\n");
	printf("1) Beginner (100 points)\n");
	printf("2) Proficient (200 points)\n");
	printf("3) Expert (300 points)\n");
	scanf("%d",&user_input);    
   	if(user_input==1)timer=400000;
	else if(user_input==2)timer=150000;
	else if(user_input==3)timer=95000;
        initscr();
	gettimeofday(&before_now, NULL);
	timeout(1);	
	SetNewRandomShape();
	while(GameOn){
		if ((c = getch()) != ERR) {
		  ManipulateCurrent(c,timer);
		}
		gettimeofday(&now, NULL);
		
		if (hasToUpdate(timer)) {
			ManipulateCurrent('s',timer);
			gettimeofday(&before_now, NULL);
		}
	}
	DeleteShape(current);
	endwin();
	int i, j;
	
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			printf("%c ", Table[i][j] ? '#': '.');
		}
		printf("\n");
	}
	
	printf("\nGame over!!!\n");
	printf("\nScore: %d\n", score);
	if(score > highscore)
        {
        fp = fopen("score.txt","w");
        fprintf(fp,"%d",score);
	printf("You have beaten the high score!!\n");
        printf("New highscore: %d\n",score);
        fclose(fp);
        }
        else{ 
        printf("Highscore: %d\n",highscore);}

    return 0;
}
