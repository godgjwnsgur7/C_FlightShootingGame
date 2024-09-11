#define _CRT_SECURE_NO_WARNINGS
#include "Define.h"
#include "Data.h"

// 밀리초 (ms)
const int g_viewUpdateTick = 50;
const int g_gameInfoUpdateTick = 10;

const int g_ingameAreaSizeY = 30;
const int g_ingameAreaSizeX = horizon;
char g_ingameScreen[31][horizon + 2];
char g_ingameUIScreen[4][horizon + 2];

int playerSpawnPointY;
int playerSpawnPointX;

bool isPlayGameState = false;
bool isPlayerEffectState = false;
int isPlayerEffectCount = 0;

int main(void)
{
	LoadFileData();
	srand(time(NULL));
	system(SetIngameSizeStr);
	currWindowType = NoneWindow;
	OpenWindow(TitleWindow);
}

#pragma region GameLogic
void StartGame()
{
	InitScreen();
	SetGameInfo();
	InitInGame();
	StartInGame();
}

void SetGameInfo()
{
	isPlayGameState = true;
	isPlayerEffectState = false;
	isPlayerEffectCount = 0;
	playerReviveTimer = NULL;
	playerInvincibleTimer = NULL;
	playerSpawnPointY = g_ingameAreaSizeY - 2;
	playerSpawnPointX = g_ingameAreaSizeX / 2 - 5;

	gameInfoData.currStageId = 1;
	gameInfoData.currScore = 0;

	gameInfoData.invincibleTime = 1500;
	gameInfoData.limitSpeed = 270;
	gameInfoData.limitAttackSpeed = 60;
	gameInfoData.limitLife = 9;
	gameInfoData.limitPower = 4;

	strcpy(gameInfoData.playerBodyChars, "-xA");
	strcpy(gameInfoData.enemyBodyChars, "[]*:<>wRV=");
	gameInfoData.playerBulletChar = '|';
	strcpy(gameInfoData.enemyBulletChars, "vW+");
	strcpy(gameInfoData.itemChars, "{}PLS");

	g_leftArrowKeyPressTick = 0;
	g_rightArrowKeyPressTick = 0;
	g_upArrowKeyPressTick = 0;
	g_downArrowKeyPressTick = 0;
	g_spaceKeyPressTick = 0;
}

void InitInGame()
{
	InitPlayer();
	InitEnemy();
	InitBullet();
	InitStage();
	InitItem();
	StartStage(1);
}

void StartInGame()
{
	DWORD gameInfoUpdateStandardTime = GetTickCount64(); // 0.01초
	DWORD viewUpdateStandardTime = GetTickCount64(); // 0.05초

	while (true)
	{
		DWORD gameInfoUpdateElapsedTime = GetTickCount64() - gameInfoUpdateStandardTime;
		if (gameInfoUpdateElapsedTime > g_gameInfoUpdateTick)
		{
			gameInfoUpdateStandardTime = GetTickCount64() - (gameInfoUpdateElapsedTime - g_gameInfoUpdateTick);

			// 실제 화면에 그리기 전에 필요한 연산 ( 0.01초 )
			UpdateInput();
			UpdateBullet();
			UpdateEnemy();
			UpdateItem();
			UpdatePlayer();
			UpdateStage();

		}

		DWORD viewUpdateElapsedTime = GetTickCount64() - viewUpdateStandardTime;
		if (viewUpdateElapsedTime > g_viewUpdateTick)
		{
			viewUpdateStandardTime = GetTickCount64() - (viewUpdateElapsedTime - g_viewUpdateTick);

			// 실제 화면에 그릴 때 필요한 연산 ( 0.05초 )
			if (!player.isDie && player.isInvincible)
				PlayerEffect();

			UpdateUI();
			UpdateScreen();
		}
	}
}

void EndGame()
{
	SaveFileData();
	ClearScreen();
	ReleaseScreen();
	OpenWindow(ResultWindow);
}

void OpenWindow(enum EWindowType type)
{
	if (currWindowType == type)
		return;

	currWindowType = type;
	PlayBgmSound(type, true);

	switch (type)
	{
	case TitleWindow:
		DrawTitleWindow();
		break;
	case InGameWindow:
		StartGame();
		break;
	case ResultWindow:
		DrawResultWindow();
		break;
	}
}

void DrawTitleWindow()
{
	PrintTitleWindow();

	while (currWindowType == TitleWindow)
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			OnEscKey();

		if (GetAsyncKeyState(0x53) & 0x0001)
		{
			OpenWindow(InGameWindow);
			return;
		}
	}
}

void DrawResultWindow()
{
	ClearScreen();
	PrintResultWindow(gameInfoData.currScore, gameInfoData.maxScore);

	while (currWindowType == ResultWindow)
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			OnEscKey();

		if (GetAsyncKeyState(0x4E) & 0x0001)
		{
			OpenWindow(TitleWindow);
			return;
		}
	}
}
#pragma endregion

#pragma region Data
const char* fileDataPath = "../GameData.txt";

void SaveFileData()
{
	FILE* fp = fopen(fileDataPath, "w");

	if (fp == NULL)
		return;

	fprintf(fp, "%d", gameInfoData.maxScore);
	fclose(fp);
}

