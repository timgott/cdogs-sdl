/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin
    Copyright (C) 2003-2007 Lucas Martin-King

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright (c) 2013-2014, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "mission.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "game_events.h"
#include "gamedata.h"
#include "map.h"
#include "map_new.h"
#include "palette.h"
#include "defs.h"
#include "pic_manager.h"
#include "actors.h"


#define EXIT_WIDTH  8
#define EXIT_HEIGHT 8


// +--------------------+
// |  Color range info  |
// +--------------------+


#define WALL_COLORS       208
#define FLOOR_COLORS      216
#define ROOM_COLORS       232
#define ALT_COLORS        224

struct ColorRange {
	char name[20];
	color_t range[8];
};


static struct ColorRange cColorRanges[] =
{
	{"Red 1",		{{33,  0,  0, 255}, {29,  0,  0, 255}, {25,  0,  0, 255}, {21,  0,  0, 255}, {17,  0,  0, 255}, {13,  0,  0, 255}, { 9,  0,  0, 255}, { 5, 0,  0, 255}}},
	{"Red 2",		{{28,  0,  0, 255}, {23,  0,  0, 255}, {18,  0,  0, 255}, {14,  0,  0, 255}, {12,  0,  0, 255}, {10,  0,  0, 255}, { 8,  0,  0, 255}, { 5, 0,  0, 255}}},
	{"Red 3",		{{18,  0,  0, 255}, {15,  0,  0, 255}, {13,  0,  0, 255}, {10,  0,  0, 255}, { 8,  0,  0, 255}, { 5,  0,  0, 255}, { 3,  0,  0, 255}, { 0, 0,  0, 255}}},
	{"Green 1",		{{ 0, 33,  0, 255}, { 0, 29,  0, 255}, { 0, 25,  0, 255}, { 0, 21,  0, 255}, { 0, 17,  0, 255}, { 0, 13,  0, 255}, { 0,  9,  0, 255}, { 0, 5,  0, 255}}},
	{"Green 2",		{{ 0, 28,  0, 255}, { 0, 23,  0, 255}, { 0, 18,  0, 255}, { 0, 14,  0, 255}, { 0, 12,  0, 255}, { 0, 10,  0, 255}, { 0,  8,  0, 255}, { 0, 5,  0, 255}}},
	{"Green 3",		{{ 0, 18,  0, 255}, { 0, 15,  0, 255}, { 0, 13,  0, 255}, { 0, 10,  0, 255}, { 0,  8,  0, 255}, { 0,  5,  0, 255}, { 0,  3,  0, 255}, { 0, 0,  0, 255}}},
	{"Blue 1",		{{ 0,  0, 33, 255}, { 0,  0, 29, 255}, { 0,  0, 25, 255}, { 0,  0, 21, 255}, { 0,  0, 17, 255}, { 0,  0, 13, 255}, { 0,  0,  9, 255}, { 0, 0,  5, 255}}},
	{"Blue 2",		{{ 0,  0, 28, 255}, { 0,  0, 23, 255}, { 0,  0, 18, 255}, { 0,  0, 14, 255}, { 0,  0, 12, 255}, { 0,  0, 10, 255}, { 0,  0,  8, 255}, { 0, 0,  5, 255}}},
	{"Blue 3",		{{ 0,  0, 18, 255}, { 0,  0, 15, 255}, { 0,  0, 13, 255}, { 0,  0, 10, 255}, { 0,  0,  8, 255}, { 0,  0,  5, 255}, { 0,  0,  3, 255}, { 0, 0,  0, 255}}},
	{"Purple 1",	{{33,  0, 33, 255}, {29,  0, 29, 255}, {25,  0, 25, 255}, {21,  0, 21, 255}, {17,  0, 17, 255}, {13,  0, 13, 255}, { 9,  0,  9, 255}, { 5, 0,  5, 255}}},
	{"Purple 2",	{{28,  0, 28, 255}, {23,  0, 23, 255}, {18,  0, 18, 255}, {14,  0, 14, 255}, {12,  0, 12, 255}, {10,  0, 10, 255}, { 8,  0,  8, 255}, { 5, 0,  5, 255}}},
	{"Purple 3",	{{18,  0, 18, 255}, {15,  0, 15, 255}, {13,  0, 13, 255}, {10,  0, 10, 255}, { 8,  0,  8, 255}, { 5,  0,  5, 255}, { 3,  0,  3, 255}, { 0, 0,  0, 255}}},
	{"Gray 1",		{{33, 33, 33, 255}, {28, 28, 28, 255}, {23, 23, 23, 255}, {18, 18, 18, 255}, {15, 15, 15, 255}, {12, 12, 12, 255}, {10, 10, 10, 255}, { 8, 8,  8, 255}}},
	{"Gray 2",		{{28, 28, 28, 255}, {23, 23, 23, 255}, {18, 18, 18, 255}, {14, 14, 14, 255}, {12, 12, 12, 255}, {10, 10, 10, 255}, { 8,  8,  8, 255}, { 5, 5,  5, 255}}},
	{"Gray 3",		{{18, 18, 18, 255}, {15, 15, 15, 255}, {13, 13, 13, 255}, {10, 10, 10, 255}, { 8,  8,  8, 255}, { 5,  5,  5, 255}, { 3,  3,  3, 255}, { 0, 0,  0, 255}}},
	{"Gray/blue 1",	{{23, 23, 33, 255}, {20, 20, 29, 255}, {18, 18, 26, 255}, {15, 15, 23, 255}, {12, 12, 20, 255}, {10, 10, 17, 255}, { 8,  8, 14, 255}, { 5, 5, 10, 255}}},
	{"Gray/blue 2",	{{18, 18, 28, 255}, {16, 16, 25, 255}, {14, 14, 23, 255}, {12, 12, 20, 255}, {10, 10, 17, 255}, { 8,  8, 15, 255}, { 6,  6, 12, 255}, { 4, 4,  9, 255}}},
	{"Gray/blue 3",	{{13, 13, 23, 255}, {12, 12, 21, 255}, {10, 10, 19, 255}, { 9,  9, 17, 255}, { 7,  7, 14, 255}, { 6,  6, 12, 255}, { 4,  4, 10, 255}, { 3, 3,  8, 255}}},
	{"Yellowish 1",	{{37, 32, 11, 255}, {33, 28, 10, 255}, {29, 24,  9, 255}, {25, 21,  8, 255}, {21, 17,  7, 255}, {17, 13,  6, 255}, {14,  9,  4, 255}, {11, 6,  2, 255}}},
	{"Yellowish 2",	{{33, 28,  9, 255}, {29, 24,  8, 255}, {25, 21,  7, 255}, {21, 17,  6, 255}, {17, 13,  5, 255}, {14,  9,  4, 255}, {11,  6,  2, 255}, {11, 4,  1, 255}}},
	{"Yellowish 3",	{{29, 24,  7, 255}, {25, 21,  6, 255}, {21, 17,  5, 255}, {17, 13,  4, 255}, {14,  9,  3, 255}, {11,  6,  2, 255}, { 9,  4,  1, 255}, {11, 2,  0, 255}}},
	{"Brown 1",		{{33, 17,  0, 255}, {29, 15,  0, 255}, {25, 13,  0, 255}, {21, 11,  0, 255}, {17,  9,  0, 255}, {13,  7,  0, 255}, { 9,  5,  0, 255}, { 5, 3,  0, 255}}},
	{"Brown 2",		{{28, 14,  0, 255}, {23, 12,  0, 255}, {18,  9,  0, 255}, {14,  7,  0, 255}, {12,  6,  0, 255}, {10,  5,  0, 255}, { 8,  4,  0, 255}, { 5, 3,  0, 255}}},
	{"Brown 3",		{{18,  9,  0, 255}, {15,  8,  0, 255}, {13,  7,  0, 255}, {10,  5,  0, 255}, { 8,  4,  0, 255}, { 5,  3,  0, 255}, { 3,  2,  0, 255}, { 0, 0,  0, 255}}},
	{"Cyan 1",		{{ 0, 33, 33, 255}, { 0, 29, 29, 255}, { 0, 25, 25, 255}, { 0, 21, 21, 255}, { 0, 17, 17, 255}, { 0, 13, 13, 255}, { 0,  9,  9, 255}, { 0, 5,  5, 255}}},
	{"Cyan 2",		{{ 0, 28, 28, 255}, { 0, 23, 23, 255}, { 0, 18, 18, 255}, { 0, 14, 14, 255}, { 0, 12, 12, 255}, { 0, 10, 10, 255}, { 0,  8,  8, 255}, { 0, 5,  5, 255}}},
	{"Cyan 3",		{{ 0, 18, 18, 255}, { 0, 15, 15, 255}, { 0, 13, 13, 255}, { 0, 10, 10, 255}, { 0,  8,  8, 255}, { 0,  5,  5, 255}, { 0,  3,  3, 255}, { 0, 0,  0, 255}}}
};
#define COLORRANGE_COUNT (sizeof( cColorRanges)/sizeof( struct ColorRange))


