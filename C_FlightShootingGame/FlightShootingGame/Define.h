#pragma once
#pragma comment (lib, "winmm.lib")
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <mmsystem.h>
#include <Digitalv.h>

#pragma region GameLogic
void StartGame();
void SetGameInfo();
void InitInGame();
void StartInGame();
void EndGame();

void OpenWindow(enum EWindowType type);
void DrawTitleWindow();
void DrawResultWindow();
#pragma endregion

#pragma region Data
void SaveFileData();
void LoadFileData();

void AddScore(int value);
#pragma endregion

#pragma region Stage
void InitStage();
void StartStage(int stageId);
void UpdateStage();
#pragma endregion

#pragma region Player
void InitPlayer();
void UpdatePlayer();

void DrawPlayer();
void ErasePlayer();

void OnPlayerAction(enum EPlayerActionType type);
void CheckPlayerCollision();
void OnPlayerHit();

void PlayerEffect();
#pragma endregion

#pragma region Item
void OnGetItem(enum EItemType type);

void InitItem();
void UpdateItem();
void SpawnItem(enum EItemType type, int y, int x);
void MoveItem(struct _ITEM* item);
void DrawItem(struct _ITEM* item);
void EraseItem(struct _ITEM* item);
#pragma endregion

#pragma region Enemy
void InitEnemy();
void UpdateEnemy();

void MoveEnemy(struct _ENEMY* enemy);
void SpawnEnemy(struct _ENEMY* enemy);

void DrawEnemy();
void EraseEnemy(struct _ENEMY* enemy);

void OnEnemyHit(struct _ENEMY* enemy);
#pragma endregion

#pragma region Bullet
void InitBullet();
void SpawnPlayerBullets();
void SpawnPlayerBullet(int x);
void UpdateBullet();
void ShotPlayerBullet(struct _BULLET* bullet);

void SpawnEnemyBullet(struct _ENEMY* enemy);
void ShotEnemyBullet(struct _BULLET* bullet);
void ShotEnemyTiltBullet(struct _TILTBULLET* bullet);

void DrawBullet(struct _BULLET* bullet);
void DrawTiltBullet(struct _TILTBULLET* bullet);
void EraseBullet(struct _BULLET* bullet);
void EraseTiltBullet(struct _TILTBULLET* bullet);
#pragma endregion

#pragma region Input
void UpdateInput();
void OnEscKey();
#pragma endregion

#pragma region IngameUI
void UpdateUI();
void SetUIText(int stageId, int life, int power, int speed, int score, int maxScore);
#pragma endregion

#pragma region Screen
void InitScreen();
void UpdateScreen();
void ClearScreen();
void CharPrintScreen(int y, int x, char* string, enum ColorType colorType);
void LinePrintScreen(int y, char* string, enum ColorType colorType);
void DividingLinePrintScreen(int y, enum ColorType colorType);
void FlippingScreen();
void ReleaseScreen(); 
void SetColor(unsigned short color);
#pragma endregion

#pragma region Util
void PlayBgmSound(enum EWindowType type);

bool IsFormulaFillTick(int* tick, float value);
bool IsEmptySpaceCheck(int y, int x, int size);

enum EObjectType GetCustomObjectType(char findCh);
struct _ENEMY* FindEnemyObject(int y, int x);
struct _ITEM* FindItemObject(int y, int x);
struct _BULLET* FindPlayerBullet(int y, int x);
struct _BULLET* FindEnemyBullet(int y, int x);
#pragma endregion