void LoadFileData()
{
	FILE* fp = fopen(fileDataPath, "r");

	if (fp == NULL)
		return;

	fscanf(fp, "%d", &gameInfoData.maxScore);
	fclose(fp);
}

void AddScore(int value)
{
	gameInfoData.currScore += value;
	if (gameInfoData.maxScore < gameInfoData.currScore)
		gameInfoData.maxScore = gameInfoData.currScore;
}
#pragma endregion

#pragma region Stage
void InitStage()
{
	waitNextStageTime = 10;
}

void StartStage(int stageId)
{
	isWaitNextStage = true;
	currWaitStageTick = 0;

	enemySpawnTick = 0;
	enemySpawnDelay = 20;
	
	currSpawnCount = 0;
	enemyRemoveCount = 0;
	maxSpawnCount = stageId + 3;
}

void UpdateStage()
{
	if (isWaitNextStage)
	{
		currWaitStageTick += 1;

		if (IsFormulaFillTick(&currWaitStageTick, waitNextStageTime))
		{
			// 스테이지 정보 없애기
			isWaitNextStage = false;
		}
		return;
	}

	enemySpawnTick += 1;

	if (currSpawnCount < maxSpawnCount && IsFormulaFillTick(&enemySpawnTick, enemySpawnDelay))
	{
		for (int i = 0; i < sizeof(enemyPool) / sizeof(ENEMY); i++)
		{
			if (enemyPool[i].isUsing == false)
			{
				currSpawnCount += 1;
				SpawnEnemy(&enemyPool[i]);
				break;
			}
		}
		enemySpawnTick = 0;
	}

	if (enemyRemoveCount >= maxSpawnCount)
	{
		gameInfoData.currStageId += 1;
		StartStage(gameInfoData.currStageId);
	}
}
#pragma endregion

#pragma region Player
void InitPlayer()
{
	player.life = 2;
	if (player.power > 2) 
		player.power = 2;
	else player.power = 1;

	player.isDie = false;
	player.isInvincible = true;

	player.y = playerSpawnPointY;
	player.x = playerSpawnPointX;
	player.damage = 70;
	player.speed = 170;
	player.bulletSpeed = 300;
	player.attackSpeed = 30;
	player.shapeSize = 5;
	strcpy(player.shapeStr, "-xAx-");

	DrawPlayer();
}

void UpdatePlayer()
{
	if (player.isDie)
	{
		if(playerReviveTimer == NULL)
			playerReviveTimer = GetTickCount64();

		if (playerReviveTimer + 1000 < GetTickCount64())
		{
			int life = player.life;
			playerReviveTimer = NULL;
			InitPlayer();
			player.life = life;
		}
		return;
	}
		
	ErasePlayer();

	if (IsFormulaFillTick(&g_leftArrowKeyPressTick, player.speed))
		OnPlayerAction(Left);
	
	if (IsFormulaFillTick(&g_rightArrowKeyPressTick, player.speed))
		OnPlayerAction(Right);

	if (IsFormulaFillTick(&g_upArrowKeyPressTick, player.speed / 2))
		OnPlayerAction(Up);
	
	if (IsFormulaFillTick(&g_downArrowKeyPressTick, player.speed / 2))
		OnPlayerAction(Down);

	if (IsFormulaFillTick(&g_spaceKeyPressTick, player.attackSpeed))
		OnPlayerAction(Attack);

	if (player.isInvincible)
		return;

	CheckPlayerCollision();
	DrawPlayer();
}

void ErasePlayer()
{
	for (int i = 0; i < player.shapeSize; i++)
		g_ingameScreen[player.y][player.x + i] = ' ';
}

void DrawPlayer()
{
	for (int i = 0; i < player.shapeSize; i++)
		g_ingameScreen[player.y][player.x + i] = player.shapeStr[i];
}

void OnPlayerAction(enum EPlayerActionType type)
{
	switch (type)
	{
	case Left:
	{
		player.x--;
		if (player.x < 1)
			player.x = 1;
		break;
	}
	case Right:
	{
		player.x++;
		if (player.x + player.shapeSize >= g_ingameAreaSizeX)
			player.x = g_ingameAreaSizeX - player.shapeSize;
		break;
	}
	case Up:
	{
		player.y--;
		if (player.y < 1)
			player.y = 1;
		break;
	}
	case Down:
	{
		player.y++;
		if (player.y >= g_ingameAreaSizeY)
			player.y = g_ingameAreaSizeY;
		break;
	}
	case Attack:
		SpawnPlayerBullets();
		break;
	case Hit:
		OnPlayerHit();
		break;
	default:
		// 없는 타입 들어옴 (break point)
		break;
	}
}

void CheckPlayerCollision()
{
	for (int i = 0; i < player.shapeSize; i++)
	{
		char CheckCh = g_ingameScreen[player.y][player.x + i];
		if (CheckCh == ' ')
			continue;

		enum EObjectType type = GetCustomObjectType(CheckCh);
		
		if (type == Enemy)
		{
			OnPlayerAction(Hit);
		 	struct _ENEMY* enemy = FindEnemyObject(player.y, player.x + i);
			if (enemy != NULL && enemy->isUsing)
			{
				enemyRemoveCount += 1;
				enemy->isUsing = false;
				return;
			}
		}
	}
}