// +--------------------+
// |  Map objects info  |
// +--------------------+


static TMapObject mapItems[] = {
	{OFSPIC_BARREL2, OFSPIC_WRECKEDBARREL2, 8, 6, 40,
	 MAPOBJ_OUTSIDE},

	{OFSPIC_BOX, OFSPIC_WRECKEDBOX, 8, 6, 20,
	 MAPOBJ_INOPEN},

	{OFSPIC_BOX2, OFSPIC_WRECKEDBOX, 8, 6, 20,
	 MAPOBJ_INOPEN},

	{OFSPIC_CABINET, OFSPIC_WRECKEDCABINET, 8, 6, 20,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR |
	 MAPOBJ_FREEINFRONT},

	{OFSPIC_PLANT, OFSPIC_WRECKEDPLANT, 4, 3, 20,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS},

	{OFSPIC_TABLE, OFSPIC_WRECKEDTABLE, 8, 6, 20,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_CHAIR, OFSPIC_WRECKEDCHAIR, 4, 3, 20,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_ROD, OFSPIC_WRECKEDCHAIR, 4, 3, 60,
	 MAPOBJ_INOPEN},

	{OFSPIC_SKULLBARREL_WOOD, OFSPIC_WRECKEDBARREL_WOOD, 8, 6, 40,
	 MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE},

	{OFSPIC_BARREL_WOOD, OFSPIC_WRECKEDBARREL_WOOD, 8, 6, 40,
	 MAPOBJ_OUTSIDE},

	{OFSPIC_GRAYBOX, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 20,
	 MAPOBJ_OUTSIDE},

	{OFSPIC_GREENBOX, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 20,
	 MAPOBJ_OUTSIDE},

	{OFSPIC_OGRESTATUE, OFSPIC_WRECKEDSAFE, 8, 6, 80,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR |
	 MAPOBJ_FREEINFRONT},

	{OFSPIC_WOODTABLE_CANDLE, OFSPIC_WRECKEDTABLE, 8, 6, 20,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_WOODTABLE, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 20,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_TREE, OFSPIC_TREE_REMAINS, 4, 3, 40,
	 MAPOBJ_INOPEN},

	{OFSPIC_BOOKSHELF, OFSPIC_WRECKEDBOX_WOOD, 8, 3, 20,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR |
	 MAPOBJ_FREEINFRONT},

	{OFSPIC_WOODENBOX, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 20,
	 MAPOBJ_OUTSIDE},

	{OFSPIC_CLOTHEDTABLE, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 20,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_STEELTABLE, OFSPIC_WRECKEDSAFE, 8, 6, 30,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_AUTUMNTREE, OFSPIC_AUTUMNTREE_REMAINS, 4, 3, 40,
	 MAPOBJ_INOPEN},

	{OFSPIC_GREENTREE, OFSPIC_GREENTREE_REMAINS, 8, 6, 40,
	 MAPOBJ_INOPEN},

// Used-to-be blow-ups

	{OFSPIC_BOX3, OFSPIC_WRECKEDBOX3, 8, 6, 40,
	 MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE | MAPOBJ_QUAKE},

	{OFSPIC_SAFE, OFSPIC_WRECKEDSAFE, 8, 6, 100,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR |
	 MAPOBJ_FREEINFRONT},

	{OFSPIC_REDBOX, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 40,
	 MAPOBJ_OUTSIDE | MAPOBJ_FLAMMABLE},

	{OFSPIC_LABTABLE, OFSPIC_WRECKEDSAFE, 8, 6, 60,
	 MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR |
	 MAPOBJ_FREEINFRONT | MAPOBJ_POISONOUS},

	{OFSPIC_TERMINAL, OFSPIC_WRECKEDBOX_WOOD, 8, 6, 40,
	 MAPOBJ_INSIDE | MAPOBJ_NOWALLS},

	{OFSPIC_BARREL, OFSPIC_WRECKEDBARREL, 8, 6, 40,
	 MAPOBJ_OUTSIDE | MAPOBJ_FLAMMABLE},

	{OFSPIC_ROCKET, OFSPIC_BURN, 8, 6, 40,
	 MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE | MAPOBJ_QUAKE},

	{OFSPIC_EGG, OFSPIC_EGG_REMAINS, 8, 6, 30,
	 (MAPOBJ_IMPASSABLE | MAPOBJ_CANBESHOT)},

	{OFSPIC_BLOODSTAIN, 0, 0, 0, 0,
	 MAPOBJ_ON_WALL},

	{OFSPIC_WALL_SKULL, 0, 0, 0, 0,
	 MAPOBJ_ON_WALL},

	{OFSPIC_BONE_N_BLOOD, 0, 0, 0, 0, 0},

	{OFSPIC_BULLET_MARKS, 0, 0, 0, 0,
	 MAPOBJ_ON_WALL},

	{OFSPIC_SKULL, 0, 0, 0, 0, 0},

	{OFSPIC_BLOOD, 0, 0, 0, 0, 0},

	{OFSPIC_SCRATCH, 0, 0, 0, 0, MAPOBJ_ON_WALL},

	{OFSPIC_WALL_STUFF, 0, 0, 0, 0, MAPOBJ_ON_WALL},

	{OFSPIC_WALL_GOO, 0, 0, 0, 0, MAPOBJ_ON_WALL},

	{OFSPIC_GOO, 0, 0, 0, 0, 0}

};
#define ITEMS_COUNT (sizeof( mapItems)/sizeof( TMapObject))


