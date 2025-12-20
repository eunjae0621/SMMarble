//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"


//board configuration parameters
static int smm_board_nr;
static int smm_food_nr;
static int smm_festival_nr;
static int smm_player_nr;

typedef struct {
	char name[MAX_CHARNAME];
	int pos;
	int energy;
	int credit;
	int flag_graduated;
	int isExperimenting;
	int expThreshold;
} smm_player_t;

smm_player_t *smm_players;

void generatePlayers(int n, int initEnergy); //generate a new player
void printPlayerStatus(void); //print all player status at the beginning of each turn
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
int isGraduated(void); //check if any player is graduated
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print grade history of the player

//function prototypes
#if 0
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void printGrades(int player); //print all the grade history of the player
#endif

void printGrades(int player)
{
	int size = smmdb_len(LISTNO_OFFSET_GRADE + player); //플레이어가 가진 성적 리스트 길이 가져오기 
	int i;
	
	//플레이어 이름과 함께 성적 리스트 헤더 출력 
	printf("===Grade history of %s===\n", smm_players[player].name);
	
	//성적 리스트에 저장된 강의 순서대로 하나씩 출력
	for (i=0; i<size; i++) {
		void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
		
		char *lecName = smmObj_getObjectName(ptr);
		int credit = smmObj_getObjectCredit(ptr);
		int grade = smmObj_getObjectGrade(ptr);
		
		printf("%d) %s, credit %d, grade %d\n", i, lecName, credit, grade);
	} 
}

void* findGrade(int player, char *lectureName)
{
	int size = smmdb_len(LISTNO_OFFSET_GRADE+player);
	int i;

	for (i=0; i<size; i++)
	{
		void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);
		if (strcmp(smmObj_getObjectName(ptr), lectureName) == 0)
		{
			return ptr;
		}
	}
	
	return NULL;
}

int isGraduated(void)
{
	int i;
	for (i=0; i<smm_player_nr; i++)
	{
		if (smm_players[i].flag_graduated == 1)
			return 1;
	}
	return 0;
}

void goForward(int player, int step)
{
	int i;
	void *ptr;
	
	//player_pos[player] = player_pos[player] + step;
	ptr = smmdb_getData(LISTNO_NODE,smm_players[player].pos);
	printf("start from %i(%s) (%i)\n", smm_players[player].pos, smmObj_getObjectName(ptr), step);
	
	for (i=0; i<step; i++) {
		smm_players[player].pos = (smm_players[player].pos + 1)%smm_board_nr;
		
		ptr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
		printf(" => moved to %i(%s)\n", smm_players[player].pos, smmObj_getObjectName(ptr));
	}
}

void printPlayerStatus(void)
{
	int i;
	for	(i=0; i<smm_player_nr; i++) {
		void *nodePtr = smmdb_getData(LISTNO_NODE, smm_players[i].pos);
		
		printf("%s - position : %i(%s), credit : %i, energy : %i\n", smm_players[i].name, smm_players[i].pos, smmObj_getObjectName(nodePtr), smm_players[i].credit, smm_players[i].energy);	
	}
}

void generatePlayers(int n, int initEnergy) //generate a new player
{	
	int i;
	
	smm_players = (smm_player_t*)malloc(n*sizeof(smm_player_t));
	
	for	(i=0; i<n; i++) 
	{
		smm_players[i].pos = 0;
		smm_players[i].credit = 0;
		smm_players[i].energy = initEnergy;
		smm_players[i].flag_graduated = 0;
		smm_players[i].isExperimenting = 0;
		smm_players[i].expThreshold = 0;
		
		printf("Input %i-th player name : ", i);
		scanf ("%s", &smm_players[i].name);
		fflush(stdin);		
	}
}

int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);
    
#if 0
    if (c == 'g')
        printGrades(player);