void OnPlayerHit()
{
	if (player.isInvincible)
		return;

	ErasePlayer();

	player.life -= 1;
	player.isDie = true;
	player.isInvincible = true;

	if (player.life <= 0)
	{
		isPlayGameState = false;
		player.power = 1;
		EndGame();
	}
}

void PlayerEffect()
{
	if (playerInvincibleTimer == NULL)
		playerInvincibleTimer = GetTickCount64();

	isPlayerEffectCount++;
	if (isPlayerEffectCount >= 5)
	{
		isPlayerEffectCount = 0;
		isPlayerEffectState = !isPlayerEffectState;
	}

	(isPlayerEffectState) ? DrawPlayer() : ErasePlayer();

	if (playerInvincibleTimer + gameInfoData.invincibleTime < GetTickCount64())
	{
		player.isInvincible = false;
		playerInvincibleTimer = NULL;
	}
}
#pragma endregion

#pragma region Item
void OnGetItem(enum EItemType type)
{
	switch (type)
	{
	case PowerItem:
		player.power += 1;
		if (player.power > gameInfoData.limitPower)
			player.damage += 10;
		break;
	case LifeItem:
		player.life += 1;
		if (player.life > gameInfoData.limitLife)
			player.life = gameInfoData.limitLife;
		break;
	case SpeedItem:
		player.speed += 15;
		if (player.speed > gameInfoData.limitSpeed)
			player.speed = gameInfoData.limitSpeed;
		player.attackSpeed += 10;
		if (player.attackSpeed > gameInfoData.limitAttackSpeed)
			player.attackSpeed = gameInfoData.limitAttackSpeed;
		break;
	default:
		return;
	}

	AddScore(10);
}

void InitItem()
{
	for (int i = 0; i < sizeof(itemPool) / sizeof(ITEM); i++)
	{
		itemPool[i].isUsing = false; 
	}
}

void SpawnItem(enum EItemType type, int y, int x)
{
	for (int i = 0; i < sizeof(itemPool) / sizeof(ITEM); i++)
	{
		if (itemPool[i].isUsing == false)
		{
			itemPool[i].itemType = type;
			itemPool[i].y = y;
			itemPool[i].x = x;
			itemPool[i].speed = 40;
			itemPool[i].moveTick = 0;
			itemPool[i].isDirRight = (rand() % 2 == 0) ? true : false;
			itemPool[i].isDirDown = (rand() % 2 == 0) ? true : false;
			itemPool[i].isUsing = true;

			switch (itemPool[i].itemType)
			{
			case PowerItem:
				itemPool[i].shapeSize = 3;
				strcpy(itemPool[i].shapeStr, "{P}");
				break;
			case LifeItem:
				itemPool[i].shapeSize = 3;
				strcpy(itemPool[i].shapeStr, "{L}");
				break;
			case SpeedItem:
				itemPool[i].shapeSize = 3;
				strcpy(itemPool[i].shapeStr, "{S}");
				break;
			default:
				// 없는 타입 들어옴 (break point)
				break;
			}
			break;
		}
	}
}

void UpdateItem()
{
	for (int i = 0; i < sizeof(itemPool) / sizeof(ITEM); i++)
	{
		if (itemPool[i].isUsing)
		{
			itemPool[i].moveTick += 1;

			if (IsFormulaFillTick(&itemPool[i].moveTick, itemPool[i].speed))
				MoveItem(&itemPool[i]);

			if (player.isDie == false && player.y == itemPool[i].y)
			{
				for (int j = 0; j < itemPool[i].shapeSize; j++)
				{
					if (player.x <= itemPool[i].x + j &&
						player.x + player.shapeSize > itemPool[i].x + j)
					{
						OnGetItem(itemPool[i].itemType);
						itemPool[i].isUsing = false;
						break;
					}
				}
			}
			
			DrawItem(&itemPool[i]);
		}
	}
}

void MoveItem(struct _ITEM* item)
{
	EraseItem(item);
	
	item->y += (item->isDirDown) ? 1 : -1;
	item->x += (item->isDirRight) ? 1 : -1;

	if (item->y >= g_ingameAreaSizeY) item->isDirDown = false;
	if (item->x + item->shapeSize >= g_ingameAreaSizeX) item->isDirRight = false;
	if (item->y <= 0) item->isDirDown = true;
	if (item->x <= 0) item->isDirRight = true;

	DrawItem(item);
}

void DrawItem(struct _ITEM* item)
{
	for (int i = 0; i < item->shapeSize; i++)
		if (g_ingameScreen[item->y][item->x + i] == ' ')
			g_ingameScreen[item->y][item->x + i] = item->shapeStr[i];
}

void EraseItem(struct _ITEM* item)
{
	for (int i = 0; i < item->shapeSize; i++)
		if (g_ingameScreen[item->y][item->x + i] == item->shapeStr[i])
			g_ingameScreen[item->y][item->x + i] = ' ';
}
#pragma endregion

