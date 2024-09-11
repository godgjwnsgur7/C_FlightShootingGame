#pragma once
#define SetIngameSizeStr "mode con:cols=82 lines=36 | title ������ - �ܼ� ���� ����"
#define horizon 80

#pragma region GameLogic
// GameInfo
typedef struct _GAMEINFO
{
	int currStageId;
	int currScore;
	int maxScore;

	int invincibleTime;
	int limitSpeed;
	int limitAttackSpeed;
	int limitLife;
	int limitPower;

	char playerBodyChars[4];
	char enemyBodyChars[11];
	char playerBulletChar;
	char enemyBulletChars[4];
	char itemChars[6];
}GAMEINFO;
GAMEINFO gameInfoData;
DWORD playerReviveTimer;
DWORD playerInvincibleTimer;

// Window
typedef enum EWindowType
{
	NoneWindow, TitleWindow, InGameWindow, ResultWindow
};
enum EWindowType currWindowType;
#pragma endregion

#pragma region Stage
int currSpawnCount;
int enemyRemoveCount;
int maxSpawnCount;

int enemySpawnTick;
int enemySpawnDelay;

bool isWaitNextStage;
int waitNextStageTime;
int currWaitStageTick;
#pragma endregion

#pragma region Player
typedef struct _PLAYER
{
	int life;
	int power;
	bool isDie;
	bool isInvincible;

	int x, y;
	int damage;
	int speed, bulletSpeed, attackSpeed;
	int shapeSize;
	char shapeStr[6];
}PLAYER;

PLAYER player;

typedef enum EPlayerActionType
{
	Left, Right, Up, Down, Attack, Hit
}PlayerActionType;
#pragma endregion

#pragma region Item
typedef enum EItemType
{
	PowerItem= 0, LifeItem = 1, SpeedItem = 2, MaxItemType = 3,
}ItemType;
typedef struct _ITEM
{
	enum EItemType itemType;
	int x, y;
	int speed;
	int moveTick;
	int shapeSize;
	char shapeStr[4];
	bool isDirRight;
	bool isDirDown;
	bool isUsing;
}ITEM;
#pragma endregion

#pragma region Enemy
typedef enum EEnemyType
{
	Enemy_S = 0, Enemy_M = 1, Enemy_L = 2, MaxEnemyType = 3
};
typedef struct _ENEMY
{
	enum EEnemyType enemyType;
	int x, y;
	int hp;
	int point;
	int speed, bulletSpeed, attackSpeed;
	int moveTick, attackTick, hitTick;
	int shapeSize;
	char shapeStr[6];
	char bulletShapeCh;
	bool isHit;
	bool isUsing;
}ENEMY;
#pragma endregion

#pragma region Bullet
typedef struct _BULLET
{
	int x, y;
	int speed;
	int moveTick;
	char shapeCh;
	bool isUsing;
}BULLET;

typedef struct _TILTBULLET
{
	int x, y;
	int speed;
	int moveTick;
	char shapeCh;
	bool isRight;
	bool isUsing;
}TILTBULLET;
#pragma endregion

#pragma region Pool
struct _ENEMY enemyPool[100];
struct _BULLET playerBulletPool[100];
struct _BULLET enemyBulletPool[100];
struct _TILTBULLET enemyTiltBulletPool[100];
struct _ITEM itemPool[3];
#pragma endregion

#pragma region Input
int g_leftArrowKeyPressTick;
int g_rightArrowKeyPressTick;
int g_upArrowKeyPressTick;
int g_downArrowKeyPressTick;
int g_spaceKeyPressTick; // Attack
#pragma endregion

#pragma region IngameUI
char ingameUIStr[horizon + 2];
char playerInfoUIStr[horizon + 2];
#pragma endregion

#pragma region Screen
int g_nScreenIndex;
HANDLE g_hScreen[2];
#pragma endregion

#pragma region Util
typedef enum EObjectType
{
	None, Player, Enemy, PlayerBullet, EnemyBullet, Item
};

enum ColorType
{
	BLACK,  	//0
	darkBLUE,	//1
	DarkGreen,	//2
	darkSkyBlue,//3
	DarkRed,  	//4
	DarkPurple,	//5
	DarkYellow,	//6
	GRAY,		//7	
	DarkGray,	//8
	BLUE,		//9 
	GREEN,		//10 
	SkyBlue,	//11 
	RED,		//12 
	PURPLE,		//13
	YELLOW,		//14 
	WHITE		//15 
};
#pragma endregion

#pragma region DrawData
void PrintTitleWindow()
{
	system("cls");
	printf("\n\n\n\n\n\n\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), SkyBlue);
	printf("                           �ö���Ʈ ���� ����\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
	printf("                           �̵�) ���� Ű\n\n");
	printf("                           ����) �����̽� ��\n\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);
	printf("                           Tip ) ������ ������ �ִ� �� ����\n\n");
	printf("                           Tip ) �������� �ʵ忡 �ִ� 3������ ����\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW);
	printf("                           ������ �����Ϸ��� 's'Ű\n\n");
	printf("                           ������ �׸��Ͻ÷��� 'Ese'Ű\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n");
}

void PrintResultWindow(int score, int maxScore)
{
	system("cls");
	printf("\n\n\n\n\n\n\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), SkyBlue);
	printf("                           ���â\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
	printf("                           ������ �������� : %d\n\n", gameInfoData.currStageId);
	printf("                           ���� : %d\n\n", gameInfoData.currScore);
	printf("                           �ְ� ���� : %d\n\n", gameInfoData.maxScore);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW);
	printf("                           Ÿ��Ʋ�� ���÷��� 'n'Ű\n\n");
	printf("                           ������ �׸��Ͻ÷��� 'Ese'Ű\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN);
	printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n");
}
#pragma endregion
