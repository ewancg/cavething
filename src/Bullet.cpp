#include "Bullet.h"
#include "Draw.h"
#include "Caret.h"
#include "NpChar.h"
#include "Game.h"

BULLET_TABLE gBulTbl[46] =
{
	{0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
	{4, 1, 20, 36, 4, 4, 2, 2, {8, 8, 8, 8}},
	{6, 1, 23, 36, 4, 4, 2, 2, {8, 8, 8, 8}},
	{8, 1, 30, 36, 4, 4, 2, 2, {8, 8, 8, 8}},
	{1, 1, 8, 32, 6, 6, 2, 2, {8, 8, 8, 8}},
	{2, 1, 12, 32, 6, 6, 2, 2, {8, 8, 8, 8}},
	{4, 1, 16, 32, 6, 6, 2, 2, {8, 8, 8, 8}},
	{2, 2, 100, 8, 8, 16, 4, 2, {8, 8, 8, 8}},
	{3, 2, 100, 8, 4, 4, 4, 2, {8, 8, 8, 8}},
	{3, 2, 100, 8, 4, 4, 4, 2, {8, 8, 8, 8}},
	{2, 1, 20, 32, 2, 2, 2, 2, {8, 8, 8, 8}},
	{4, 1, 20, 32, 2, 2, 2, 2, {8, 8, 8, 8}},
	{6, 1, 20, 32, 2, 2, 2, 2, {8, 8, 8, 8}},
	{0, 10, 50, 40, 2, 2, 2, 2, {8, 8, 8, 8}},
	{0, 10, 70, 40, 4, 4, 4, 4, {8, 8, 8, 8}},
	{0, 10, 90, 40, 4, 4, 0, 0, {8, 8, 8, 8}},
	{1, 100, 100, 20, 16, 16, 0, 0, {0, 0, 0, 0}},
	{1, 100, 100, 20, 16, 16, 0, 0, {0, 0, 0, 0}},
	{1, 100, 100, 20, 16, 16, 0, 0, {0, 0, 0, 0}},
	{1, 1, 20, 8, 2, 2, 2, 2, {4, 4, 4, 4}},
	{2, 1, 20, 8, 2, 2, 2, 2, {4, 4, 4, 4}},
	{2, 1, 20, 8, 4, 4, 4, 4, {4, 4, 4, 4}},
	{3, 1, 32, 32, 2, 2, 2, 2, {4, 4, 4, 4}},
	{0, 100, 0, 36, 8, 8, 8, 8, {12, 12, 12, 12}},
	{127, 1, 2, 4, 8, 4, 8, 4, {0, 0, 0, 0}},
	{15, 1, 30, 36, 8, 8, 4, 2, {8, 8, 8, 8}},
	{6, 3, 18, 36, 10, 10, 4, 2, {12, 12, 12, 12}},
	{1, 100, 30, 36, 6, 6, 4, 4, {12, 12, 12, 12}},
	{0, 10, 30, 40, 2, 2, 2, 2, {8, 8, 8, 8}},
	{0, 10, 40, 40, 4, 4, 4, 4, {8, 8, 8, 8}},
	{0, 10, 40, 40, 4, 4, 0, 0, {8, 8, 8, 8}},
	{2, 100, 100, 20, 12, 12, 0, 0, {0, 0, 0, 0}},
	{2, 100, 100, 20, 12, 12, 0, 0, {0, 0, 0, 0}},
	{2, 100, 100, 20, 12, 12, 0, 0, {0, 0, 0, 0}},
	{4, 4, 20, 32, 4, 4, 3, 3, {8, 8, 24, 8}},
	{4, 2, 20, 32, 2, 2, 2, 2, {8, 8, 24, 8}},
	{1, 1, 20, 32, 2, 2, 2, 2, {8, 8, 24, 8}},
	{4, 4, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},
	{8, 8, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},
	{12, 12, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},
	{3, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},
	{6, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},
	{11, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},
	{4, 4, 20, 32, 4, 4, 3, 3, {8, 8, 24, 8}},
	{0, 4, 4, 4, 0, 0, 0, 0, {0, 0, 0, 0}},
	{1, 1, 1, 36, 1, 1, 1, 1, {1, 1, 1, 1}}
};

BULLET gBul[BULLET_MAX];

void InitBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
		gBul[i].cond = 0;
}

