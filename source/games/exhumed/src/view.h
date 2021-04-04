//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#pragma once 
#include "build.h"

BEGIN_PS_NS

extern short bSubTitles;
extern short besttarget;
extern bool bCamera;

void DrawStatusBar();
void DrawView(double smoothRatio, bool sceneonly = false);
void ResetView();
void NoClip();
void Clip();

extern short dVertPan[];
extern short nQuake[];
extern int nCamerax;
extern int nCameray;
extern int nCameraz;
extern short bTouchFloor;
extern short nChunkTotal;
extern int gFov;

static inline int angle_interpolate16(int a, int b, int smooth)
{
    return a + MulScale(((b+1024-a)&2047)-1024, smooth, 16);
}

END_PS_NS
