//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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

#include "ns.h"	// Must come before everything else!

#include "compat.h"
#include "build.h"

#include "blood.h"

BEGIN_BLD_NS

static void sub_72580(DBloodActor *);
static void sub_725A4(DBloodActor *);
static void sub_72850(DBloodActor *);
static void sub_72934(DBloodActor *);


AISTATE tchernobogIdle = { kAiStateIdle, 0, -1, 0, NULL, NULL, sub_725A4, NULL };
AISTATE tchernobogSearch = { kAiStateSearch, 8, -1, 1800, NULL, aiMoveForward, sub_72580, &tchernobogIdle };
AISTATE tchernobogChase = { kAiStateChase, 8, -1, 0, NULL, aiMoveForward, sub_72934, NULL };
AISTATE tchernobogRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &tchernobogSearch };
AISTATE tcherno13A9B8 = { kAiStateMove, 8, -1, 600, NULL, aiMoveForward, sub_72850, &tchernobogIdle };
AISTATE tcherno13A9D4 = { kAiStateMove, 6, dword_279B54, 60, NULL, NULL, NULL, &tchernobogChase };
AISTATE tcherno13A9F0 = { kAiStateChase, 6, dword_279B58, 60, NULL, NULL, NULL, &tchernobogChase };
AISTATE tcherno13AA0C = { kAiStateChase, 7, dword_279B5C, 60, NULL, NULL, NULL, &tchernobogChase };
AISTATE tcherno13AA28 = { kAiStateChase, 8, -1, 60, NULL, aiMoveTurn, NULL, &tchernobogChase };

void sub_71A90(int, DBloodActor* actor)
{
    XSPRITE* pXSprite = &actor->x();
    spritetype* pSprite = &actor->s();
    spritetype *pTarget = &sprite[pXSprite->target];
    XSPRITE *pXTarget = &xsprite[pTarget->extra];
    int nTarget = pTarget->index;
    int nOwner = pSprite->owner;
    if (pXTarget->burnTime == 0)
        evPost(nTarget, 3, 0, kCallbackFXFlameLick);
    actBurnSprite(nOwner, pXTarget, 40);
    if (Chance(0x6000))
        aiNewState(actor, &tcherno13A9D4);
}

void sub_71BD4(int, DBloodActor* actor)
{
    XSPRITE* pXSprite = &actor->x();
    spritetype* pSprite = &actor->s();
    DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
    int height = pSprite->yrepeat*pDudeInfo->eyeHeight;
    ///assert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    if (!(pXSprite->target >= 0 && pXSprite->target < kMaxSprites)) {
        Printf(PRINT_HIGH, "pXSprite->target >= 0 && pXSprite->target < kMaxSprites");
        return;
    }
    int x = pSprite->x;
    int y = pSprite->y;
    int z = height;
    TARGETTRACK tt = { 0x10000, 0x10000, 0x100, 0x55, 0x100000 };
    Aim aim;
    aim.dx = CosScale16(pSprite->ang);
    aim.dy = SinScale16(pSprite->ang);
    aim.dz = actor->dudeSlope;
    int nClosest = 0x7fffffff;
    int nSprite2;
    StatIterator it(kStatDude);
    while ((nSprite2 = it.NextIndex()) >= 0)
    {
        spritetype *pSprite2 = &sprite[nSprite2];
        if (pSprite == pSprite2 || !(pSprite2->flags&8))
            continue;
        int x2 = pSprite2->x;
        int y2 = pSprite2->y;
        int z2 = pSprite2->z;
        int nDist = approxDist(x2-x, y2-y);
        if (nDist == 0 || nDist > 0x2800)
            continue;
        if (tt.at10)
        {
            int t = DivScale(nDist, tt.at10, 12);
            x2 += (xvel[nSprite2]*t)>>12;
            y2 += (yvel[nSprite2]*t)>>12;
            z2 += (zvel[nSprite2]*t)>>8;
        }
        int tx = x+MulScale(Cos(pSprite->ang), nDist, 30);
        int ty = y+MulScale(Sin(pSprite->ang), nDist, 30);
        int tz = z+MulScale(actor->dudeSlope, nDist, 10);
        int tsr = MulScale(9460, nDist, 10);
        int top, bottom;
        GetSpriteExtents(pSprite2, &top, &bottom);
        if (tz-tsr > bottom || tz+tsr < top)
            continue;
        int dx = (tx-x2)>>4;
        int dy = (ty-y2)>>4;
        int dz = (tz-z2)>>8;
        int nDist2 = ksqrt(dx*dx+dy*dy+dz*dz);
        if (nDist2 < nClosest)
        {
            int nAngle = getangle(x2-x, y2-y);
            int nDeltaAngle = ((nAngle-pSprite->ang+1024)&2047)-1024;
            if (abs(nDeltaAngle) <= tt.at8)
            {
                int tz = pSprite2->z-pSprite->z;
                if (cansee(x, y, z, pSprite->sectnum, x2, y2, z2, pSprite2->sectnum))
                {
                    nClosest = nDist2;
                    aim.dx = CosScale16(nAngle);
                    aim.dy = SinScale16(nAngle);
                    aim.dz = DivScale(tz, nDist, 10);
                }
                else
                    aim.dz = tz;
            }
        }
    }
    actFireMissile(pSprite, -350, 0, aim.dx, aim.dy, aim.dz, kMissileFireballTchernobog);
    actFireMissile(pSprite, 350, 0, aim.dx, aim.dy, aim.dz, kMissileFireballTchernobog);
}