#endif    
    return (rand()%MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
	void *ptr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
	int type = smmObj_getObjectType(ptr);
	int credit = smmObj_getObjectCredit(ptr);
    int	energy = smmObj_getObjectEnergy(ptr);
    int grade;
	void *gradePtr;
	
    switch(type)
    {
		case SMMNODE_TYPE_LECTURE:
    	{
			//이미 수강한 강의면 바로 종료 
    		if (findGrade(player, smmObj_getObjectName(ptr)) != NULL ) {
    			printf("%s : already took lecture %s.\n", smm_players[player].name, smmObj_getObjectName(ptr));
    			break;
			}
			
			//에너지 부족하면 수강 불가 
			if (smm_players[player].energy < energy) {
				printf("%s : not enough energy to take %s.\n", smm_players[player].name, smmObj_getObjectName(ptr));
				break;
			} 
			
			//수강, 드랍 선택 
			char choice;
			
			printf("%s : lecture %s (credit %d, energy %d)\n", smm_players[player].name, smmObj_getObjectName(ptr), credit, energy);
			printf("-> take(t) or drop(d)? : ");
			scanf(" c", &choice);
		
			if (choice == 't' || choice == 'T') {
				smm_players[player].credit += credit;
				smm_players[player].energy -= energy;
				
				printf("%s : took lecture %s, credit +%d -> %d, energy -%d -> %d\n", smm_players[player].name, smmObj_getObjectName(ptr), credit, smm_players[player].credit, energy, smm_players[player].energy);
				
				grade = rand()%SMMNODE_MAX_GRADE;
    			gradePtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, grade);
    			smmdb_addTail(LISTNO_OFFSET_GRADE+player, gradePtr);
    				
			} else {
				printf("%s : dropped lectrue %s\n", smm_players[player].name, smmObj_getObjectName(ptr));
			}

    		break;
		}	
    	
		//보충 에너지만큼 에너지 회복	 		
		case SMMNODE_TYPE_RESTAURANT:
			smm_players[player].energy += energy;
			printf("%s : ate food, energy +%d -> %d\n", smm_players[player].name, energy, smm_players[player].energy);
			break;
						
		case SMMNODE_TYPE_LABORATORY:
		{
			//실험중 상태가 아니라면 아무 일도 일어나지 않음
			if (smm_players[player].isExperimenting == 0) {
				printf("%s : at laboratory, but no experiment in progress\n", smm_players[player].name);
				break;
			} 
			
			//실험 시도마다 소요 에너지만큼 감소
			smm_players[player].energy -= energy; //energy = 실험 소요 에너지
			printf("%s : experiment try, energy -%d -> %d\n", smm_players[player].name, energy, smm_players[player].energy);
			
			//주사위를 굴려서 성공 기준값과 비교
			int die = (rand()%MAX_DIE)+1;
			printf("rolled die = %d (threshold = %d)\n", die, smm_players[player].expThreshold);
			
			//기준값 이상이면 실험 성공 -> 실험 종료, 상태 해제/ 기준값 미만이면 실패 -> 실험실에 머묾 
			if (die >= smm_players[player].expThreshold) {
				smm_players[player].isExperimenting = 0;
				smm_players[player].expThreshold = 0;
				
				printf("%s : experiment success! experiment finished\n", smm_players[player].name);
			} else {
				printf("%s : experiment failed. stay in laboratory\n", smm_players[player].name);
			} 
		
			break;
		}
		
		//집에 도착하면 보충 에너지만큼 에너지 회복 
		case SMMNODE_TYPE_HOME:	
			smm_players[player].energy += energy;
			printf("%s : at home, energy +%d -> %d\n", smm_players[player].name, energy, smm_players[player].energy);
			
			//GRADUATE_CREDIT 이상 학점 채우고 집에 도착하면 졸업 
			if (smm_players[player].credit >= GRADUATE_CREDIT)
			{
				smm_players[player].flag_graduated = 1;
				printf("%s : graduated with %d credits!\n", smm_players[player].name, smm_players[player].credit);
			}
			break;	
			
		case SMMNODE_TYPE_GOTOLAB:	
		{
			//이미 실험중이면 다시 설정하지 않음
			if (smm_players[player].isExperimenting == 1) {
				printf("%s : already experimenting\n", smm_players[player].name);
				break;
			} 
			
			//실험 중 상태로 전환
			smm_players[player].isExperimenting = 1;
			
			//실험 성공 기준값 랜덤 지정 
			smm_players[player].expThreshold = (rand()%MAX_DIE)+1;
			
			//보드에서 실험실 노드 위치로 이동
			int i, labIndex = smm_players[player].pos;
			int boardLen = smmdb_len(LISTNO_NODE);
			
			for (i=0; i<boardLen; i++) {
				void *nodePtr = smmdb_getData(LISTNO_NODE, i);
				if (smmObj_getObjectType(nodePtr) == SMMNODE_TYPE_LABORATORY) {
					labIndex = i;
					break;
				}
			}
		
			smm_players[player].pos = labIndex;
			
			printf("%s : experiment started! moved to lab(pos %d), threshold = %d\n", smm_players[player].name, smm_players[player].pos, smm_players[player].expThreshold);
			break;
		}
					
		case SMMNODE_TYPE_FOODCHANCE:
		{
			int len = smmdb_len(LISTNO_FOODCARD);
			if (len<=0) {
				printf("%s : no food cards\n", smm_players[player].name);
				break;
			}
			
			//랜덤  카드 선택
			int idx = rand()%len;
			void *foodPtr = smmdb_getData(LISTNO_FOODCARD, idx);
			
			int foodEnergy = smmObj_getObjectEnergy(foodPtr); //카드 에너지 
			char *foodName = smmObj_getObjectName(foodPtr); //카드 이름 
			
			smm_players[player].energy += foodEnergy; //에너지 보충
			
			printf("%s : drew food card [%s], energy +%d -> %d\n", smm_players[player].name, foodName, foodEnergy, smm_players[player].energy); 
			break;
		}
							
		case SMMNODE_TYPE_FESTIVAL:		
		{
			int len = smmdb_len(LISTNO_FESTCARD);
			if (len<=0) {
				printf("%s : no festival cards\n", smm_players[player].name);
				break;	
			}
			
			//랜덤 카드 선택
			int idx = rand()%len;
			void *festPtr = smmdb_getData(LISTNO_FESTCARD, idx);
			char *festName = smmObj_getObjectName(festPtr); //미션 문자열 
			
			printf("%s : festival mission [%s] (mission logic TODO)\n", smm_players[player].name, festName); 
			break;
		}

        default:
            break;
    }
}