// +----------------+
// |  Pickups info  |
// +----------------+


static int pickupItems[] = {
	OFSPIC_FOLDER,
	OFSPIC_DISK1,
	OFSPIC_DISK2,
	OFSPIC_DISK3,
	OFSPIC_BLUEPRINT,
	OFSPIC_CD,
	OFSPIC_BAG,
	OFSPIC_HOLO,
	OFSPIC_BOTTLE,
	OFSPIC_RADIO,
	OFSPIC_CIRCUIT,
	OFSPIC_PAPER
};
#define PICKUPS_COUNT (sizeof( pickupItems)/sizeof( int))


// +-------------+
// |  Keys info  |
// +-------------+


static int officeKeys[4] =
    { OFSPIC_KEYCARD_YELLOW, OFSPIC_KEYCARD_GREEN, OFSPIC_KEYCARD_BLUE,
	OFSPIC_KEYCARD_RED
};

static int dungeonKeys[4] =
    { OFSPIC_KEY_YELLOW, OFSPIC_KEY_GREEN, OFSPIC_KEY_BLUE,
	OFSPIC_KEY_RED
};

static int plainKeys[4] =
    { OFSPIC_KEY3_YELLOW, OFSPIC_KEY3_GREEN, OFSPIC_KEY3_BLUE,
	OFSPIC_KEY3_RED
};