void sub_720AC(int, DBloodActor* actor)
{
    XSPRITE* pXSprite = &actor->x();
    spritetype* pSprite = &actor->s();
    ///assert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    if (!(pXSprite->target >= 0 && pXSprite->target < kMaxSprites)) {
        Printf(PRINT_HIGH, "pXSprite->target >= 0 && pXSprite->target < kMaxSprites");
        return;
    }
    DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
    int height = pSprite->yrepeat*pDudeInfo->eyeHeight;
    int ax, ay, az;
    ax = CosScale16(pSprite->ang);
    ay = SinScale16(pSprite->ang);
    int x = pSprite->x;
    int y = pSprite->y;
    int z = height;
    TARGETTRACK tt = { 0x10000, 0x10000, 0x100, 0x55, 0x100000 };
    Aim aim;
    aim.dx = ax;
    aim.dy = ay;
    aim.dz = actor->dudeSlope;
    int nClosest = 0x7fffffff;
    az = 0;
    int nSprite2;
    StatIterator it(kStatDude);
    while ((nSprite2 = it.NextIndex()) >= 0)
    {
        spritetype *pSprite2 = &sprite[nSprite2];
        if (pSprite == pSprite2 || !(pSprite2->flags&8))
            continue;
        int x2 = pSprite2->x;
        int y2 = pSprite2->y;
        int z2 = pSprite2->z;
        int nDist = approxDist(x2-x, y2-y);
        if (nDist == 0 || nDist > 0x2800)
            continue;
        if (tt.at10)
        {
            int t = DivScale(nDist, tt.at10, 12);
            x2 += (xvel[nSprite2]*t)>>12;
            y2 += (yvel[nSprite2]*t)>>12;
            z2 += (zvel[nSprite2]*t)>>8;
        }
        int tx = x+MulScale(Cos(pSprite->ang), nDist, 30);
        int ty = y+MulScale(Sin(pSprite->ang), nDist, 30);
        int tz = z+MulScale(actor->dudeSlope, nDist, 10);
        int tsr = MulScale(9460, nDist, 10);
        int top, bottom;
        GetSpriteExtents(pSprite2, &top, &bottom);
        if (tz-tsr > bottom || tz+tsr < top)
            continue;
        int dx = (tx-x2)>>4;
        int dy = (ty-y2)>>4;
        int dz = (tz-z2)>>8;
        int nDist2 = ksqrt(dx*dx+dy*dy+dz*dz);
        if (nDist2 < nClosest)
        {
            int nAngle = getangle(x2-x, y2-y);
            int nDeltaAngle = ((nAngle-pSprite->ang+1024)&2047)-1024;
            if (abs(nDeltaAngle) <= tt.at8)
            {
                int tz = pSprite2->z-pSprite->z;
                if (cansee(x, y, z, pSprite->sectnum, x2, y2, z2, pSprite2->sectnum))
                {
                    nClosest = nDist2;
                    aim.dx = CosScale16(nAngle);
                    aim.dy = SinScale16(nAngle);
                    aim.dz = DivScale(tz, nDist, 10);
                }
                else
                    aim.dz = tz;
            }
        }
    }
    actFireMissile(pSprite, 350, 0, aim.dx, aim.dy, -aim.dz, kMissileFireballTchernobog);
    actFireMissile(pSprite, -350, 0, ax, ay, az, kMissileFireballTchernobog);
}

static void sub_72580(DBloodActor* actor)
{
    auto pXSprite = &actor->x();
    auto pSprite = &actor->s();
    aiChooseDirection(pSprite, pXSprite, pXSprite->goalAng);
    aiThinkTarget(actor);
}