int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int cnt;
    int turn;
    
    smm_board_nr = 0;
    smm_food_nr = 0;
    smm_festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        void* ptr;
        printf("%s %i %i %i\n", name, type, credit, energy);
        ptr = smmObj_genObject(name,  SMMNODE_OBJTYPE_BOARD, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, ptr);
    }
    fclose(fp);
    
    smm_board_nr = smmdb_len(LISTNO_NODE);
    printf("Total number of board nodes : %i\n", smm_board_nr);
    
    

    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %d", name, &energy) == 2) //read a food parameter set
    {
    	//store the parameter set
    	void *ptr;
    	printf("%s %d\n", name, energy);
    	
		//objType = FOOD (type, credit, grade는 0) 
		ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_FOOD, 0, 0, energy, 0);
		smmdb_addTail(LISTNO_FOODCARD, ptr);	    
    }
    fclose(fp);
 
	smm_food_nr = smmdb_len(LISTNO_FOODCARD);
    printf("Total number of food cards : %i\n", smm_food_nr);
  
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", name) == 1) //read a festival card string
    {
        //store the parameter set
        void *ptr;
        printf("%s\n", name);
        
        //objType = FEST name : 카드 이름 (type, credit, energy, grade는 사용하지 않으므로 0으로 초기화) 
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_FEST, 0, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTCARD, ptr);
    }
    fclose(fp);
    
    smm_festival_nr = smmdb_len(LISTNO_FESTCARD);
    printf("Total number of festival cards : %i\n", smm_festival_nr);
      
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("Input player number : ");
		scanf("%i", &smm_player_nr);
        fflush(stdin);
        
        if (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER)
        	printf("Invalid player number!\n");
    }
    while (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER);
    
    void *homePtr = smmdb_getData(LISTNO_NODE, 0);
    generatePlayers(smm_player_nr, smmObj_getObjectEnergy(homePtr));

    turn = 0;
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (isGraduated() ==0) //is anybody graduated?
    {
        int die_result;

        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
		//pos = pos + 2;
		
		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn       
		turn = (turn + 1)%smm_player_nr;
    }
    
    //게임 종료 (졸업한 플레이어 성적 출력)
	{
		int i;
		for (i=0; i<smm_player_nr; i++) {
			if (smm_players[i].flag_graduated == 1) {
				printf("\n===%s graduated with %d credits===\n", smm_players[i].name, smm_players[i].credit);
				
				//수강한 강의 이름,학점,성적 전체 출력
				printGrades(i);
				break; 
			}
		} 
	 } 
	
	free(smm_players);
    system("PAUSE");
	return 0;
}