#pragma region Enemy
void InitEnemy()
{
	for (int i = 0; i < sizeof(enemyPool) / sizeof(ENEMY); i++)
	{
		enemyPool[i].x = 0;
		enemyPool[i].y = 0;
		enemyPool[i].isUsing = false;
		enemyPool[i].isHit = false;
	}
}

void UpdateEnemy()
{
	for (int i = 0; i < sizeof(enemyPool) / sizeof(ENEMY); i++)
	{
		if (enemyPool[i].isUsing)
		{
			enemyPool[i].moveTick++;
			if (IsFormulaFillTick(&enemyPool[i].moveTick, enemyPool[i].speed))
			{
				MoveEnemy(&enemyPool[i]);
				enemyPool[i].moveTick = 0;
			}

			enemyPool[i].attackTick++;
			if (IsFormulaFillTick(&enemyPool[i].attackTick, enemyPool[i].attackSpeed))
			{
				SpawnEnemyBullet(&enemyPool[i]);
				enemyPool[i].attackTick = 0;
			}
		}
	}

	DrawEnemy();
}

void MoveEnemy(struct _ENEMY* enemy)
{
	bool isRightMove = rand() % 2 == 0;
	int movePosX = (isRightMove) ? 1 : -1;
	int movePosY = ((rand() % 3) - 1);

	if (movePosX == 0 && movePosY == 0)
		return;

	bool isMove = IsEmptySpaceCheck(enemy->y + movePosY, enemy->x + movePosX, enemy->shapeSize);
	if (isMove)
	{
		EraseEnemy(enemy);
		enemy->y += movePosY;
		enemy->x += movePosX;

		if (enemy->x < 1) enemy->x = 1;
		if (enemy->y < 1) enemy->y = 1;

		if (enemy->x >= g_ingameAreaSizeX - 1)
			enemy->x = g_ingameAreaSizeX - 2;
		if (enemy->y > g_ingameAreaSizeY / 2)
			enemy->y = g_ingameAreaSizeY / 2;

		DrawEnemy(enemy);
	}
}

void SpawnEnemy(struct _ENEMY* enemy)
{
	int enemyId = rand() % (int)MaxEnemyType;

	enemy->enemyType = enemyId;
	enemy->moveTick = 0;
	enemy->hitTick = 0;

	switch (enemy->enemyType)
	{
	case Enemy_S:
		enemy->hp = 50 + (gameInfoData.currStageId * 10);
		enemy->point = 5 * gameInfoData.currStageId;
		enemy->speed = 75;
		enemy->attackSpeed = 7 + (rand() % 2);
		enemy->bulletSpeed = 130;
		enemy->bulletShapeCh = 'v';
		enemy->shapeSize = 3;
		strcpy(enemy->shapeStr, "=V=");
		break;
	case Enemy_M:
		enemy->hp = 100 + (gameInfoData.currStageId * 15);
		enemy->point = 7 * gameInfoData.currStageId;
		enemy->speed = 40;
		enemy->attackSpeed = 4 + (rand() % 2);
		enemy->bulletSpeed = 200;
		enemy->bulletShapeCh = 'W';
		enemy->shapeSize = 5;
		strcpy(enemy->shapeStr, "<RwR>");
		break;
	case Enemy_L:
		enemy->hp = 150 + (gameInfoData.currStageId * 20);
		enemy->point = 10 * gameInfoData.currStageId;
		enemy->speed = 30;
		enemy->attackSpeed = 5 + (rand() % 2);
		enemy->bulletSpeed = 100;
		enemy->bulletShapeCh = '+';
		enemy->shapeSize = 5;
		strcpy(enemy->shapeStr, "[*:*]");
		break;
	default:
		// 없는 타입 들어옴 (break point)
		break;
	}

	while (enemy->isUsing == false)
	{
		int spawnPosY = rand() % (g_ingameAreaSizeY / 2) + 1;
		int spawnPosX = rand() % (g_ingameAreaSizeX - 5) + 1;

		bool isUsing = IsEmptySpaceCheck(spawnPosY, spawnPosX, enemy->shapeSize);

		if (isUsing)
		{
			enemy->y = spawnPosY;
			enemy->x = spawnPosX;
			enemy->isUsing = true;

			for (int i = 0; i < enemy->shapeSize; i++)
				g_ingameScreen[spawnPosY][spawnPosX + i] = enemy->shapeStr[i];
		}
	}
}

void DrawEnemy()
{
	for (int i = 0; i < sizeof(enemyPool) / sizeof(ENEMY); i++)
	{
		if (enemyPool[i].isUsing)
		{
			if (enemyPool[i].isHit)
			{
				enemyPool[i].hitTick += 1;

				if (IsFormulaFillTick(&enemyPool[i].hitTick, 100))
				{
					enemyPool[i].hitTick = 0;
					enemyPool[i].isHit = false;
				}				
				continue;
			}

			for (int j = 0; j < enemyPool[i].shapeSize; j++)
				g_ingameScreen[enemyPool[i].y][enemyPool[i].x + j] = enemyPool[i].shapeStr[j];
		}
	}
		
}

void EraseEnemy(struct _ENEMY* enemy)
{
	for (int i = 0; i < enemy->shapeSize; i++)
		g_ingameScreen[enemy->y][enemy->x + i] = ' ';
}