static void sub_725A4(DBloodActor* actor)
{
    auto pXSprite = &actor->x();
    auto pSprite = &actor->s();
    ///assert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    if (!(pSprite->type >= kDudeBase && pSprite->type < kDudeMax)) {
        Printf(PRINT_HIGH, "pSprite->type >= kDudeBase && pSprite->type < kDudeMax");
        return;
    }
    DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
    DUDEEXTRA_at6_u2 *pDudeExtraE = &actor->dudeExtra.at6.u2;
    if (pDudeExtraE->xval2 && pDudeExtraE->xval1 < 10)
        pDudeExtraE->xval1++;
    else if (pDudeExtraE->xval1 >= 10 && pDudeExtraE->xval2)
    {
        pXSprite->goalAng += 256;
        POINT3D *pTarget = &baseSprite[pSprite->index];
        aiSetTarget(pXSprite, pTarget->x, pTarget->y, pTarget->z);
        aiNewState(actor, &tcherno13AA28);
        return;
    }
    if (Chance(pDudeInfo->alertChance))
    {
        for (int p = connecthead; p >= 0; p = connectpoint2[p])
        {
            PLAYER *pPlayer = &gPlayer[p];
            if (pPlayer->pXSprite->health == 0 || powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
                continue;
            int x = pPlayer->pSprite->x;
            int y = pPlayer->pSprite->y;
            int z = pPlayer->pSprite->z;
            int nSector = pPlayer->pSprite->sectnum;
            int dx = x-pSprite->x;
            int dy = y-pSprite->y;
            int nDist = approxDist(dx, dy);
            if (nDist > pDudeInfo->seeDist && nDist > pDudeInfo->hearDist)
                continue;
            if (!cansee(x, y, z, nSector, pSprite->x, pSprite->y, pSprite->z-((pDudeInfo->eyeHeight*pSprite->yrepeat)<<2), pSprite->sectnum))
                continue;
            int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
            if (nDist < pDudeInfo->seeDist && abs(nDeltaAngle) <= pDudeInfo->periphery)
            {
                pDudeExtraE->xval1 = 0;
                aiSetTarget(pXSprite, pPlayer->nSprite);
                aiActivateDude(&bloodActors[pXSprite->reference]);
            }
            else if (nDist < pDudeInfo->hearDist)
            {
                pDudeExtraE->xval1 = 0;
                aiSetTarget(pXSprite, x, y, z);
                aiActivateDude(&bloodActors[pXSprite->reference]);
            }
            else
                continue;
            break;
        }
    }
}

static void sub_72850(DBloodActor* actor)
{
    auto pXSprite = &actor->x();
    auto pSprite = &actor->s();
    ///assert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    if (!(pSprite->type >= kDudeBase && pSprite->type < kDudeMax)) {
        Printf(PRINT_HIGH, "pSprite->type >= kDudeBase && pSprite->type < kDudeMax");
        return;
    }
    DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
    int dx = pXSprite->targetX-pSprite->x;
    int dy = pXSprite->targetY-pSprite->y;
    int nAngle = getangle(dx, dy);
    int nDist = approxDist(dx, dy);
    aiChooseDirection(pSprite, pXSprite, nAngle);
    if (nDist < 512 && abs(pSprite->ang - nAngle) < pDudeInfo->periphery)
        aiNewState(actor, &tchernobogSearch);
    aiThinkTarget(actor);
}

static void sub_72934(DBloodActor* actor)
{
    auto pXSprite = &actor->x();
    auto pSprite = &actor->s();
    if (pXSprite->target == -1)
    {
        aiNewState(actor, &tcherno13A9B8);
        return;
    }
    ///assert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    if (!(pSprite->type >= kDudeBase && pSprite->type < kDudeMax)) {
        Printf(PRINT_HIGH, "pSprite->type >= kDudeBase && pSprite->type < kDudeMax");
        return;
    }
    DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
    ///assert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    if (!(pXSprite->target >= 0 && pXSprite->target < kMaxSprites)) {
        Printf(PRINT_HIGH, "pXSprite->target >= 0 && pXSprite->target < kMaxSprites");
        return;
    }
    spritetype *pTarget = &sprite[pXSprite->target];
    XSPRITE *pXTarget = &xsprite[pTarget->extra];
    int dx = pTarget->x-pSprite->x;
    int dy = pTarget->y-pSprite->y;
    aiChooseDirection(pSprite, pXSprite, getangle(dx, dy));
    if (pXTarget->health == 0)
    {
        aiNewState(actor, &tchernobogSearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], kPwUpShadowCloak) > 0)
    {
        aiNewState(actor, &tchernobogSearch);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->seeDist)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->eyeHeight*pSprite->yrepeat)<<2;
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            if (nDist < pDudeInfo->seeDist && abs(nDeltaAngle) <= pDudeInfo->periphery)
            {
                aiSetTarget(pXSprite, pXSprite->target);
                if (nDist < 0x1f00 && nDist > 0xd00 && abs(nDeltaAngle) < 85)
                    aiNewState(actor, &tcherno13AA0C);
                else if (nDist < 0xd00 && nDist > 0xb00 && abs(nDeltaAngle) < 85)
                    aiNewState(actor, &tcherno13A9D4);
                else if (nDist < 0xb00 && nDist > 0x500 && abs(nDeltaAngle) < 85)
                    aiNewState(actor, &tcherno13A9F0);
                return;
            }
        }
    }

    aiNewState(actor, &tcherno13A9B8);
    pXSprite->target = -1;
}

END_BLD_NS