static int cubeKeys[4] =
    { OFSPIC_KEY4_YELLOW, OFSPIC_KEY4_GREEN, OFSPIC_KEY4_BLUE,
	OFSPIC_KEY4_RED
};


static int *keyStyles[] = {
	officeKeys,
	dungeonKeys,
	plainKeys,
	cubeKeys
};
#define KEYSTYLE_COUNT  (sizeof( keyStyles)/sizeof( int *))


// +-------------+
// |  Door info  |
// +-------------+

// note that the horz pic in the last pair is a TILE pic, not an offset pic!

static struct DoorPic officeDoors[6] = { {OFSPIC_DOOR, OFSPIC_VDOOR},
{OFSPIC_HDOOR_YELLOW, OFSPIC_VDOOR_YELLOW},
{OFSPIC_HDOOR_GREEN, OFSPIC_VDOOR_GREEN},
{OFSPIC_HDOOR_BLUE, OFSPIC_VDOOR_BLUE},
{OFSPIC_HDOOR_RED, OFSPIC_VDOOR_RED},
{109, OFSPIC_VDOOR_OPEN}
};

static struct DoorPic dungeonDoors[6] = { {OFSPIC_DOOR2, OFSPIC_VDOOR2},
{OFSPIC_HDOOR2_YELLOW, OFSPIC_VDOOR2_YELLOW},
{OFSPIC_HDOOR2_GREEN, OFSPIC_VDOOR2_GREEN},
{OFSPIC_HDOOR2_BLUE, OFSPIC_VDOOR2_BLUE},
{OFSPIC_HDOOR2_RED, OFSPIC_VDOOR2_RED},
{342, OFSPIC_VDOOR2_OPEN}
};

static struct DoorPic pansarDoors[6] = { {OFSPIC_HDOOR3, OFSPIC_VDOOR3},
{OFSPIC_HDOOR3_YELLOW, OFSPIC_VDOOR3_YELLOW},
{OFSPIC_HDOOR3_GREEN, OFSPIC_VDOOR3_GREEN},
{OFSPIC_HDOOR3_BLUE, OFSPIC_VDOOR3_BLUE},
{OFSPIC_HDOOR3_RED, OFSPIC_VDOOR3_RED},
{P2 + 148, OFSPIC_VDOOR2_OPEN}
};

static struct DoorPic alienDoors[6] = { {OFSPIC_HDOOR4, OFSPIC_VDOOR4},
{OFSPIC_HDOOR4_YELLOW, OFSPIC_VDOOR4_YELLOW},
{OFSPIC_HDOOR4_GREEN, OFSPIC_VDOOR4_GREEN},
{OFSPIC_HDOOR4_BLUE, OFSPIC_VDOOR4_BLUE},
{OFSPIC_HDOOR4_RED, OFSPIC_VDOOR4_RED},
{P2 + 163, OFSPIC_VDOOR2_OPEN}
};


static struct DoorPic *doorStyles[] = {
	officeDoors,
	dungeonDoors,
	pansarDoors,
	alienDoors
};

#define DOORSTYLE_COUNT (sizeof( doorStyles)/sizeof( struct DoorPic *))


// +-------------+
// |  Exit info  |
// +-------------+


static int exitPics[] = {
	375, 376,	// hazard stripes
	380, 381	// yellow plates
};

// Every exit has TWO pics, so actual # of exits == # pics / 2!
#define EXIT_COUNT (sizeof( exitPics)/sizeof( int)/2)


// +----------------+
// |  Mission info  |
// +----------------+


struct MissionOld dogFight1 = {
	"",
	"",
	WALL_STYLE_STONE, FLOOR_STYLE_STONE, FLOOR_STYLE_WOOD, 0, 1, 1,
	32, 32,
	50, 25,
	4, 2,
	0, 0, 0, 0,
	0,
	{
	 {"", 0, 0, 0, 0, 0}
	 },

	0, {0},
	0, {0},
	8,
	{8, 9, 10, 11, 12, 13, 14, 15},
	{10, 10, 10, 10, 10, 10, 10, 10},

	0, 0,
	"", "",
	14, 13, 22, 1
};

struct MissionOld dogFight2 = {
	"",
	"",
	WALL_STYLE_STEEL, FLOOR_STYLE_BLUE, FLOOR_STYLE_WHITE, 0, 0, 0,
	64, 64,
	50, 50,
	10, 3,
	0, 0, 0, 0,
	0,
	{
	 {"", 0, 0, 0, 0, 0}
	 },