void OnEnemyHit(struct _ENEMY* enemy)
{
	if (enemy == NULL || !enemy->isUsing)
		return;

	enemy->hp -= player.damage;
	AddScore(1);

	if (enemy->hp <= 0)
	{
		int random = rand() % 100 + 1;
		if (15 >= random)
			SpawnItem(PowerItem, enemy->y, enemy->x);
		else if (30 >= random)
			SpawnItem(SpeedItem, enemy->y, enemy->x);
		else if (33 >= random)
			SpawnItem(LifeItem, enemy->y, enemy->x);

		AddScore(enemy->point);
		enemyRemoveCount += 1;
		enemy->isUsing = false;
	}
	else
		enemy->isHit = true;
}
#pragma endregion

#pragma region Bullet
void InitBullet()
{
	// playerBullet
	for (int i = 0; i < sizeof(playerBulletPool)/sizeof(BULLET); i++)
	{
		playerBulletPool[i].x = 0;
		playerBulletPool[i].y = 0;
		playerBulletPool[i].speed = player.bulletSpeed;
		playerBulletPool[i].moveTick = 0;
		playerBulletPool[i].shapeCh = '|';
		playerBulletPool[i].isUsing = false;
	}

	// enemyBullet
	for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
	{
		enemyBulletPool[i].x = 0;
		enemyBulletPool[i].y = 0;
		enemyBulletPool[i].moveTick = 0;
		enemyBulletPool[i].isUsing = false;
	}

	// enemyTiltBullet
	for (int i = 0; i < sizeof(enemyTiltBulletPool) / sizeof(TILTBULLET); i++)
	{
		enemyTiltBulletPool[i].x = 0;
		enemyTiltBulletPool[i].y = 0;
		enemyTiltBulletPool[i].moveTick = 0;
		enemyTiltBulletPool[i].isUsing = false;
	}
}

void UpdateBullet()
{
	// playerBullet
	for (int i = 0; i < sizeof(playerBulletPool) / sizeof(BULLET); i++)
	{
		if (playerBulletPool[i].isUsing)
		{
			playerBulletPool[i].moveTick += 1;

			if (IsFormulaFillTick(&playerBulletPool[i].moveTick, playerBulletPool[i].speed))
				ShotPlayerBullet(&playerBulletPool[i]);
			else
				DrawBullet(&playerBulletPool[i]);
		}
	}

	// enemyBullet
	for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
	{
		if (enemyBulletPool[i].isUsing)
		{
			enemyBulletPool[i].moveTick += 1;

			if (IsFormulaFillTick(&enemyBulletPool[i].moveTick, enemyBulletPool[i].speed))
				ShotEnemyBullet(&enemyBulletPool[i]);
			else
				DrawBullet(&enemyBulletPool[i]);
		}
	}

	// enemyTiltBullet
	for (int i = 0; i < sizeof(enemyTiltBulletPool) / sizeof(TILTBULLET); i++)
	{
		if (enemyTiltBulletPool[i].isUsing)
		{
			enemyTiltBulletPool[i].moveTick += 1;

			if (IsFormulaFillTick(&enemyTiltBulletPool[i].moveTick, enemyTiltBulletPool[i].speed))
				ShotEnemyTiltBullet(&enemyTiltBulletPool[i]);
			else
				DrawTiltBullet(&enemyTiltBulletPool[i]);
		}
	}
}

void SpawnPlayerBullets()
{
	if (player.power >= 4)
	{
		SpawnPlayerBullet(2);
		SpawnPlayerBullet(1);
		SpawnPlayerBullet(-1);
		SpawnPlayerBullet(-2);
	}
	else if (player.power >= 3)
	{
		SpawnPlayerBullet(2);
		SpawnPlayerBullet(0);
		SpawnPlayerBullet(-2);
	}
	else if (player.power >= 2)
	{
		SpawnPlayerBullet(1);
		SpawnPlayerBullet(-1);
	}
	else
	{
		SpawnPlayerBullet(0);
	}
}

void SpawnPlayerBullet(int x)
{
	for (int i = 0; i < sizeof(playerBulletPool) / sizeof(BULLET); i++)
	{
		if (playerBulletPool[i].isUsing == false)
		{
			playerBulletPool[i].x = player.x + (player.shapeSize / 2) + x;
			playerBulletPool[i].y = player.y;
			playerBulletPool[i].isUsing = true;
			break;
		}
	}
}

void ShotPlayerBullet(struct _BULLET* bullet)
{
	EraseBullet(bullet);
	bullet->y -= 1;
	
	if (bullet->y < 0)
	{
		bullet->isUsing = false;
		return;
	}
	
	enum EObjectType type = GetCustomObjectType(g_ingameScreen[bullet->y][bullet->x]);

	if (type == Enemy)
	{
		struct _ENEMY* enemy = FindEnemyObject(bullet->y, bullet->x);

		if (enemy != NULL && enemy->isUsing)
		{
			OnEnemyHit(enemy);
			bullet->isUsing = false;
		}
		else
		{
			// 못 찾음 (break point)
		}
	}
	else if (type == None)
		DrawBullet(bullet);
}