int CountArmsBullet(int arms_code)
{
	int count = 0;
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (gBul[i].cond & 0x80 && (gBul[i].code_bullet + 2) / 3 == arms_code)
			++count;
	}
	
	return count;
}

int CountBulletNum(int bullet_code)
{
	int count = 0;
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (gBul[i].cond & 0x80 && gBul[i].code_bullet == bullet_code)
			++count;
	}
	
	return count;
}

void DeleteBullet(int code)
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (gBul[i].cond & 0x80)
		{
			if ((gBul[i].code_bullet + 2) / 3 == code)
				gBul[i].cond = 0;
		}
	}
}

void ClearBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
		gBul[i].cond = 0;
}

void PutBullet(int fx, int fy)
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (gBul[i].cond & 0x80)
		{
			int x, y;
			
			switch (gBul[i].direct)
			{
				case 0:
					x = gBul[i].x - gBul[i].view.front;
					y = gBul[i].y - gBul[i].view.top;
					break;
				case 1:
					x = gBul[i].x - gBul[i].view.top;
					y = gBul[i].y - gBul[i].view.front;
					break;
				case 2:
					x = gBul[i].x - gBul[i].view.back;
					y = gBul[i].y - gBul[i].view.top;
					break;
				case 3:
					x = gBul[i].x - gBul[i].view.top;
					y = gBul[i].y - gBul[i].view.back;
					break;
			}
			
			PutBitmap3(&grcGame, x / 0x200 - fx / 0x200, y / 0x200 - fy / 0x200, &gBul[i].rect, 17);
		}
	}
}

void SetBullet(int no, int x, int y, int dir)
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (!(gBul[i].cond & 0x80))
		{
			memset(&gBul[i], 0, sizeof(BULLET));
			gBul[i].code_bullet = no;
			gBul[i].cond = 0x80;
			gBul[i].direct = dir;
			gBul[i].damage = gBulTbl[no].damage;
			gBul[i].life = gBulTbl[no].life;
			gBul[i].life_count = gBulTbl[no].life_count;
			gBul[i].bbits = gBulTbl[no].bbits;
			gBul[i].enemyXL = gBulTbl[no].enemyXL << 9;
			gBul[i].enemyYL = gBulTbl[no].enemyYL << 9;
			gBul[i].blockXL = gBulTbl[no].blockXL << 9;
			gBul[i].blockYL = gBulTbl[no].blockYL << 9;
			gBul[i].view.back = gBulTbl[no].view.back << 9;
			gBul[i].view.front = gBulTbl[no].view.front << 9;
			gBul[i].view.top = gBulTbl[no].view.top << 9;
			gBul[i].view.bottom = gBulTbl[no].view.bottom << 9;
			gBul[i].x = x;
			gBul[i].y = y;
			break;
		}
	}
}