	0, {0},
	0, {0},
	8,
	{0, 1, 2, 3, 4, 5, 6, 7},
	{10, 10, 10, 10, 10, 10, 10, 10},

	0, 0,
	"", "",
	5, 2, 9, 4
};


// +-----------------+
// |  Campaign info  |
// +-----------------+

#include "files.h"
#include <missions/bem.h>
#include <missions/ogre.h>


static CampaignSettingOld df1 =
{
	"Dogfight in the dungeon",
	"", "",
	1, &dogFight1,
	0, NULL
};

static CampaignSettingOld df2 =
{
	"Cubicle wars",
	"", "",
	1, &dogFight2,
	0, NULL
};


// +---------------------------------------------------+
// |  Objective colors (for automap & status display)  |
// +---------------------------------------------------+


// TODO: no limit to objective colours
color_t objectiveColors[OBJECTIVE_MAX_OLD] =
{
	{ 0, 252, 252, 255 },
	{ 252, 224, 0, 255 },
	{ 252, 0, 0, 255 },
	{ 192, 0, 192, 255 },
	{ 112, 112, 112, 255 }
};


// +-----------------------+
// |  And now the code...  |
// +-----------------------+

static void SetupBadguysForMission(Mission *mission)
{
	int i;
	CharacterStore *s = &gCampaign.Setting.characters;

	CharacterStoreResetOthers(s);

	if (s->OtherChars.size == 0)
	{
		return;
	}

	for (i = 0; i < (int)mission->Objectives.size; i++)
	{
		MissionObjective *mobj = CArrayGet(&mission->Objectives, i);
		if (mobj->Type == OBJECTIVE_RESCUE)
		{
			CharacterStoreAddPrisoner(s, mobj->Index);
			break;	// TODO: multiple prisoners
		}
	}

	for (i = 0; i < (int)mission->Enemies.size; i++)
	{
		CharacterStoreAddBaddie(s, *(int *)CArrayGet(&mission->Enemies, i));
	}

	for (i = 0; i < (int)mission->SpecialChars.size; i++)
	{
		CharacterStoreAddSpecial(
			s, *(int *)CArrayGet(&mission->SpecialChars, i));
	}
}

int SetupBuiltinCampaign(int idx)
{
	switch (idx)
	{
	case 0:
		ConvertCampaignSetting(&gCampaign.Setting, &BEM_campaign);
		break;
	case 1:
		ConvertCampaignSetting(&gCampaign.Setting, &OGRE_campaign);
		break;
	default:
		ConvertCampaignSetting(&gCampaign.Setting, &OGRE_campaign);
		return 0;
	}
	return 1;
}

int SetupBuiltinDogfight(int idx)
{
	switch (idx)
	{
	case 0:
		ConvertCampaignSetting(&gCampaign.Setting, &df1);
		break;
	case 1:
		ConvertCampaignSetting(&gCampaign.Setting, &df2);
		break;
	default:
		ConvertCampaignSetting(&gCampaign.Setting, &df1);
		return 0;
	}
	return 1;
}

// Generate a random partition of an integer `total` into a pair of ints x, y
// With the restrictions that neither x, y are less than min, and
// neither x, y are greater than max
static Vec2i GenerateRandomPairPartitionWithRestrictions(
	int total, int min, int max)
{
	Vec2i v;
	int xLow, xHigh;

	// Check for invalid input
	// Can't proceed if exactly half of total is greater than max,
	// or if total less than min
	if ((total + 1) / 2 > max || total < min)
	{
		assert(0 && "invalid random pair partition input");
		return Vec2iNew(total / 2, total - (total / 2));
	}

	// Find range of x first
	// Must be at least min, or total - max
	// Must be at most max, or total - min
	xLow = MAX(min, total - max);
	xHigh = MIN(max, total - min);
	v.x = xLow + (rand() % (xHigh - xLow + 1));
	v.y = total - v.x;
	assert(v.x >= min);
	assert(v.y >= min);
	assert(v.x <= max);
	assert(v.y <= max);
	return v;
}

static Vec2i GenerateQuickPlayMapSize(QuickPlayQuantity size)
{
	const int minMapDim = 16;
	const int maxMapDim = 64;
	// Map sizes based on total dimensions (width + height)
	// Small: 32 - 64
	// Medium: 64 - 96
	// Large: 96 - 128
	// Restrictions: at least 16, at most 64 per side
	switch (size)
	{
	case QUICKPLAY_QUANTITY_ANY:
		return GenerateRandomPairPartitionWithRestrictions(
			32 + (rand() % (128 - 32 + 1)),
			minMapDim, maxMapDim);
	case QUICKPLAY_QUANTITY_SMALL:
		return GenerateRandomPairPartitionWithRestrictions(
			32 + (rand() % (64 - 32 + 1)),
			minMapDim, maxMapDim);
	case QUICKPLAY_QUANTITY_MEDIUM:
		return GenerateRandomPairPartitionWithRestrictions(
			64 + (rand() % (96 - 64 + 1)),
			minMapDim, maxMapDim);
	case QUICKPLAY_QUANTITY_LARGE:
		return GenerateRandomPairPartitionWithRestrictions(
			96 + (rand() % (128 - 96 + 1)),
			minMapDim, maxMapDim);
	default:
		assert(0 && "invalid quick play map size config");
		return Vec2iZero();
	}
}