void SpawnEnemyBullet(struct _ENEMY* enemy)
{
	if (enemy->enemyType == Enemy_L)
	{
		for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
		{
			if (enemyBulletPool[i].isUsing == false)
			{
				enemyBulletPool[i].x = enemy->x + 2;
				enemyBulletPool[i].y = enemy->y + 1;
				enemyBulletPool[i].speed = enemy->bulletSpeed;
				enemyBulletPool[i].shapeCh = enemy->bulletShapeCh;
				enemyBulletPool[i].moveTick = 0;
				enemyBulletPool[i].isUsing = true;
				break;
			}
		}

		for (int i = 0; i < sizeof(enemyTiltBulletPool) / sizeof(TILTBULLET); i++)
		{
			if (enemyTiltBulletPool[i].isUsing == false)
			{
				enemyTiltBulletPool[i].x = enemy->x;
				enemyTiltBulletPool[i].y = enemy->y + 1;
				enemyTiltBulletPool[i].speed = enemy->bulletSpeed;
				enemyTiltBulletPool[i].shapeCh = enemy->bulletShapeCh;
				enemyTiltBulletPool[i].isRight = true;
				enemyTiltBulletPool[i].isUsing = true;
				break;
			}
		}

		for (int i = 0; i < sizeof(enemyTiltBulletPool) / sizeof(TILTBULLET); i++)
		{
			if (enemyTiltBulletPool[i].isUsing == false)
			{
				enemyTiltBulletPool[i].x = enemy->x + 4;
				enemyTiltBulletPool[i].y = enemy->y + 1;
				enemyTiltBulletPool[i].speed = enemy->bulletSpeed;
				enemyTiltBulletPool[i].shapeCh = enemy->bulletShapeCh;
				enemyTiltBulletPool[i].isRight = false;
				enemyTiltBulletPool[i].isUsing = true;
				break;
			}
		}
		return;
	}

	for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
	{
		if (enemyBulletPool[i].isUsing == false)
		{
			enemyBulletPool[i].x = enemy->x + 1;
			enemyBulletPool[i].y = enemy->y + 1;
			enemyBulletPool[i].speed = enemy->bulletSpeed;
			enemyBulletPool[i].shapeCh = enemy->bulletShapeCh;
			enemyBulletPool[i].moveTick = 0;
			enemyBulletPool[i].isUsing = true;
			break;
		}
	}

	if (enemy->enemyType == Enemy_M)
	{
		for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
		{
			if (enemyBulletPool[i].isUsing == false)
			{
				enemyBulletPool[i].x = enemy->x + 3;
				enemyBulletPool[i].y = enemy->y + 1;
				enemyBulletPool[i].speed = enemy->bulletSpeed;
				enemyBulletPool[i].shapeCh = enemy->bulletShapeCh;
				enemyBulletPool[i].moveTick = 0;
				enemyBulletPool[i].isUsing = true;
				break;
			}
		}
	}
}

void ShotEnemyBullet(struct _BULLET* bullet)
{
	EraseBullet(bullet);
	bullet->y += 1;

	if (bullet->y > g_ingameAreaSizeY + 1)
	{
		bullet->isUsing = false;
		return;
	}

	enum EObjectType type = GetCustomObjectType(g_ingameScreen[bullet->y][bullet->x]);
	
	if (type == Player)
	{
		OnPlayerAction(Hit);
		bullet->isUsing = false;
	}
		
	if (type == None)
		DrawBullet(bullet);
}

void ShotEnemyTiltBullet(struct _TILTBULLET* bullet)
{
	EraseTiltBullet(bullet);

	bullet->y += 1;
	if (bullet->y % 2 == 0)
		bullet->x += (bullet->isRight) ? -1 : 1;

	if (bullet->y > g_ingameAreaSizeY + 1 || 
		bullet->x < 0 || bullet->x >= g_ingameAreaSizeX)
	{
		bullet->isUsing = false;
		return;
	}

	enum EObjectType type = GetCustomObjectType(g_ingameScreen[bullet->y][bullet->x]);

	if (type == Player)
	{
		OnPlayerAction(Hit);
		bullet->isUsing = false;
	}

	if (type == None)
		DrawTiltBullet(bullet);
}

void DrawBullet(struct _BULLET* bullet)
{
	if (g_ingameScreen[bullet->y][bullet->x] == ' ')
		g_ingameScreen[bullet->y][bullet->x] = bullet->shapeCh;
}

void DrawTiltBullet(struct _TILTBULLET* bullet)
{
	if (g_ingameScreen[bullet->y][bullet->x] == ' ')
		g_ingameScreen[bullet->y][bullet->x] = bullet->shapeCh;
}

void EraseBullet(struct _BULLET* bullet)
{
	if (g_ingameScreen[bullet->y][bullet->x] == bullet->shapeCh)
		g_ingameScreen[bullet->y][bullet->x] = ' ';
}

void EraseTiltBullet(struct _TILTBULLET* bullet)
{
	if (g_ingameScreen[bullet->y][bullet->x] == bullet->shapeCh)
		g_ingameScreen[bullet->y][bullet->x] = ' ';
}
#pragma endregion

