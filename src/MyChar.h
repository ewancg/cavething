#pragma once
#include <stdint.h>
#include "WindowsWrapper.h"

struct MYCHAR
{
	uint8_t cond;
	unsigned int flag;
	int direct;
	int up;
	int down;
	int unit;
	int equip;
	int x;
	int y;
	int tgt_x;
	int tgt_y;
	int index_x;
	int index_y;
	int xm;
	int ym;
	int ani_wait;
	int ani_no;
	RECT hit;
	RECT view;
	RECT rect;
	RECT rect_arms;
	int level;
	int exp_wait;
	int exp_count;
	uint8_t shock;
	uint8_t no_life;
	uint8_t rensha;
	uint8_t bubble;
	int16_t life;
	int16_t star;
	int16_t max_life;
	int16_t a;
	int lifeBr;
	int lifeBr_count;
	int air;
	int air_get;
	char sprash;
	char ques;
	char boost_sw;
	int boost_cnt;
};

extern MYCHAR gMC;

void InitMyChar();
void AnimationMyChar(bool bKey);
void ShowMyChar(bool bShow);
void GetMyCharPosition(int *x, int *y);
void SetMyCharPosition(int x, int y);
void MoveMyChar(int x, int y);