// Generate a quick play parameter based on the quantity setting, and various
// thresholds
// e.g. if qty is "small", generate random number between small and medium
static int GenerateQuickPlayParam(
	QuickPlayQuantity qty, int small, int medium, int large, int max)
{
	switch (qty)
	{
	case QUICKPLAY_QUANTITY_ANY:
		return small + (rand() % (max - small + 1));
	case QUICKPLAY_QUANTITY_SMALL:
		return small + (rand() % (medium - small + 1));
	case QUICKPLAY_QUANTITY_MEDIUM:
		return medium + (rand() % (large - medium + 1));
	case QUICKPLAY_QUANTITY_LARGE:
		return large + (rand() % (max - large + 1));
	default:
		assert(0);
		return 0;
	}
}

static void SetupQuickPlayEnemy(
	Character *enemy, const QuickPlayConfig *config, gun_e gun)
{
	enemy->looks.armedBody = BODY_ARMED;
	enemy->looks.unarmedBody = BODY_UNARMED;
	enemy->looks.face = rand() % FACE_COUNT;
	enemy->gun = gun;
	enemy->speed =
		GenerateQuickPlayParam(config->EnemySpeed, 64, 112, 160, 256);
	if (IsShortRange(enemy->gun))
	{
		enemy->speed = enemy->speed * 4 / 3;
	}
	if (IsShortRange(enemy->gun))
	{
		enemy->bot.probabilityToMove = 35 + (rand() % 35);
	}
	else
	{
		enemy->bot.probabilityToMove = 30 + (rand() % 30);
	}
	enemy->bot.probabilityToTrack = 10 + (rand() % 60);
	if (enemy->gun == GUN_KNIFE)
	{
		enemy->bot.probabilityToShoot = 0;
	}
	else if (IsHighDPS(enemy->gun))
	{
		enemy->bot.probabilityToShoot = 2 + (rand() % 10);
	}
	else
	{
		enemy->bot.probabilityToShoot = 15 + (rand() % 30);
	}
	enemy->bot.actionDelay = rand() % (50 + 1);
	enemy->looks.skin = rand() % SHADE_COUNT;
	enemy->looks.arm = rand() % SHADE_COUNT;
	enemy->looks.body = rand() % SHADE_COUNT;
	enemy->looks.leg = rand() % SHADE_COUNT;
	enemy->looks.hair = rand() % SHADE_COUNT;
	enemy->maxHealth =
		GenerateQuickPlayParam(config->EnemyHealth, 10, 20, 40, 60);
	enemy->flags = 0;
}

static void SetupQuickPlayEnemies(
	Mission *mission,
	int numEnemies,
	CharacterStore *store,
	const QuickPlayConfig *config)
{
	int i;
	for (i = 0; i < numEnemies; i++)
	{
		Character *ch;
		gun_e gun;
		CArrayPushBack(&mission->Enemies, &i);

		for (;;)
		{
			gun = rand() % GUN_COUNT;
			// make at least one of each type of enemy:
			// - Short range weapon
			// - Long range weapon
			// - High explosive weapon
			if (i == 0 && !IsShortRange(gun))
			{
				continue;
			}
			if (i == 1 && !IsLongRange(gun))
			{
				continue;
			}
			if (i == 2 && config->EnemiesWithExplosives && !IsHighDPS(gun))
			{
				continue;
			}

			if (!config->EnemiesWithExplosives && IsHighDPS(gun))
			{
				continue;
			}
			break;
		}
		ch = CharacterStoreAddOther(store);
		SetupQuickPlayEnemy(ch, config, gun);
		CharacterSetLooks(ch, &ch->looks);
	}
}