#pragma region Input
void UpdateInput()
{
	if (player.isDie || !isPlayGameState)
		return;

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		g_leftArrowKeyPressTick++;
	else g_leftArrowKeyPressTick = 0;

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) 
		g_rightArrowKeyPressTick++;
	else g_rightArrowKeyPressTick = 0;

	if (GetAsyncKeyState(VK_UP) & 0x8000) 
		g_upArrowKeyPressTick++; 
	else g_upArrowKeyPressTick = 0;

	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		g_downArrowKeyPressTick++;
	else g_downArrowKeyPressTick = 0;
		
	if (GetAsyncKeyState(VK_SPACE) && 0x8000)
		g_spaceKeyPressTick++;
}

void OnEscKey()
{
	ReleaseScreen();
	exit(0);
}
#pragma endregion

#pragma region IngameUI
void UpdateUI()
{
	SetUIText(gameInfoData.currStageId, player.life, player.power, 
		player.speed, gameInfoData.currScore, gameInfoData.maxScore);
}

void SetUIText(int stageId, int life, int power, int speed, int score, int maxScore)
{
	char str[20];

	// Set ingameUIStr
	char temp[horizon + 2] = " Stage : ";
	sprintf(str, "%d", stageId);
	strcat(temp, str);
	
	strcat(temp, "  Life : ");
	sprintf(str, "%d", life);
	strcat(temp, str);
	
	strcat(temp, "  Power : ");
	sprintf(str, "%2d", power);
	strcat(temp, str);

	strcat(temp, "  Speed : ");
	sprintf(str, "%d", speed);
	strcat(temp, str);

	strcat(temp, "  Score : ");
	sprintf(str, "%5d", score);
	strcat(temp, str);

	strcat(temp, "  MaxScore : ");
	sprintf(str, "%5d", maxScore);
	strcat(temp, str);

	strcpy(ingameUIStr, temp);
}
#pragma endregion