void ActBullet_PoleStar(BULLET *bul, int level)
{
	if (++bul->count1 <= bul->life_count)
	{
		if (bul->act_no)
		{
			//Move
			bul->x += bul->xm;
			bul->y += bul->ym;
		}
		else
		{
			bul->act_no = 1;
			
			//Set speed
			switch (bul->direct)
			{
				case 0:
					bul->xm = -0x1000;
					break;
				case 1:
					bul->ym = -0x1000;
					break;
				case 2:
					bul->xm = 0x1000;
					break;
				case 3:
					bul->ym = 0x1000;
					break;
			}
			
			//Set hitbox
			if (level == 1)
			{
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x400;
						break;
					case 1:
						bul->enemyXL = 0x400;
						break;
					case 2:
						bul->enemyYL = 0x400;
						break;
					case 3:
						bul->enemyXL = 0x400;
						break;
				}
			}
			else if (level == 2)
			{
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x800;
						break;
					case 1:
						bul->enemyXL = 0x800;
						break;
					case 2:
						bul->enemyYL = 0x800;
						break;
					case 3:
						bul->enemyXL = 0x800;
						break;
				}
			}
			
			//Set framerect
			switch (level)
			{
				case 1:
					if (bul->direct != 1 && bul->direct != 3)
						bul->rect = {128, 32, 144, 48};
					else
						bul->rect = {144, 32, 160, 48};
					break;
				case 2:
					if (bul->direct != 1 && bul->direct != 3)
						bul->rect = {160, 32, 176, 48};
					else
						bul->rect = {176, 32, 192, 48};
					break;
				case 3:
					if (bul->direct != 1 && bul->direct != 3)
						bul->rect = {128, 48, 144, 64};
					else
						bul->rect = {144, 48, 160, 64};
					break;
			}
		}
	}
	else
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
	}
}

void ActBullet_MachineGun(BULLET *bul, int level)
{
	RECT rect1[4];
	RECT rect2[4];
	RECT rect3[4];
	
	rect1[0] = {64, 0, 80, 16};
	rect1[1] = {80, 0, 96, 16};
	rect1[2] = {96, 0, 112, 16};
	rect1[3] = {112, 0, 128, 16};
	rect2[0] = {64, 16, 80, 32};
	rect2[1] = {80, 16, 96, 32};
	rect2[2] = {96, 16, 112, 32};
	rect2[3] = {112, 16, 128, 32};
	rect3[0] = {64, 32, 80, 48};
	rect3[1] = {80, 32, 96, 48};
	rect3[2] = {96, 32, 112, 48};
	rect3[3] = {112, 32, 128, 48};
	
	if (++bul->count1 <= bul->life_count)
	{
		if (bul->act_no)
		{
			bul->x += bul->xm;
			bul->y += bul->ym;
			
			switch ( level )
			{
				case 1:
					bul->rect = rect1[bul->direct];
					break;
				case 2:
					bul->rect = rect2[bul->direct];
					if (bul->direct != 1 && bul->direct != 3)
						SetNpChar(127, bul->x, bul->y, 0, 0, 0, 0, 256);
					else
						SetNpChar(127, bul->x, bul->y, 0, 0, 1, 0, 256);
					break;
				case 3:
					bul->rect = rect3[bul->direct];
					SetNpChar(128, bul->x, bul->y, 0, 0, bul->direct, 0, 256);
					break;
			}
		}
		else
		{
			int move;
			switch (level)
			{
				case 1:
					move = 0x1000;
					break;
				case 2:
					move = 0x1000;
					break;
				case 3:
					move = 0x1000;
					break;
			}
			
			bul->act_no = 1;
			
			switch (bul->direct)
			{
				case 0:
					bul->xm = -move;
					bul->ym = Random(-0xAA, 0xAA);
					break;
				case 1:
					bul->xm = Random(-0xAA, 0xAA);
					bul->ym = -move;
					break;
				case 2:
					bul->xm = move;
					bul->ym = Random(-0xAA, 0xAA);
					break;
				case 3:
					bul->xm = Random(-0xAA, 0xAA);
					bul->ym = move;
					break;
			}
		}
	}
	else
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
	}
}

void ActBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (gBul[i].cond & 0x80)
		{
			if (gBul[i].life > 0)
			{
				switch (gBul[i].code_bullet)
				{
					case 4:
						ActBullet_PoleStar(&gBul[i], 1);
						break;
					case 5:
						ActBullet_PoleStar(&gBul[i], 2);
						break;
					case 6:
						ActBullet_PoleStar(&gBul[i], 3);
						break;
					case 10:
						ActBullet_MachineGun(&gBul[i], 1);
						break;
					case 11:
						ActBullet_MachineGun(&gBul[i], 2);
						break;
					case 12:
						ActBullet_MachineGun(&gBul[i], 3);
						break;
				}
			}
			else
			{
				gBul[i].cond = 0;
			}
		}
	}
}