void SetupQuickPlayCampaign(
	CampaignSetting *setting, const QuickPlayConfig *config)
{
	int i;
	Mission *m;
	int c;
	CMALLOC(m, sizeof *m);
	MissionInit(m);
	m->WallStyle = rand() % WALL_STYLE_COUNT;
	m->FloorStyle = rand() % FLOOR_STYLE_COUNT;
	m->RoomStyle = rand() % FLOOR_STYLE_COUNT;
	m->ExitStyle = rand() % EXIT_COUNT;
	m->KeyStyle = rand() % KEYSTYLE_COUNT;
	m->DoorStyle = rand() % DOORSTYLE_COUNT;
	m->Size = GenerateQuickPlayMapSize(config->MapSize);
	m->Type = MAPTYPE_CLASSIC;	// TODO: generate different map types
	switch (m->Type)
	{
	case MAPTYPE_CLASSIC:
		m->u.Classic.Walls =
			GenerateQuickPlayParam(config->WallCount, 0, 5, 15, 30);
		m->u.Classic.WallLength =
			GenerateQuickPlayParam(config->WallLength, 1, 3, 6, 12);
		m->u.Classic.Rooms =
			GenerateQuickPlayParam(config->RoomCount, 0, 2, 5, 12);
		m->u.Classic.Squares =
			GenerateQuickPlayParam(config->SquareCount, 0, 1, 3, 6);
		break;
	default:
		assert(0 && "unknown map type");
		break;
	}
	CharacterStoreInit(&setting->characters);
	c = GenerateQuickPlayParam(config->EnemyCount, 3, 5, 8, 12);
	SetupQuickPlayEnemies(m, c, &setting->characters, config);

	c = GenerateQuickPlayParam(config->ItemCount, 0, 2, 5, 10);
	for (i = 0; i < c; i++)
	{
		int n = rand() % (ITEMS_MAX - 1 + 1);
		CArrayPushBack(&m->Items, &n);
		n = GenerateQuickPlayParam(config->ItemCount, 0, 5, 10, 20);
		CArrayPushBack(&m->ItemDensities, &n);
	}
	m->EnemyDensity = (40 + (rand() % 20)) / m->Enemies.size;
	for (i = 0; i < WEAPON_MAX; i++)
	{
		CArrayPushBack(&m->Weapons, &i);
	}
	m->WallColor = rand() % (COLORRANGE_COUNT - 1 + 1);
	m->FloorColor = rand() % (COLORRANGE_COUNT - 1 + 1);
	m->RoomColor = rand() % (COLORRANGE_COUNT - 1 + 1);
	m->AltColor = rand() % (COLORRANGE_COUNT - 1 + 1);

	CFREE(setting->Title);
	CSTRDUP(setting->Title, "Quick play");
	CFREE(setting->Author);
	CSTRDUP(setting->Author, "");
	CFREE(setting->Description);
	CSTRDUP(setting->Description, "");
	CArrayPushBack(&setting->Missions, m);
}

static void SetupObjectives(struct MissionOptions *mo, Mission *mission)
{
	int i;
	for (i = 0; i < (int)mission->Objectives.size; i++)
	{
		MissionObjective *mobj = CArrayGet(&mission->Objectives, i);
		struct Objective o;
		memset(&o, 0, sizeof o);
		assert(i < OBJECTIVE_MAX_OLD);
		o.color = objectiveColors[i];
		o.blowupObject = &mapItems[mobj->Index % ITEMS_COUNT];
		o.pickupItem = pickupItems[mobj->Index % PICKUPS_COUNT];
		CArrayPushBack(&mo->Objectives, &o);
	}
}

static void CleanupPlayerInventory(struct PlayerData *data, CArray *weapons)
{
	int i;
	for (i = data->weaponCount - 1; i >= 0; i--)
	{
		int j;
		int hasWeapon = 0;
		for (j = 0; j < (int)weapons->size; j++)
		{
			if (data->weapons[i] == *(int *)CArrayGet(weapons, j))
			{
				hasWeapon = 1;
				break;
			}
		}
		if (!hasWeapon)
		{
			for (j = i + 1; j < data->weaponCount; j++)
			{
				data->weapons[j - 1] = data->weapons[j];
			}
			data->weaponCount--;
		}
	}
}

static void SetupWeapons(struct MissionOptions *mo, CArray *weapons)
{
	int i;
	for (i = 0; i < (int)weapons->size; i++)
	{
		CArrayPushBack(&mo->AvailableWeapons, CArrayGet(weapons, i));
	}
	// Now remove unavailable weapons from players inventories
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		CleanupPlayerInventory(&gPlayerDatas[i], weapons);
	}
}

void SetRange(int start, int range)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		gPicManager.palette[start + i] = cColorRanges[range].range[i];
	}
	CDogsSetPalette(gPicManager.palette);
}

void SetupMission(int idx, int buildTables, CampaignOptions *campaign)
{
	int i;
	int x, y;
	Mission *m;

	MissionOptionsInit(&gMission);
	gMission.index = idx;
	m = CArrayGet(&campaign->Setting.Missions, idx);
	gMission.missionData = m;
	gMission.doorPics =
	    doorStyles[abs(m->DoorStyle) % DOORSTYLE_COUNT];
	gMission.keyPics = keyStyles[abs(m->KeyStyle) % KEYSTYLE_COUNT];
	for (i = 0; i < (int)m->Items.size; i++)
	{
		CArrayPushBack(
			&gMission.MapObjects,
			&mapItems[*(int32_t *)CArrayGet(&m->Items, i)]);
	}

	srand(10 * idx + campaign->seed);

	gMission.exitPic = exitPics[2 * (abs(m->ExitStyle) % EXIT_COUNT)];
	gMission.exitShadow =
	    exitPics[2 * (abs(m->ExitStyle) % EXIT_COUNT) + 1];

	if (m->Size.x)
	{
		x = (rand() % (abs(m->Size.x) - EXIT_WIDTH)) +
			(XMAX - abs(m->Size.x)) / 2;
	}
	else
	{
		x = rand() % (XMAX - EXIT_WIDTH);
	}
	if (m->Size.y)
	{
		y = (rand() % (abs(m->Size.y) - EXIT_HEIGHT)) +
			(YMAX - abs(m->Size.y)) / 2;
	}
	else
	{
		y = rand() % (YMAX - EXIT_HEIGHT);
	}
	gMission.exitLeft = x;
	gMission.exitRight = x + EXIT_WIDTH + 1;
	gMission.exitTop = y;
	gMission.exitBottom = y + EXIT_HEIGHT + 1;

	SetupObjectives(&gMission, m);
	SetupBadguysForMission(m);
	SetupWeapons(&gMission, &m->Weapons);
	SetPaletteRanges(m->WallColor, m->FloorColor, m->RoomColor, m->AltColor);
	if (buildTables)
	{
		BuildTranslationTables(gPicManager.palette);
	}
}