#pragma region Screen
void InitScreen()
{
	CONSOLE_CURSOR_INFO cci;

	//화면 버퍼 2개를 만든다.
	g_hScreen[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	g_hScreen[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	//커서 숨기기
	cci.dwSize = 1;
	cci.bVisible = FALSE;
	SetConsoleCursorInfo(g_hScreen[0], &cci);
	SetConsoleCursorInfo(g_hScreen[1], &cci);

	for (int y = 0; y <= g_ingameAreaSizeY; y++)
	{
		for (int x = 0; x < g_ingameAreaSizeX; x++)
		{
			g_ingameScreen[y][x] = ' ';
		}
		g_ingameScreen[y][g_ingameAreaSizeX + 1] = '\0';
	}
}

void UpdateScreen()
{
	ClearScreen();

	for (int y = 0; y <= g_ingameAreaSizeY; y++)
	{
		for (int x = 0; x < g_ingameAreaSizeX; x++)
		{
			char tempStr[2] = "";
			tempStr[0] = g_ingameScreen[y][x];
			tempStr[1] = '\0';

			if (tempStr[0] == ' ')
				continue;

			enum EObjectType type = GetCustomObjectType(tempStr[0]);
			switch (type)
			{
			case Player: CharPrintScreen(y, x, tempStr, WHITE); break;
			case Enemy: CharPrintScreen(y, x, tempStr, RED); break;
			case PlayerBullet: CharPrintScreen(y, x, tempStr, WHITE); break;
			case EnemyBullet: CharPrintScreen(y, x, tempStr, YELLOW); break;
			case Item: CharPrintScreen(y, x, tempStr, DarkYellow); break;
			}
		}
	}

	if (isWaitNextStage)
	{
		char tempCh[horizon];
		sprintf(tempCh, "                                     Stage %d", gameInfoData.currStageId);
		LinePrintScreen(g_ingameAreaSizeY / 2, tempCh, RED);
	}
	
	DividingLinePrintScreen(g_ingameAreaSizeY + 2, GREEN);
	LinePrintScreen(g_ingameAreaSizeY + 3, ingameUIStr, SkyBlue);
	DividingLinePrintScreen(g_ingameAreaSizeY + 4, GREEN);
	
	FlippingScreen();
}

void ClearScreen()
{
	COORD Coor = { 0,0 };
	DWORD dw;
	FillConsoleOutputCharacter(g_hScreen[g_nScreenIndex], ' ', (g_ingameAreaSizeY * g_ingameAreaSizeX) + (g_ingameAreaSizeX * 2), Coor, &dw);
}

void CharPrintScreen(int y, int x, char* string, enum ColorType colorType)
{
	DWORD dw;
	COORD CursorPosition = { x, y };
	SetConsoleCursorPosition(g_hScreen[g_nScreenIndex], CursorPosition);
	SetColor(colorType);
	WriteFile(g_hScreen[g_nScreenIndex], string, strlen(string), &dw, NULL);
}

void LinePrintScreen(int y, char* string, enum ColorType colorType)
{
	DWORD dw;
	COORD CursorPosition = { 0, y };
	SetConsoleCursorPosition(g_hScreen[g_nScreenIndex], CursorPosition);
	SetColor(colorType);
	WriteFile(g_hScreen[g_nScreenIndex], string, strlen(string), &dw, NULL);
}

void DividingLinePrintScreen(int y, enum ColorType colorType)
{
	DWORD dw;
	COORD CursorPosition = { 0, y };
	SetConsoleCursorPosition(g_hScreen[g_nScreenIndex], CursorPosition);

	char string[horizon + 3] = "";
	for (int i = 2; i < horizon + 2; i += 2)
		strcat(string, "ㅡ");
	string[horizon + 2] = '\0';

	SetColor(colorType);
	WriteFile(g_hScreen[g_nScreenIndex], string, strlen(string), &dw, NULL);
}

void FlippingScreen()
{
	for (int y = 0; y <= g_ingameAreaSizeY; y++)
	{
		for (int x = 0; x < g_ingameAreaSizeX; x++)
		{
			g_ingameScreen[y][x] = ' ';
		}
		g_ingameScreen[y][g_ingameAreaSizeX + 1] = '\0';
	}

	SetConsoleActiveScreenBuffer(g_hScreen[g_nScreenIndex]);
	g_nScreenIndex = !g_nScreenIndex;
}

void ReleaseScreen()
{
	if (g_hScreen[0] != NULL) CloseHandle(g_hScreen[0]);
	if (g_hScreen[1] != NULL) CloseHandle(g_hScreen[1]);
	system("cls");
}

void SetColor(unsigned short color)
{
	SetConsoleTextAttribute(g_hScreen[g_nScreenIndex], color);
}
#pragma endregion

#pragma region Util
void PlayBgmSound(enum EWindowType type)
{
	switch (type)
	{
	case TitleWindow:
		PlaySound(TEXT("../TitleBgm.wav"), NULL, SND_ASYNC | SND_LOOP);
		break;
	case InGameWindow:
		PlaySound(TEXT("../InGameBgm.wav"), NULL, SND_ASYNC | SND_LOOP);
		break;
	case ResultWindow:
		PlaySound(TEXT("../ResultBgm.wav"), NULL, SND_ASYNC | SND_LOOP);
		break;
	}
}

bool IsFormulaFillTick(int* tick, float value)
{
	int stdValue = 1000 / (float)value;
	if (stdValue <= *tick)
	{
		*tick -= stdValue;
		return true;
	}
	return false;
}

// 비어있으면 True
bool IsEmptySpaceCheck(int y, int x, int size)
{
	for (int i = 0; i <= size; i++)
		if (g_ingameScreen[y][x + i] != ' ')
			return false;

	return true;
}

// 못 얻으면 None
enum EObjectType GetCustomObjectType(char findCh)
{
	if (findCh == gameInfoData.playerBulletChar)
		return PlayerBullet;

	for (int i = 0; i < strlen(gameInfoData.playerBodyChars); i++)
		if (findCh == gameInfoData.playerBodyChars[i])
			return Player;

	for (int i = 0; i < strlen(gameInfoData.enemyBodyChars); i++)
		if (findCh == gameInfoData.enemyBodyChars[i])
			return Enemy;

	for (int i = 0; i < strlen(gameInfoData.enemyBulletChars); i++)
		if (findCh == gameInfoData.enemyBulletChars[i])
			return EnemyBullet;

	for (int i = 0; i < strlen(gameInfoData.itemChars); i++)
		if (findCh == gameInfoData.itemChars[i])
			return Item;

	return None;
}

// 못 찾으면 NULL
struct _ENEMY* FindEnemyObject(int y, int x)
{
	for (int i = 0; i < sizeof(enemyPool) / sizeof(ENEMY); i++)
	{
		if (enemyPool[i].isUsing == false)
			continue;

		if (enemyPool[i].y == y && enemyPool[i].x <= x &&
			enemyPool[i].x + enemyPool[i].shapeSize > x)
			return &enemyPool[i];
	}
	return NULL;
}

// 못 찾으면 NULL
struct _ITEM* FindItemObject(int y, int x)
{
	for (int i = 0; i < sizeof(itemPool) / sizeof(ITEM); i++)
	{
		if (itemPool[i].isUsing == false)
			continue;

		if (itemPool[i].y == y && itemPool[i].x <= x &&
			itemPool[i].x + itemPool[i].shapeSize > x)
			return &itemPool[i];
	}
	return NULL;
}

// 못 찾으면 NULL
struct _BULLET* FindPlayerBullet(int y, int x)
{
	for (int i = 0; i < sizeof(playerBulletPool) / sizeof(BULLET); i++)
	{
		if (playerBulletPool[i].isUsing == false)
			continue;

		if (playerBulletPool[i].y == y && playerBulletPool[i].x == x)
			return &playerBulletPool[i];
	}
	return NULL;
}

// 못 찾으면 NULL
struct _BULLET* FindEnemyBullet(int y, int x)
{
	for (int i = 0; i < sizeof(enemyBulletPool) / sizeof(BULLET); i++)
	{
		if (enemyBulletPool[i].isUsing == false)
			continue;

		if (enemyBulletPool[i].y == y && enemyBulletPool[i].x == x)
			return &enemyBulletPool[i];
	}
	return NULL;
}
#pragma endregion