void SetPaletteRanges(int wall_range, int floor_range, int room_range, int alt_range)
{
	SetRange(WALL_COLORS, abs(wall_range) % COLORRANGE_COUNT);
	SetRange(FLOOR_COLORS, abs(floor_range) % COLORRANGE_COUNT);
	SetRange(ROOM_COLORS, abs(room_range) % COLORRANGE_COUNT);
	SetRange(ALT_COLORS, abs(alt_range) % COLORRANGE_COUNT);
}

void MissionSetMessageIfComplete(struct MissionOptions *options)
{
	if (CanCompleteMission(options))
	{
		GameEvent msg;
		msg.Type = GAME_EVENT_SET_MESSAGE;
		strcpy(msg.u.SetMessage.Message, "Mission Complete");
		msg.u.SetMessage.Ticks = -1;
		GameEventsEnqueue(&gGameEvents, msg);
	}
}

int CheckMissionObjective(
	struct MissionOptions *options, int flags, ObjectiveType type)
{
	int idx;
	MissionObjective *mobj;
	struct Objective *o;
	if (!(flags & TILEITEM_OBJECTIVE))
	{
		return 0;
	}
	idx = ObjectiveFromTileItem(flags);
	mobj = CArrayGet(&options->missionData->Objectives, idx);
	if (mobj->Type != type)
	{
		return 0;
	}
	o = CArrayGet(&options->Objectives, idx);
	o->done++;
	MissionSetMessageIfComplete(options);
	return 1;
}

int CanCompleteMission(struct MissionOptions *options)
{
	int i;

	// Death is the only escape from dogfights and quick play
	if (gCampaign.Entry.mode == CAMPAIGN_MODE_DOGFIGHT)
	{
		return GetNumPlayersAlive() <= 1;
	}
	else if (gCampaign.Entry.mode == CAMPAIGN_MODE_QUICK_PLAY)
	{
		return GetNumPlayersAlive() == 0;
	}

	// Check all objective counts are enough
	for (i = 0; i < (int)options->Objectives.size; i++)
	{
		struct Objective *o = CArrayGet(&options->Objectives, i);
		MissionObjective *mobj =
			CArrayGet(&options->missionData->Objectives, i);
		if (o->done < mobj->Required)
		{
			return 0;
		}
	}

	return 1;
}

int IsMissionComplete(struct MissionOptions *options)
{
	int rescuesRequired = 0;
	int i;

	if (!CanCompleteMission(options))
	{
		return 0;
	}

	// Check if dogfight is complete
	if (gCampaign.Entry.mode == CAMPAIGN_MODE_DOGFIGHT &&
		GetNumPlayersAlive() <= 1)
	{
		return 1;
	}

	// Check that all surviving players are in exit zone
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (gPlayers[i] && !IsTileInExit(&gPlayers[i]->tileItem, options))
		{
			return 0;
		}
	}

	// Find number of rescues required
	// TODO: support multiple rescue objectives
	for (i = 0; i < (int)options->missionData->Objectives.size; i++)
	{
		MissionObjective *mobj =
			CArrayGet(&options->missionData->Objectives, i);
		if (mobj->Type == OBJECTIVE_RESCUE)
		{
			rescuesRequired = mobj->Required;
			break;
		}
	}
	// Check that enough prisoners are in exit zone
	if (rescuesRequired > 0)
	{
		int prisonersRescued = 0;
		TActor *a = ActorList();
		while (a != NULL)
		{
			if (a->character == CharacterStoreGetPrisoner(
				&gCampaign.Setting.characters, 0) &&
				IsTileInExit(&a->tileItem, options))
			{
				prisonersRescued++;
			}
			a = a->next;
		}
		if (prisonersRescued < rescuesRequired)
		{
			return 0;
		}
	}

	return 1;
}

struct EditorInfo GetEditorInfo(void)
{
	struct EditorInfo ei;
	ei.itemCount = ITEMS_COUNT;
	ei.pickupCount = PICKUPS_COUNT;
	ei.keyCount = KEYSTYLE_COUNT;
	ei.doorCount = DOORSTYLE_COUNT;
	ei.exitCount = EXIT_COUNT;
	ei.rangeCount = COLORRANGE_COUNT;
	return ei;
}

const char *RangeName(int idx)
{
	if (idx >= 0 && idx < (int)COLORRANGE_COUNT)
	{
		return cColorRanges[idx].name;
	}
	else
	{
		return "Invalid";
	}
}