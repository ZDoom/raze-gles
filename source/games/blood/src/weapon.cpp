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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "build.h"

#include "blood.h"
#include "bloodactor.h"

BEGIN_BLD_NS

void FirePitchfork(int, PLAYER *pPlayer);
void FireSpray(int, PLAYER *pPlayer);
void ThrowCan(int, PLAYER *pPlayer);
void DropCan(int, PLAYER *pPlayer);
void ExplodeCan(int, PLAYER *pPlayer);
void ThrowBundle(int, PLAYER *pPlayer);
void DropBundle(int, PLAYER *pPlayer);
void ExplodeBundle(int, PLAYER *pPlayer);
void ThrowProx(int, PLAYER *pPlayer);
void DropProx(int, PLAYER *pPlayer);
void ThrowRemote(int, PLAYER *pPlayer);
void DropRemote(int, PLAYER *pPlayer);
void FireRemote(int, PLAYER *pPlayer);
void FireShotgun(int nTrigger, PLAYER *pPlayer);
void EjectShell(int, PLAYER *pPlayer);
void FireTommy(int nTrigger, PLAYER *pPlayer);
void FireSpread(int nTrigger, PLAYER *pPlayer);
void AltFireSpread(int nTrigger, PLAYER *pPlayer);
void AltFireSpread2(int nTrigger, PLAYER *pPlayer);
void FireFlare(int nTrigger, PLAYER *pPlayer);
void AltFireFlare(int nTrigger, PLAYER *pPlayer);
void FireVoodoo(int nTrigger, PLAYER *pPlayer);
void AltFireVoodoo(int nTrigger, PLAYER *pPlayer);
void DropVoodoo(int nTrigger, PLAYER *pPlayer);
void FireTesla(int nTrigger, PLAYER *pPlayer);
void AltFireTesla(int nTrigger, PLAYER *pPlayer);
void FireNapalm(int nTrigger, PLAYER *pPlayer);
void FireNapalm2(int nTrigger, PLAYER *pPlayer);
void AltFireNapalm(int nTrigger, PLAYER *pPlayer);
void FireLifeLeech(int nTrigger, PLAYER *pPlayer);
void AltFireLifeLeech(int nTrigger, PLAYER *pPlayer);
void FireBeast(int nTrigger, PLAYER * pPlayer);

typedef void(*QAVTypeCast)(int, void *);

void (*qavClientCallback[])(int, void*) =
{
(QAVTypeCast)FirePitchfork,
(QAVTypeCast)FireSpray,
(QAVTypeCast)ThrowCan,
(QAVTypeCast)DropCan,
(QAVTypeCast)ExplodeCan,
(QAVTypeCast)ThrowBundle,
(QAVTypeCast)DropBundle,
(QAVTypeCast)ExplodeBundle,
(QAVTypeCast)ThrowProx,
(QAVTypeCast)DropProx,
(QAVTypeCast)ThrowRemote,
(QAVTypeCast)DropRemote,
(QAVTypeCast)FireRemote,
(QAVTypeCast)FireShotgun,
(QAVTypeCast)EjectShell,
(QAVTypeCast)FireTommy,
(QAVTypeCast)AltFireSpread2,
(QAVTypeCast)FireSpread,
(QAVTypeCast)AltFireSpread,
(QAVTypeCast)FireFlare,
(QAVTypeCast)AltFireFlare,
(QAVTypeCast)FireVoodoo,
(QAVTypeCast)AltFireVoodoo,
(QAVTypeCast)FireTesla,
(QAVTypeCast)AltFireTesla,
(QAVTypeCast)FireNapalm,
(QAVTypeCast)FireNapalm2,
(QAVTypeCast)FireLifeLeech,
(QAVTypeCast)FireBeast,
(QAVTypeCast)AltFireLifeLeech,
(QAVTypeCast)DropVoodoo,
(QAVTypeCast)AltFireNapalm,
};

enum
{
    nClientFirePitchfork,
    nClientFireSpray,
    nClientThrowCan,
    nClientDropCan,
    nClientExplodeCan,
    nClientThrowBundle,
    nClientDropBundle,
    nClientExplodeBundle,
    nClientThrowProx,
    nClientDropProx,
    nClientThrowRemote,
    nClientDropRemote,
    nClientFireRemote,
    nClientFireShotgun,
    nClientEjectShell,
    nClientFireTommy,
    nClientAltFireSpread2,
    nClientFireSpread,
    nClientAltFireSpread,
    nClientFireFlare,
    nClientAltFireFlare,
    nClientFireVoodoo,
    nClientAltFireVoodoo,
    nClientFireTesla,
    nClientAltFireTesla,
    nClientFireNapalm,
    nClientFireNapalm2,
    nClientFireLifeLeech,
    nClientFireBeast,
    nClientAltFireLifeLeech,
    nClientDropVoodoo,
    nClientAltFireNapalm,
};

static bool checkFired6or7(PLAYER *pPlayer)
{
    switch (pPlayer->curWeapon)
    {
    case kWeapSpraycan:
        switch (pPlayer->weaponState)
        {
        case 5:
        case 6:
            return 1;
        case 7:
            if (VanillaMode())
                return 0;
            return 1;
        }
        break;
    case kWeapDynamite:
        switch (pPlayer->weaponState)
        {
        case 4:
        case 5:
        case 6:
            return 1;
        }
        break;
    }
    return 0;
}

static bool BannedUnderwater(int nWeapon)
{
    return nWeapon == kWeapSpraycan || nWeapon == kWeapDynamite;
}

static bool CheckWeaponAmmo(PLAYER *pPlayer, int weapon, int ammotype, int count)
{
    if (gInfiniteAmmo)
        return 1;
    if (ammotype == -1)
        return 1;
    if (weapon == kWeapRemote && pPlayer->weaponAmmo == 11 && pPlayer->weaponState == 11)
        return 1;
    if (weapon == kWeapLifeLeech && pPlayer->pXSprite->health > 0)
        return 1;
    return pPlayer->ammoCount[ammotype] >= count;
}

static bool CheckAmmo(PLAYER *pPlayer, int ammotype, int count)
{
    if (gInfiniteAmmo)
        return 1;
    if (ammotype == -1)
        return 1;
    if (pPlayer->curWeapon == kWeapRemote && pPlayer->weaponAmmo == 11 && pPlayer->weaponState == 11)
        return 1;
    if (pPlayer->curWeapon == kWeapLifeLeech && pPlayer->pXSprite->health >= unsigned(count<<4))
        return 1;
    return pPlayer->ammoCount[ammotype] >= count;
}

static bool checkAmmo2(PLAYER *pPlayer, int ammotype, int amount)
{
    if (gInfiniteAmmo)
        return 1;
    if (ammotype == -1)
        return 1;
    return pPlayer->ammoCount[ammotype] >= amount;
}

void SpawnBulletEject(PLAYER *pPlayer, int a2, int a3)
{
    POSTURE *pPosture = &pPlayer->pPosture[pPlayer->lifeMode][pPlayer->posture];
    pPlayer->zView = pPlayer->pSprite->z-pPosture->eyeAboveZ;
    int dz = pPlayer->zWeapon-(pPlayer->zWeapon-pPlayer->zView)/2;
    fxSpawnEjectingBrass(pPlayer->pSprite, dz, a2, a3);
}

void SpawnShellEject(PLAYER *pPlayer, int a2, int a3)
{
    POSTURE *pPosture = &pPlayer->pPosture[pPlayer->lifeMode][pPlayer->posture];
    pPlayer->zView = pPlayer->pSprite->z-pPosture->eyeAboveZ;
    int t = pPlayer->zWeapon - pPlayer->zView;
    int dz = pPlayer->zWeapon-t+(t>>2);
    fxSpawnEjectingShell(pPlayer->pSprite, dz, a2, a3);
}

void WeaponInit(void)
{
    for (int i = 0; i < kQAVEnd; i++)
    {
        auto pQAV = getQAV(i);
        if (!pQAV)
            I_Error("Could not load QAV %d\n", i);
        pQAV->nSprite = -1;
    }
}

void WeaponPrecache()
{
    for (int i = 0; i < kQAVEnd; i++)
    {
        auto pQAV = getQAV(i);
        if (pQAV)
            pQAV->Precache();
    }
}

void WeaponDraw(PLAYER *pPlayer, int shade, double xpos, double ypos, int palnum)
{
    assert(pPlayer != NULL);
    if (pPlayer->weaponQav == kQAVNone)
        return;
    auto pQAV = getQAV(pPlayer->weaponQav);
    int duration;
    double smoothratio;

    qavProcessTimer(pPlayer, pQAV, &duration, &smoothratio, pPlayer->weaponState == -1 || (pPlayer->curWeapon == kWeapShotgun && pPlayer->weaponState == 7));

    pQAV->x = int(xpos);
    pQAV->y = int(ypos);
    int flags = 2;
    int nInv = powerupCheck(pPlayer, kPwUpShadowCloak);
    if (nInv >= 120 * 8 || (nInv != 0 && (PlayClock & 32)))
    {
        shade = -128;
        flags |= 1;
    }
    pQAV->Draw(xpos, ypos, duration, flags, shade, palnum, true, smoothratio);
}

void WeaponPlay(PLAYER *pPlayer)
{
    assert(pPlayer != NULL);
    if (pPlayer->weaponQav == kQAVNone)
        return;
    auto pQAV = getQAV(pPlayer->weaponQav);
    pQAV->nSprite = pPlayer->pSprite->index;
    int nTicks = pQAV->duration - pPlayer->weaponTimer;
    pQAV->Play(nTicks-4, nTicks, pPlayer->qavCallback, pPlayer);
}

static void StartQAV(PLAYER *pPlayer, int nWeaponQAV, int callback = -1, bool looped = false)
{
    assert(nWeaponQAV < kQAVEnd);
    auto res_id = qavGetCorrectID(nWeaponQAV);
    auto pQAV = getQAV(res_id);
    pPlayer->weaponQav = res_id;
    pPlayer->weaponTimer = pQAV->duration;
    pPlayer->qavCallback = callback;
    pPlayer->qavLoop = looped;
    pPlayer->qavLastTick = I_GetTime(pQAV->ticrate);
    pPlayer->qavTimer = pQAV->duration;
    //pQAV->Preload();
    WeaponPlay(pPlayer);
    pPlayer->weaponTimer -= 4;
}

struct WEAPONTRACK
{
    int aimSpeedHorz;
    int aimSpeedVert;
    int angleRange;
    int thingAngle;
    int seeker;
    bool bIsProjectile;
};

WEAPONTRACK gWeaponTrack[] = {
    { 0, 0, 0, 0, 0, false },
    { 0x6000, 0x6000, 0x71, 0x55, 0x111111, false },
    { 0x8000, 0x8000, 0x71, 0x55, 0x2aaaaa, true },
    { 0x10000, 0x10000, 0x38, 0x1c, 0, false },
    { 0x6000, 0x8000, 0x38, 0x1c, 0, false },
    { 0x6000, 0x6000, 0x38, 0x1c, 0x2aaaaa, true },
    { 0x6000, 0x6000, 0x71, 0x55, 0, true },
    { 0x6000, 0x6000, 0x71, 0x38, 0, true },
    { 0x8000, 0x10000, 0x71, 0x55, 0x255555, true },
    { 0x10000, 0x10000, 0x71, 0, 0, true },
    { 0x10000, 0x10000, 0xaa, 0, 0, false },
    { 0x6000, 0x6000, 0x71, 0x55, 0, true },
    { 0x6000, 0x6000, 0x71, 0x55, 0, true },
    { 0x6000, 0x6000, 0x71, 0x55, 0, false },
};

void UpdateAimVector(PLAYER * pPlayer)
{
    spritetype *pSprite;
    assert(pPlayer != NULL);
    spritetype *pPSprite = pPlayer->pSprite;
    int x = pPSprite->x;
    int y = pPSprite->y;
    int z = pPlayer->zWeapon;
    Aim aim;
    aim.dx = CosScale16(pPSprite->ang);
    aim.dy = SinScale16(pPSprite->ang);
    aim.dz = pPlayer->slope;
    WEAPONTRACK *pWeaponTrack = &gWeaponTrack[pPlayer->curWeapon];
    int nTarget = -1;
    pPlayer->aimTargetsCount = 0;
    int autoaim = Autoaim(pPlayer->nPlayer);
    if (autoaim == 1 || (autoaim == 2 && !pWeaponTrack->bIsProjectile) || pPlayer->curWeapon == kWeapVoodooDoll || pPlayer->curWeapon == kWeapLifeLeech)
    {
        int nClosest = 0x7fffffff;
        int nSprite;
        StatIterator it(kStatDude);
        while ((nSprite = it.NextIndex()) >= 0)
        {
            pSprite = &sprite[nSprite];
            if (pSprite == pPSprite)
                continue;
            if (!gGameOptions.bFriendlyFire && IsTargetTeammate(pPlayer, pSprite))
                continue;
            if (pSprite->flags&32)
                continue;
            if (!(pSprite->flags&8))
                continue;
            int x2 = pSprite->x;
            int y2 = pSprite->y;
            int z2 = pSprite->z;
            int nDist = approxDist(x2-x, y2-y);
            if (nDist == 0 || nDist > 51200)
                continue;
            if (pWeaponTrack->seeker)
            {
                int t = DivScale(nDist,pWeaponTrack->seeker, 12);
                x2 += (xvel[nSprite]*t)>>12;
                y2 += (yvel[nSprite]*t)>>12;
                z2 += (zvel[nSprite]*t)>>8;
            }
            int lx = x + MulScale(Cos(pPSprite->ang), nDist, 30);
            int ly = y + MulScale(Sin(pPSprite->ang), nDist, 30);
            int lz = z + MulScale(pPlayer->slope, nDist, 10);
            int zRange = MulScale(9460, nDist, 10);
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            if (lz-zRange>bottom || lz+zRange<top)
                continue;
            int angle = getangle(x2-x,y2-y);
            if (abs(((angle-pPSprite->ang+1024)&2047)-1024) > pWeaponTrack->angleRange)
                continue;
            if (pPlayer->aimTargetsCount < 16 && cansee(x,y,z,pPSprite->sectnum,x2,y2,z2,pSprite->sectnum))
                pPlayer->aimTargets[pPlayer->aimTargetsCount++] = nSprite;
            // Inlined?
            int dz = (lz-z2)>>8;
            int dy = (ly-y2)>>4;
            int dx = (lx-x2)>>4;
            int nDist2 = ksqrt(dx*dx+dy*dy+dz*dz);
            if (nDist2 >= nClosest)
                continue;
            DUDEINFO *pDudeInfo = getDudeInfo(pSprite->type);
            int center = (pSprite->yrepeat*pDudeInfo->aimHeight)<<2;
            int dzCenter = (z2-center)-z;
            if (cansee(x, y, z, pPSprite->sectnum, x2, y2, z2, pSprite->sectnum))
            {
                nClosest = nDist2;
                aim.dx = CosScale16(angle);
                aim.dy = SinScale16(angle);
                aim.dz = DivScale(dzCenter, nDist, 10);
                nTarget = nSprite;
            }
        }
        if (pWeaponTrack->thingAngle > 0)
        {
            int nSprite;
            StatIterator it(kStatThing);
            while ((nSprite = it.NextIndex()) >= 0)
            {
                pSprite = &sprite[nSprite];
                if (!gGameOptions.bFriendlyFire && IsTargetTeammate(pPlayer, pSprite))
                    continue;
                if (!(pSprite->flags&8))
                    continue;
                int x2 = pSprite->x;
                int y2 = pSprite->y;
                int z2 = pSprite->z;
                int dx = x2-x;
                int dy = y2-y;
                int dz = z2-z;
                int nDist = approxDist(dx, dy);
                if (nDist == 0 || nDist > 51200)
                    continue;
                int lx = x + MulScale(Cos(pPSprite->ang), nDist, 30);
                int ly = y + MulScale(Sin(pPSprite->ang), nDist, 30);
                int lz = z + MulScale(pPlayer->slope, nDist, 10);
                int zRange = MulScale(9460, nDist, 10);
                int top, bottom;
                GetSpriteExtents(pSprite, &top, &bottom);
                if (lz-zRange>bottom || lz+zRange<top)
                    continue;
                int angle = getangle(dx,dy);
                if (abs(((angle-pPSprite->ang+1024)&2047)-1024) > pWeaponTrack->thingAngle)
                    continue;
                if (pPlayer->aimTargetsCount < 16 && cansee(x,y,z,pPSprite->sectnum,pSprite->x,pSprite->y,pSprite->z,pSprite->sectnum))
                    pPlayer->aimTargets[pPlayer->aimTargetsCount++] = nSprite;
                // Inlined?
                int dz2 = (lz-z2)>>8;
                int dy2 = (ly-y2)>>4;
                int dx2 = (lx-x2)>>4;
                int nDist2 = ksqrt(dx2*dx2+dy2*dy2+dz2*dz2);
                if (nDist2 >= nClosest)
                    continue;
                if (cansee(x, y, z, pPSprite->sectnum, pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum))
                {
                    nClosest = nDist2;
                    aim.dx = CosScale16(angle);
                    aim.dy = SinScale16(angle);
                    aim.dz = DivScale(dz, nDist, 10);
                    nTarget = nSprite;
                }
            }
        }
    }
    Aim aim2;
    aim2 = aim;
    RotateVector((int*)&aim2.dx, (int*)&aim2.dy, -pPSprite->ang);
    aim2.dz -= pPlayer->slope;
    pPlayer->relAim.dx = interpolatedvalue(pPlayer->relAim.dx, aim2.dx, pWeaponTrack->aimSpeedHorz);
    pPlayer->relAim.dy = interpolatedvalue(pPlayer->relAim.dy, aim2.dy, pWeaponTrack->aimSpeedHorz);
    pPlayer->relAim.dz = interpolatedvalue(pPlayer->relAim.dz, aim2.dz, pWeaponTrack->aimSpeedVert);
    pPlayer->aim = pPlayer->relAim;
    RotateVector((int*)&pPlayer->aim.dx, (int*)&pPlayer->aim.dy, pPSprite->ang);
    pPlayer->aim.dz += pPlayer->slope;
    pPlayer->aimTarget = nTarget;
}

struct t_WeaponModes
{
    int update;
    int ammoType;
};

t_WeaponModes weaponModes[] = {
    { 0, -1 },
    { 1, -1 },
    { 1, 1 },
    { 1, 2 },
    { 1, 3 },
    { 1, 4 },
    { 1, 5 },
    { 1, 6 },
    { 1, 7 },
    { 1, 8 },
    { 1, 9 },
    { 1, 10 },
    { 1, 11 },
    { 0, -1 },
};

void WeaponRaise(PLAYER *pPlayer)
{
    assert(pPlayer != NULL);
    int prevWeapon = pPlayer->curWeapon;
    pPlayer->curWeapon = pPlayer->newWeapon;
    pPlayer->newWeapon = kWeapNone;
    pPlayer->weaponAmmo = weaponModes[pPlayer->curWeapon].ammoType;
    switch (pPlayer->curWeapon)
    {
    case kWeapPitchFork:
        pPlayer->weaponState = 0;
        StartQAV(pPlayer, kQAVFORKUP);
        break;
    case kWeapSpraycan:
        if (pPlayer->weaponState == 2)
        {
            pPlayer->weaponState = 3;
            StartQAV(pPlayer, kQAVCANPREF);
        }
        else
        {
            pPlayer->weaponState = 0;
            StartQAV(pPlayer, kQAVLITEOPEN);
        }
        break;
    case kWeapDynamite:
        if (gInfiniteAmmo || checkAmmo2(pPlayer, 5, 1))
        {
            pPlayer->weaponState = 3;
            if (prevWeapon == kWeapSpraycan)
                StartQAV(pPlayer, kQAVBUNUP);
            else
                StartQAV(pPlayer, kQAVBUNUP2);
        }
        break;
    case kWeapProximity:
        if (gInfiniteAmmo || checkAmmo2(pPlayer, 10, 1))
        {
            pPlayer->weaponState = 7;
            StartQAV(pPlayer, kQAVPROXUP);
        }
        break;
    case kWeapRemote:
        if (gInfiniteAmmo || checkAmmo2(pPlayer, 11, 1))
        {
            pPlayer->weaponState = 10;
            StartQAV(pPlayer, kQAVREMUP2);
        }
        else
        {
            StartQAV(pPlayer, kQAVREMUP3);
            pPlayer->weaponState = 11;
        }
        break;
    case kWeapShotgun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns))
        {
            if (gInfiniteAmmo || pPlayer->ammoCount[2] >= 4)
                StartQAV(pPlayer, kQAV2SHOTUP);
            else
                StartQAV(pPlayer, kQAVSHOTUP);
            if (gInfiniteAmmo || pPlayer->ammoCount[2] >= 4)
                pPlayer->weaponState = 7;
            else if (pPlayer->ammoCount[2] > 1)
                pPlayer->weaponState = 3;
            else if (pPlayer->ammoCount[2] > 0)
                pPlayer->weaponState = 2;
            else
                pPlayer->weaponState = 1;
        }
        else
        {
            if (gInfiniteAmmo || pPlayer->ammoCount[2] > 1)
                pPlayer->weaponState = 3;
            else if (pPlayer->ammoCount[2] > 0)
                pPlayer->weaponState = 2;
            else
                pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVSHOTUP);
        }
        break;
    case kWeapTommyGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 3, 2))
        {
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAV2TOMUP);
        }
        else
        {
            pPlayer->weaponState = 0;
            StartQAV(pPlayer, kQAVTOMUP);
        }
        break;
    case kWeapVoodooDoll:
        if (gInfiniteAmmo || checkAmmo2(pPlayer, 9, 1))
        {
            pPlayer->weaponState = 2;
            StartQAV(pPlayer, kQAVVDUP);
        }
        break;
    case kWeapFlareGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 1, 2))
        {
            StartQAV(pPlayer, kQAVFLAR2UP);
            pPlayer->weaponState = 3;
        }
        else
        {
            StartQAV(pPlayer, kQAVFLARUP);
            pPlayer->weaponState = 2;
        }
        break;
    case kWeapTeslaCannon:
        if (checkAmmo2(pPlayer, 7, 1))
        {
            pPlayer->weaponState = 2;
            if (powerupCheck(pPlayer, kPwUpTwoGuns))
                StartQAV(pPlayer, kQAV2SGUNUP);
            else
                StartQAV(pPlayer, kQAVSGUNUP);
        }
        else
        {
            pPlayer->weaponState = 3;
            StartQAV(pPlayer, kQAVSGUNUP);
        }
        break;
    case kWeapNapalm:
        if (powerupCheck(pPlayer, kPwUpTwoGuns))
        {
            StartQAV(pPlayer, kQAV2NAPUP);
            pPlayer->weaponState = 3;
        }
        else
        {
            StartQAV(pPlayer, kQAVNAPUP);
            pPlayer->weaponState = 2;
        }
        break;
    case kWeapLifeLeech:
        pPlayer->weaponState = 2;
        StartQAV(pPlayer, kQAVSTAFUP);
        break;
    case kWeapBeast:
        pPlayer->weaponState = 2;
        StartQAV(pPlayer, kQAVBSTUP);
        break;
    }
}

void WeaponLower(PLAYER *pPlayer)
{
    assert(pPlayer != NULL);
    if (checkFired6or7(pPlayer))
        return;
    pPlayer->throwPower = 0;
    int prevState = pPlayer->weaponState;
    switch (pPlayer->curWeapon)
    {
    case kWeapPitchFork:
        StartQAV(pPlayer, kQAVFORKDOWN);
        break;
    case kWeapSpraycan:
        sfxKill3DSound(pPlayer->pSprite, -1, 441);
        switch (prevState)
        {
        case 1:
            if (VanillaMode())
            {
                StartQAV(pPlayer, kQAVLITECLO2);
            }
            else
            {
                if (pPlayer->newWeapon == kWeapDynamite) // do not put away lighter if TNT was selected while throwing a spray can
                {
                    pPlayer->weaponState = 2;
                    StartQAV(pPlayer, kQAVCANDOWN);
                    WeaponRaise(pPlayer);
                    return;
                }
            }
            break;
        case 2:
            pPlayer->weaponState = 1;
            WeaponRaise(pPlayer);
            return;
        case 4:
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVCANDOWN);
            if (VanillaMode())
            {
                pPlayer->newWeapon = kWeapNone;
                WeaponLower(pPlayer);
            }
            else
            {
                if (pPlayer->newWeapon == kWeapDynamite)
                {
                    pPlayer->weaponState = 2;
                    StartQAV(pPlayer, kQAVCANDOWN);
                    return;
                }
                else
                {
                    WeaponLower(pPlayer);
                }
            }
            break;
        case 3:
            if (pPlayer->newWeapon == kWeapDynamite)
            {
                pPlayer->weaponState = 2;
                StartQAV(pPlayer, kQAVCANDOWN);
                return;
            }
            else if (pPlayer->newWeapon == kWeapSpraycan)
            {
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVCANDOWN);
                pPlayer->newWeapon = kWeapNone;
                WeaponLower(pPlayer);
            }
            else
            {
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVCANDOWN);
            }
            break;
        case 7: // throwing ignited alt fire spray
            if (VanillaMode() || (pPlayer->newWeapon != 0))
                break;
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, 11, -1, 0);
            break;
        }
        break;
    case kWeapDynamite:
        switch (prevState)
        {
        case 1:
            if (!VanillaMode() && (pPlayer->newWeapon == kWeapSpraycan)) // do not put away lighter after TNT is thrown if while throwing the weapon was switched already to spray
            {
                pPlayer->weaponState = 2;
                StartQAV(pPlayer, kQAVBUNDOWN);
                WeaponRaise(pPlayer);
                return;
            }
            StartQAV(pPlayer, kQAVLITECLO2);
            break;
        case 2:
            WeaponRaise(pPlayer);
            break;
        case 3:
            if (pPlayer->newWeapon == kWeapSpraycan)
            {
                pPlayer->weaponState = 2;
                StartQAV(pPlayer, kQAVBUNDOWN);
            }
            else
            {
                StartQAV(pPlayer, kQAVBUNDOWN2);
            }
            break;
        default:
            break;
        }
        break;
    case kWeapProximity:
        switch (prevState)
        {
        case 7:
            StartQAV(pPlayer, kQAVPROXDOWN);
            break;
        }
        break;
    case kWeapRemote:
        switch (prevState)
        {
        case 10:
            StartQAV(pPlayer, kQAVREMDOWN2);
            break;
        case 11:
            StartQAV(pPlayer, kQAVREMDOWN3);
            break;
        }
        break;
    case kWeapShotgun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns))
            StartQAV(pPlayer, kQAV2SHOTDWN);
        else
            StartQAV(pPlayer, kQAVSHOTDOWN);
        break;
    case kWeapTommyGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns) && pPlayer->weaponState == 1)
            StartQAV(pPlayer, kQAV2TOMDOWN);
        else
            StartQAV(pPlayer, kQAVTOMDOWN);
        break;
    case kWeapFlareGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns) && pPlayer->weaponState == 3)
            StartQAV(pPlayer, kQAVFLAR2DWN);
        else
            StartQAV(pPlayer, kQAVFLARDOWN);
        break;
    case kWeapVoodooDoll:
        StartQAV(pPlayer, kQAVVDDOWN);
        break;
    case kWeapTeslaCannon:
        if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
            StartQAV(pPlayer, kQAV2SGUNDWN);
        else
            StartQAV(pPlayer, kQAVSGUNDOWN);
        break;
    case kWeapNapalm:
        if (powerupCheck(pPlayer, kPwUpTwoGuns))
            StartQAV(pPlayer, kQAV2NAPDOWN);
        else
            StartQAV(pPlayer, kQAVNAPDOWN);
        break;
    case kWeapLifeLeech:
        StartQAV(pPlayer, kQAVSTAFDOWN);
        break;
    case kWeapBeast:
        StartQAV(pPlayer, kQAVBSTDOWN);
        break;
    }
    pPlayer->curWeapon = kWeapNone;
    pPlayer->qavLoop = 0;
}

void WeaponUpdateState(PLAYER *pPlayer)
{
    static int lastWeapon = 0;
    static int lastState = 0;
    XSPRITE *pXSprite = pPlayer->pXSprite;
    int va = pPlayer->curWeapon;
    int vb = pPlayer->weaponState;
    if (va != lastWeapon || vb != lastState)
    {
        lastWeapon = va;
        lastState = vb;
    }
    switch (lastWeapon)
    {
    case kWeapPitchFork:
        pPlayer->weaponQav = qavGetCorrectID(kQAVFORKIDLE);
        break;
    case kWeapSpraycan:
        switch (vb)
        {
        case 0:
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVLITEFLAM);
            break;
        case 1:
            if (CheckAmmo(pPlayer, 6, 1))
            {
                pPlayer->weaponState = 3;
                StartQAV(pPlayer, kQAVCANPREF);
            }
            else
                pPlayer->weaponQav = qavGetCorrectID(kQAVLITEIDLE);
            break;
        case 3:
            pPlayer->weaponQav = qavGetCorrectID(kQAVCANIDLE);
            break;
        case 4:
            if (CheckAmmo(pPlayer, 6, 1))
            {
                pPlayer->weaponQav = qavGetCorrectID(kQAVCANIDLE);
                pPlayer->weaponState = 3;
            }
            else
            {
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVCANDOWN);
            }
            sfxKill3DSound(pPlayer->pSprite, -1, 441);
            break;
        }
        break;
    case kWeapDynamite:
        switch (vb)
        {
        case 1:
            if (pPlayer->weaponAmmo == 5 && CheckAmmo(pPlayer, 5, 1))
            {
                pPlayer->weaponState = 3;
                StartQAV(pPlayer, kQAVBUNUP);
            }
            break;
        case 0:
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVLITEFLAM);
            break;
        case 2:
            if (pPlayer->ammoCount[5] > 0)
            {
                pPlayer->weaponState = 3;
                StartQAV(pPlayer, kQAVBUNUP);
            }
            else
                pPlayer->weaponQav = qavGetCorrectID(kQAVLITEIDLE);
            break;
        case 3:
            pPlayer->weaponQav = qavGetCorrectID(kQAVBUNIDLE);
            break;
        }
        break;
    case kWeapProximity:
        switch (vb)
        {
        case 7:
            pPlayer->weaponQav = qavGetCorrectID(kQAVPROXIDLE);
            break;
        case 8:
            pPlayer->weaponState = 7;
            StartQAV(pPlayer, kQAVPROXUP);
            break;
        }
        break;
    case kWeapRemote:
        switch (vb)
        {
        case 10:
            pPlayer->weaponQav = qavGetCorrectID(kQAVREMIDLE1);
            break;
        case 11:
            pPlayer->weaponQav = qavGetCorrectID(kQAVREMIDLE2);
            break;
        case 12:
            if (pPlayer->ammoCount[11] > 0)
            {
                pPlayer->weaponState = 10;
                StartQAV(pPlayer, kQAVREMUP2);
            }
            else
                pPlayer->weaponState = -1;
            break;
        }
        break;
    case kWeapShotgun:
        switch (vb)
        {
        case 6:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && (gInfiniteAmmo || CheckAmmo(pPlayer, 2, 4)))
                pPlayer->weaponState = 7;
            else
                pPlayer->weaponState = 1;
            break;
        case 7:
            pPlayer->weaponQav = qavGetCorrectID(kQAV2SHOTI);
            break;
        case 1:
            if (CheckAmmo(pPlayer, 2, 1))
            {
                sfxPlay3DSound(pPlayer->pSprite, 410, 3, 2);
                StartQAV(pPlayer, kQAVSHOTL1, nClientEjectShell);
                if (gInfiniteAmmo || pPlayer->ammoCount[2] > 1)
                    pPlayer->weaponState = 3;
                else
                    pPlayer->weaponState = 2;
            }
            else
                pPlayer->weaponQav = qavGetCorrectID(kQAVSHOTI3);
            break;
        case 2:
            pPlayer->weaponQav = qavGetCorrectID(kQAVSHOTI2);
            break;
        case 3:
            pPlayer->weaponQav = qavGetCorrectID(kQAVSHOTI1);
            break;
        }
        break;
    case kWeapTommyGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 3, 2))
        {
            pPlayer->weaponQav = qavGetCorrectID(kQAV2TOMIDLE);
            pPlayer->weaponState = 1;
        }
        else
        {
            pPlayer->weaponQav = qavGetCorrectID(kQAVTOMIDLE);
            pPlayer->weaponState = 0;
        }
        break;
    case kWeapFlareGun:
        if (powerupCheck(pPlayer, kPwUpTwoGuns))
        {
            if (vb == 3 && checkAmmo2(pPlayer, 1, 2))
                pPlayer->weaponQav = qavGetCorrectID(kQAVFLAR2I);
            else
            {
                pPlayer->weaponQav = qavGetCorrectID(kQAVFLARIDLE);
                pPlayer->weaponState = 2;
            }
        }
        else
            pPlayer->weaponQav = qavGetCorrectID(kQAVFLARIDLE);
        break;
    case kWeapVoodooDoll:
        if (pXSprite->height < 256 && pPlayer->swayHeight != 0)
            StartQAV(pPlayer, kQAVVDIDLE2);
        else
            pPlayer->weaponQav = qavGetCorrectID(kQAVVDIDLE1);
        break;
    case kWeapTeslaCannon:
        switch (vb)
        {
        case 2:
            if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
                pPlayer->weaponQav = qavGetCorrectID(kQAV2SGUNIDL);
            else
                pPlayer->weaponQav = qavGetCorrectID(kQAVSGUNIDL1);
            break;
        case 3:
            pPlayer->weaponQav = qavGetCorrectID(kQAVSGUNIDL2);
            break;
        }
        break;
    case kWeapNapalm:
        switch (vb)
        {
        case 3:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && (gInfiniteAmmo || CheckAmmo(pPlayer,4, 4)))
                pPlayer->weaponQav = qavGetCorrectID(kQAV2NAPIDLE);
            else
                pPlayer->weaponQav = qavGetCorrectID(kQAVNAPIDLE);
            break;
        case 2:
            pPlayer->weaponQav = qavGetCorrectID(kQAVNAPIDLE);
            break;
        }
        break;
    case kWeapLifeLeech:
        switch (vb)
        {
        case 2:
            pPlayer->weaponQav = qavGetCorrectID(kQAVSTAFIDL1);
            break;
        }
        break;
    case kWeapBeast:
        pPlayer->weaponQav = qavGetCorrectID(kQAVBSTIDLE);
        break;
    }
}

void FirePitchfork(int, PLAYER *pPlayer)
{
    Aim *aim = &pPlayer->aim;
    int r1 = Random2(2000);
    int r2 = Random2(2000);
    int r3 = Random2(2000);
    for (int i = 0; i < 4; i++)
        actFireVector(pPlayer->pSprite, (2*i-3)*40, pPlayer->zWeapon-pPlayer->pSprite->z, aim->dx+r1, aim->dy+r2, aim->dz+r3, kVectorTine);
}

void FireSpray(int, PLAYER *pPlayer)
{
    playerFireMissile(pPlayer, 0, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFlameSpray);
    UseAmmo(pPlayer, 6, 4);
    if (CheckAmmo(pPlayer, 6, 1))
        sfxPlay3DSound(pPlayer->pSprite, 441, 1, 2);
    else
        sfxKill3DSound(pPlayer->pSprite, -1, 441);
}

void ThrowCan(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, -1, 441);
    int nSpeed = MulScale(pPlayer->throwPower, 0x177777, 16)+0x66666;
    sfxPlay3DSound(pPlayer->pSprite, 455, 1, 0);
    spritetype *pSprite = playerFireThing(pPlayer, 0, -9460, kThingArmedSpray, nSpeed);
    if (pSprite)
    {
        sfxPlay3DSound(pSprite, 441, 0, 0);
        evPost(pSprite->index, 3, pPlayer->fuseTime, kCmdOn);
        int nXSprite = pSprite->extra;
        XSPRITE *pXSprite = &xsprite[nXSprite];
        pXSprite->Impact = 1;
        UseAmmo(pPlayer, 6, gAmmoItemData[0].count);
        pPlayer->throwPower = 0;
    }
}

void DropCan(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, -1, 441);
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedSpray, 0);
    if (pSprite)
    {
        evPost(pSprite->index, 3, pPlayer->fuseTime, kCmdOn);
        UseAmmo(pPlayer, 6, gAmmoItemData[0].count);
    }
}

void ExplodeCan(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, -1, 441);
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedSpray, 0);
    evPost(pSprite->index, 3, 0, kCmdOn);
    UseAmmo(pPlayer, 6, gAmmoItemData[0].count);
    StartQAV(pPlayer, kQAVCANBOOM);
    pPlayer->curWeapon = kWeapNone;
    pPlayer->throwPower = 0;
}

void ThrowBundle(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, 16, -1);
    int nSpeed = MulScale(pPlayer->throwPower, 0x177777, 16)+0x66666;
    sfxPlay3DSound(pPlayer->pSprite, 455, 1, 0);
    spritetype *pSprite = playerFireThing(pPlayer, 0, -9460, kThingArmedTNTBundle, nSpeed);
    int nXSprite = pSprite->extra;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    if (pPlayer->fuseTime < 0)
        pXSprite->Impact = 1;
    else
        evPost(pSprite->index, 3, pPlayer->fuseTime, kCmdOn);
    UseAmmo(pPlayer, 5, 1);
    pPlayer->throwPower = 0;
}

void DropBundle(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, 16, -1);
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedTNTBundle, 0);
    evPost(pSprite->index, 3, pPlayer->fuseTime, kCmdOn);
    UseAmmo(pPlayer, 5, 1);
}

void ExplodeBundle(int, PLAYER *pPlayer)
{
    sfxKill3DSound(pPlayer->pSprite, 16, -1);
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedTNTBundle, 0);
    evPost(pSprite->index, 3, 0, kCmdOn);
    UseAmmo(pPlayer, 5, 1);
    StartQAV(pPlayer, kQAVDYNEXPLO);
    pPlayer->curWeapon = kWeapNone;
    pPlayer->throwPower = 0;
}

void ThrowProx(int, PLAYER *pPlayer)
{
    int nSpeed = MulScale(pPlayer->throwPower, 0x177777, 16)+0x66666;
    sfxPlay3DSound(pPlayer->pSprite, 455, 1, 0);
    spritetype *pSprite = playerFireThing(pPlayer, 0, -9460, kThingArmedProxBomb, nSpeed);
    evPost(pSprite->index, 3, 240, kCmdOn);
    UseAmmo(pPlayer, 10, 1);
    pPlayer->throwPower = 0;
}

void DropProx(int, PLAYER *pPlayer)
{
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedProxBomb, 0);
    evPost(pSprite->index, 3, 240, kCmdOn);
    UseAmmo(pPlayer, 10, 1);
}

void ThrowRemote(int, PLAYER *pPlayer)
{
    int nSpeed = MulScale(pPlayer->throwPower, 0x177777, 16)+0x66666;
    sfxPlay3DSound(pPlayer->pSprite, 455, 1, 0);
    spritetype *pSprite = playerFireThing(pPlayer, 0, -9460, kThingArmedRemoteBomb, nSpeed);
    int nXSprite = pSprite->extra;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    pXSprite->rxID = 90+(pPlayer->pSprite->type-kDudePlayer1);
    UseAmmo(pPlayer, 11, 1);
    pPlayer->throwPower = 0;
}

void DropRemote(int, PLAYER *pPlayer)
{
    spritetype *pSprite = playerFireThing(pPlayer, 0, 0, kThingArmedRemoteBomb, 0);
    int nXSprite = pSprite->extra;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    pXSprite->rxID = 90+(pPlayer->pSprite->type-kDudePlayer1);
    UseAmmo(pPlayer, 11, 1);
}

void FireRemote(int, PLAYER *pPlayer)
{
    evSend(0, 0, 90+(pPlayer->pSprite->type-kDudePlayer1), kCmdOn);
}

enum { kMaxShotgunBarrels = 4 };

void FireShotgun(int nTrigger, PLAYER *pPlayer)
{
    assert(nTrigger > 0 && nTrigger <= kMaxShotgunBarrels);
    if (nTrigger == 1)
    {
        sfxPlay3DSound(pPlayer->pSprite, 411, 2, 0);
        pPlayer->tiltEffect = 30;
        pPlayer->visibility = 20;
    }
    else
    {
        sfxPlay3DSound(pPlayer->pSprite, 412, 2, 0);
        pPlayer->tiltEffect = 50;
        pPlayer->visibility = 40;
    }
    int n = nTrigger<<4;
    for (int i = 0; i < n; i++)
    {
        int r1, r2, r3;
        VECTOR_TYPE nType;
        if (nTrigger == 1)
        {
            r1 = Random3(1500);
            r2 = Random3(1500);
            r3 = Random3(500);
            nType = kVectorShell;
        }
        else
        {
            r1 = Random3(2500);
            r2 = Random3(2500);
            r3 = Random3(1500);
            nType = kVectorShellAP;
        }
        actFireVector(pPlayer->pSprite, 0, pPlayer->zWeapon-pPlayer->pSprite->z, pPlayer->aim.dx+r1, pPlayer->aim.dy+r2, pPlayer->aim.dz+r3, nType);
    }
    UseAmmo(pPlayer, pPlayer->weaponAmmo, nTrigger);
    pPlayer->flashEffect = 1;
}

void EjectShell(int, PLAYER *pPlayer)
{
    SpawnShellEject(pPlayer, 25, 35);
    SpawnShellEject(pPlayer, 48, 35);
}

void FireTommy(int nTrigger, PLAYER *pPlayer)
{
    Aim *aim = &pPlayer->aim;
    sfxPlay3DSound(pPlayer->pSprite, 431, -1, 0);
    switch (nTrigger)
    {
    case 1:
    {
        int r1 = Random3(400);
        int r2 = Random3(1200);
        int r3 = Random3(1200);
        actFireVector(pPlayer->pSprite, 0, pPlayer->zWeapon-pPlayer->pSprite->z, aim->dx+r3, aim->dy+r2, aim->dz+r1, kVectorTommyRegular);
        SpawnBulletEject(pPlayer, -15, -45);
        pPlayer->visibility = 20;
        break;
    }
    case 2:
    {
        int r1 = Random3(400);
        int r2 = Random3(1200);
        int r3 = Random3(1200);
        actFireVector(pPlayer->pSprite, -120, pPlayer->zWeapon-pPlayer->pSprite->z, aim->dx+r3, aim->dy+r2, aim->dz+r1, kVectorTommyRegular);
        SpawnBulletEject(pPlayer, -140, -45);
        r1 = Random3(400);
        r2 = Random3(1200);
        r3 = Random3(1200);
        actFireVector(pPlayer->pSprite, 120, pPlayer->zWeapon-pPlayer->pSprite->z, aim->dx+r3, aim->dy+r2, aim->dz+r1, kVectorTommyRegular);
        SpawnBulletEject(pPlayer, 140, 45);
        pPlayer->visibility = 30;
        break;
    }
    }
    UseAmmo(pPlayer, pPlayer->weaponAmmo, nTrigger);
    pPlayer->flashEffect = 1;
}

enum { kMaxSpread = 14 };

void FireSpread(int nTrigger, PLAYER *pPlayer)
{
    assert(nTrigger > 0 && nTrigger <= kMaxSpread);
    Aim *aim = &pPlayer->aim;
    int angle = (getangle(aim->dx, aim->dy)+((112*(nTrigger-1))/14-56))&2047;
    int dx = CosScale16(angle);
    int dy = SinScale16(angle);
    sfxPlay3DSound(pPlayer->pSprite, 431, -1, 0);
    int r1, r2, r3;
    r1 = Random3(300);
    r2 = Random3(600);
    r3 = Random3(600);
    actFireVector(pPlayer->pSprite, 0, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
    r1 = Random2(90);
    r2 = Random2(30);
    SpawnBulletEject(pPlayer, r2, r1);
    pPlayer->visibility = 20;
    UseAmmo(pPlayer, pPlayer->weaponAmmo, 1);
    pPlayer->flashEffect = 1;
}

void AltFireSpread(int nTrigger, PLAYER *pPlayer)
{
    assert(nTrigger > 0 && nTrigger <= kMaxSpread);
    Aim *aim = &pPlayer->aim;
    int angle = (getangle(aim->dx, aim->dy)+((112*(nTrigger-1))/14-56))&2047;
    int dx = CosScale16(angle);
    int dy = SinScale16(angle);
    sfxPlay3DSound(pPlayer->pSprite, 431, -1, 0);
    int r1, r2, r3;
    r1 = Random3(300);
    r2 = Random3(600);
    r3 = Random3(600);
    actFireVector(pPlayer->pSprite, -120, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
    r1 = Random2(45);
    r2 = Random2(120);
    SpawnBulletEject(pPlayer, r2, r1);
    r1 = Random3(300);
    r2 = Random3(600);
    r3 = Random3(600);
    actFireVector(pPlayer->pSprite, 120, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
    r1 = Random2(-45);
    r2 = Random2(-120);
    SpawnBulletEject(pPlayer, r2, r1);
    pPlayer->tiltEffect = 20;
    pPlayer->visibility = 30;
    UseAmmo(pPlayer, pPlayer->weaponAmmo, 2);
    pPlayer->flashEffect = 1;
}

void AltFireSpread2(int nTrigger, PLAYER *pPlayer)
{
    assert(nTrigger > 0 && nTrigger <= kMaxSpread);
    Aim *aim = &pPlayer->aim;
    int angle = (getangle(aim->dx, aim->dy)+((112*(nTrigger-1))/14-56))&2047;
    int dx = CosScale16(angle);
    int dy = SinScale16(angle);
    sfxPlay3DSound(pPlayer->pSprite, 431, -1, 0);
    if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 3, 2))
    {
        int r1, r2, r3;
        r1 = Random3(300);
        r2 = Random3(600);
        r3 = Random3(600);
        actFireVector(pPlayer->pSprite, -120, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
        r1 = Random2(45);
        r2 = Random2(120);
        SpawnBulletEject(pPlayer, r2, r1);
        r1 = Random3(300);
        r2 = Random3(600);
        r3 = Random3(600);
        actFireVector(pPlayer->pSprite, 120, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
        r1 = Random2(-45);
        r2 = Random2(-120);
        SpawnBulletEject(pPlayer, r2, r1);
        pPlayer->tiltEffect = 30;
        pPlayer->visibility = 45;
        UseAmmo(pPlayer, pPlayer->weaponAmmo, 2);
    }
    else
    {
        int r1, r2, r3;
        r1 = Random3(300);
        r2 = Random3(600);
        r3 = Random3(600);
        actFireVector(pPlayer->pSprite, 0, pPlayer->zWeapon-pPlayer->pSprite->z, dx+r3, dy+r2, aim->dz+r1, kVectorTommyAP);
        r1 = Random2(90);
        r2 = Random2(30);
        SpawnBulletEject(pPlayer, r2, r1);
        pPlayer->tiltEffect = 20;
        pPlayer->visibility = 30;
        UseAmmo(pPlayer, pPlayer->weaponAmmo, 1);
    }
    pPlayer->flashEffect = 1;
    if (!checkAmmo2(pPlayer, 3, 1))
    {
        WeaponLower(pPlayer);
        pPlayer->weaponState = -1;
    }
}

void FireFlare(int nTrigger, PLAYER *pPlayer)
{
    spritetype *pSprite = pPlayer->pSprite;
    int offset = 0;
    switch (nTrigger)
    {
    case 2:
        offset = -120;
        break;
    case 3:
        offset = 120;
        break;
    }
    playerFireMissile(pPlayer, offset, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFlareRegular);
    UseAmmo(pPlayer, 1, 1);
    sfxPlay3DSound(pSprite, 420, 2, 0);
    pPlayer->visibility = 30;
    pPlayer->flashEffect = 1;
}

void AltFireFlare(int nTrigger, PLAYER *pPlayer)
{
    spritetype *pSprite = pPlayer->pSprite;
    int offset = 0;
    switch (nTrigger)
    {
    case 2:
        offset = -120;
        break;
    case 3:
        offset = 120;
        break;
    }
    playerFireMissile(pPlayer, offset, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFlareAlt);
    UseAmmo(pPlayer, 1, 8);
    sfxPlay3DSound(pSprite, 420, 2, 0);
    pPlayer->visibility = 45;
    pPlayer->flashEffect = 1;
}

void FireVoodoo(int nTrigger, PLAYER *pPlayer)
{
    nTrigger--;
    int nSprite = pPlayer->nSprite;
    spritetype *pSprite = pPlayer->pSprite;
    if (nTrigger == 4)
    {
        actDamageSprite(nSprite, pSprite, kDamageBullet, 1<<4);
        return;
    }
    assert(pPlayer->voodooTarget >= 0);
    spritetype *pTarget = &sprite[pPlayer->voodooTarget];
    if (!gGameOptions.bFriendlyFire && IsTargetTeammate(pPlayer, pTarget))
        return;
    switch (nTrigger)
    {
    case 0:
    {
        sfxPlay3DSound(pSprite, 460, 2, 0);
        fxSpawnBlood(pTarget, 17<<4);
        int nDamage = actDamageSprite(nSprite, pTarget, kDamageSpirit, 17<<4);
        UseAmmo(pPlayer, 9, nDamage/4);
        break;
    }
    case 1:
    {
        sfxPlay3DSound(pSprite, 460, 2, 0);
        fxSpawnBlood(pTarget, 17<<4);
        int nDamage = actDamageSprite(nSprite, pTarget, kDamageSpirit, 9<<4);
        if (IsPlayerSprite(pTarget))
            WeaponLower(&gPlayer[pTarget->type-kDudePlayer1]);
        UseAmmo(pPlayer, 9, nDamage/4);
        break;
    }
    case 3:
    {
        sfxPlay3DSound(pSprite, 463, 2, 0);
        fxSpawnBlood(pTarget, 17<<4);
        int nDamage = actDamageSprite(nSprite, pTarget, kDamageSpirit, 49<<4);
        UseAmmo(pPlayer, 9, nDamage/4);
        break;
    }
    case 2:
    {
        sfxPlay3DSound(pSprite, 460, 2, 0);
        fxSpawnBlood(pTarget, 17<<4);
        int nDamage = actDamageSprite(nSprite, pTarget, kDamageSpirit, 11<<4);
        if (IsPlayerSprite(pTarget))
        {
            PLAYER *pOtherPlayer = &gPlayer[pTarget->type - kDudePlayer1];
            pOtherPlayer->blindEffect = 128;
        }
        UseAmmo(pPlayer, 9, nDamage/4);
        break;
    }
    }
}

void AltFireVoodoo(int nTrigger, PLAYER *pPlayer)
{
    if (nTrigger == 2) {

        // by NoOne: trying to simulate v1.0x voodoo here.
        // dunno how exactly it works, but at least it not spend all the ammo on alt fire
        if (gGameOptions.weaponsV10x && !VanillaMode()) {
            int nCount = ClipHigh(pPlayer->ammoCount[9], pPlayer->aimTargetsCount);
            if (nCount > 0)
            {
                for (int i = 0; i < pPlayer->aimTargetsCount; i++)
                {
                    int nTarget = pPlayer->aimTargets[i];
                    spritetype* pTarget = &sprite[nTarget];
                    if (!gGameOptions.bFriendlyFire && IsTargetTeammate(pPlayer, pTarget))
                        continue;
                    int nDist = approxDist(pTarget->x - pPlayer->pSprite->x, pTarget->y - pPlayer->pSprite->y);
                    if (nDist > 0 && nDist < 51200)
                    {
                        int vc = pPlayer->ammoCount[9] >> 3;
                        int v8 = pPlayer->ammoCount[9] << 1;
                        int nDamage = (v8 + Random(vc)) << 4;
                        nDamage = (nDamage * ((51200 - nDist) + 1)) / 51200;
                        nDamage = actDamageSprite(pPlayer->nSprite, pTarget, kDamageSpirit, nDamage);

                        if (IsPlayerSprite(pTarget))
                        {
                            PLAYER* pOtherPlayer = &gPlayer[pTarget->type - kDudePlayer1];
                            if (!pOtherPlayer->godMode || !powerupCheck(pOtherPlayer, kPwUpDeathMask))
                                powerupActivate(pOtherPlayer, kPwUpDeliriumShroom);
                        }
                        fxSpawnBlood(pTarget, 0);
                    }
                }
            }

            UseAmmo(pPlayer, 9, 20);
            pPlayer->weaponState = 0;
            return;
        }

        //int nAmmo = pPlayer->ammCount[9];
        int nCount = ClipHigh(pPlayer->ammoCount[9], pPlayer->aimTargetsCount);
        if (nCount > 0)
        {
            int v4 = pPlayer->ammoCount[9] - (pPlayer->ammoCount[9] / nCount) * nCount;
            for (int i = 0; i < pPlayer->aimTargetsCount; i++)
            {
                int nTarget = pPlayer->aimTargets[i];
                spritetype* pTarget = &sprite[nTarget];
                if (!gGameOptions.bFriendlyFire && IsTargetTeammate(pPlayer, pTarget))
                    continue;
                if (v4 > 0)
                    v4--;
                int nDist = approxDist(pTarget->x - pPlayer->pSprite->x, pTarget->y - pPlayer->pSprite->y);
                if (nDist > 0 && nDist < 51200)
                {
                    int vc = pPlayer->ammoCount[9] >> 3;
                    int v8 = pPlayer->ammoCount[9] << 1;
                    int nDamage = (v8 + Random2(vc)) << 4;
                    nDamage = (nDamage * ((51200 - nDist) + 1)) / 51200;
                    nDamage = actDamageSprite(pPlayer->nSprite, pTarget, kDamageSpirit, nDamage);
                    UseAmmo(pPlayer, 9, nDamage);
                    if (IsPlayerSprite(pTarget))
                    {
                        PLAYER* pOtherPlayer = &gPlayer[pTarget->type - kDudePlayer1];
                        if (!pOtherPlayer->godMode || !powerupCheck(pOtherPlayer, kPwUpDeathMask))
                            powerupActivate(pOtherPlayer, kPwUpDeliriumShroom);
                    }
                    fxSpawnBlood(pTarget, 0);
                }
            }
        }
        UseAmmo(pPlayer, 9, pPlayer->ammoCount[9]);
        pPlayer->hasWeapon[10] = 0;
        pPlayer->weaponState = -1;
    }
}

void DropVoodoo(int , PLAYER *pPlayer)
{
    sfxPlay3DSound(pPlayer->pSprite, 455, 2, 0);
    spritetype *pSprite = playerFireThing(pPlayer, 0, -4730, kThingVoodooHead, 0xccccc);
    if (pSprite)
    {
        int nXSprite = pSprite->extra;
        XSPRITE *pXSprite = &xsprite[nXSprite];
        pXSprite->data1 = pPlayer->ammoCount[9];
        evPost(pSprite->index, 3, 90, kCallbackDropVoodoo);
        UseAmmo(pPlayer, 6, gAmmoItemData[0].count);
        UseAmmo(pPlayer, 9, pPlayer->ammoCount[9]);
        pPlayer->hasWeapon[10] = 0;
    }
}

struct TeslaMissile
{
    int offset; // offset
    int id; // id
    int ammouse; // ammo use
    int sound; // sound
    int light; // light
    int flash; // weapon flash
};

void FireTesla(int nTrigger, PLAYER *pPlayer)
{
    TeslaMissile teslaMissile[6] = 
    {
        { 0, 306, 1, 470, 20, 1 },
        { -140, 306, 1, 470, 30, 1 },
        { 140, 306, 1, 470, 30, 1 },
        { 0, 302, 35, 471, 40, 1 },
        { -140, 302, 35, 471, 50, 1 },
        { 140, 302, 35, 471, 50, 1 },
    };
    if (nTrigger > 0 && nTrigger <= 6)
    {
        nTrigger--;
        spritetype *pSprite = pPlayer->pSprite;
        TeslaMissile *pMissile = &teslaMissile[nTrigger];
        if (!checkAmmo2(pPlayer, 7, pMissile->ammouse))
        {
            pMissile = &teslaMissile[0];
            if (!checkAmmo2(pPlayer, 7, pMissile->ammouse))
            {
                pPlayer->weaponState = -1;
                pPlayer->weaponQav = qavGetCorrectID(kQAVSGUNIDL2);
                pPlayer->flashEffect = 0;
                return;
            }
        }
        playerFireMissile(pPlayer, pMissile->offset, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, pMissile->id);
        UseAmmo(pPlayer, 7, pMissile->ammouse);
        sfxPlay3DSound(pSprite, pMissile->sound, 1, 0);
        pPlayer->visibility = pMissile->light;
        pPlayer->flashEffect = pMissile->flash;
    }
}

void AltFireTesla(int , PLAYER *pPlayer)
{
    spritetype *pSprite = pPlayer->pSprite;
    playerFireMissile(pPlayer, 0, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileTeslaAlt);
    UseAmmo(pPlayer, pPlayer->weaponAmmo, 35);
    sfxPlay3DSound(pSprite, 471, 2, 0);
    pPlayer->visibility = 40;
    pPlayer->flashEffect = 1;
}

void FireNapalm(int nTrigger, PLAYER *pPlayer)
{
    spritetype *pSprite = pPlayer->pSprite;
    int offset = 0;
    switch (nTrigger)
    {
    case 2:
        offset = -50;
        break;
    case 3:
        offset = 50;
        break;
    }
    playerFireMissile(pPlayer, offset, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFireballNapalm);
    sfxPlay3DSound(pSprite, 480, 2, 0);
    UseAmmo(pPlayer, 4, 1);
    pPlayer->flashEffect = 1;
}

void FireNapalm2(int , PLAYER *pPlayer)
{
    spritetype *pSprite = pPlayer->pSprite;
    playerFireMissile(pPlayer, -120, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFireballNapalm);
    playerFireMissile(pPlayer, 120, pPlayer->aim.dx, pPlayer->aim.dy, pPlayer->aim.dz, kMissileFireballNapalm);
    sfxPlay3DSound(pSprite, 480, 2, 0);
    UseAmmo(pPlayer, 4, 2);
    pPlayer->flashEffect = 1;
}

void AltFireNapalm(int , PLAYER *pPlayer)
{
    int nSpeed = MulScale(0x8000, 0x177777, 16)+0x66666;
    spritetype *pMissile = playerFireThing(pPlayer, 0, -4730, kThingNapalmBall, nSpeed);
    if (pMissile)
    {
        XSPRITE *pXSprite = &xsprite[pMissile->extra];
        pXSprite->data4 = ClipHigh(pPlayer->ammoCount[4], 12);
        UseAmmo(pPlayer, 4, pXSprite->data4);
        seqSpawn(22, 3, pMissile->extra, -1);
        actBurnSprite(pPlayer->pSprite->index, pXSprite, 600);
        evPost(pMissile->index, 3, 0, kCallbackFXFlameLick);
        sfxPlay3DSound(pMissile, 480, 2, 0);
        pPlayer->visibility = 30;
        pPlayer->flashEffect = 1;
    }
}

void FireLifeLeech(int nTrigger, PLAYER *pPlayer)
{
    if (!CheckAmmo(pPlayer, 8, 1))
        return;
    int r1 = Random2(2000);
    int r2 = Random2(2000);
    int r3 = Random2(1000);
    spritetype *pMissile = playerFireMissile(pPlayer, 0, pPlayer->aim.dx+r1, pPlayer->aim.dy+r2, pPlayer->aim.dz+r3, 315);
    if (pMissile)
    {
        XSPRITE *pXSprite = &xsprite[pMissile->extra];
        pXSprite->target = pPlayer->aimTarget;
        pMissile->ang = (nTrigger==2) ? 1024 : 0;
    }
    if (checkAmmo2(pPlayer, 8, 1))
        UseAmmo(pPlayer, 8, 1);
    else
        actDamageSprite(pPlayer->nSprite, pPlayer->pSprite, kDamageSpirit, 16);
    pPlayer->visibility = ClipHigh(pPlayer->visibility+5, 50);
}

void AltFireLifeLeech(int , PLAYER *pPlayer)
{
    sfxPlay3DSound(pPlayer->pSprite, 455, 2, 0);
    spritetype *pMissile = playerFireThing(pPlayer, 0, -4730, kThingDroppedLifeLeech, 0x19999);
    if (pMissile)
    {
        pMissile->cstat |= 4096;
        XSPRITE *pXSprite = &xsprite[pMissile->extra];
        pXSprite->Push = 1;
        pXSprite->Proximity = 1;
        pXSprite->DudeLockout = 1;
        pXSprite->data4 = ClipHigh(pPlayer->ammoCount[4], 12);
        pXSprite->stateTimer = 1;
        evPost(pMissile->index, 3, 120, kCallbackLeechStateTimer);
        if (gGameOptions.nGameType <= 1)
        {
            int nAmmo = pPlayer->ammoCount[8];
            if (nAmmo < 25 && pPlayer->pXSprite->health > unsigned((25-nAmmo)<<4))
            {
                actDamageSprite(pPlayer->nSprite, pPlayer->pSprite, kDamageSpirit, ((25-nAmmo)<<4));
                nAmmo = 25;
            }
            pXSprite->data3 = nAmmo;
            UseAmmo(pPlayer, 8, nAmmo);
        }
        else
        {
            pXSprite->data3 = pPlayer->ammoCount[8];
            pPlayer->ammoCount[8] = 0;
        }
        pPlayer->hasWeapon[9] = 0;
    }
}

void FireBeast(int , PLAYER * pPlayer)
{
    int r1 = Random2(2000);
    int r2 = Random2(2000);
    int r3 = Random2(2000);
    actFireVector(pPlayer->pSprite, 0, pPlayer->zWeapon-pPlayer->pSprite->z, pPlayer->aim.dx+r1, pPlayer->aim.dy+r2, pPlayer->aim.dz+r3, kVectorBeastSlash);
}

uint8_t gWeaponUpgrade[][13] = {
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
};

int WeaponUpgrade(PLAYER *pPlayer, int newWeapon)
{
    int weapon = pPlayer->curWeapon;
    if (!checkFired6or7(pPlayer) && (cl_weaponswitch&1) && (gWeaponUpgrade[pPlayer->curWeapon][newWeapon] || (cl_weaponswitch&2)))
        weapon = newWeapon;
    return weapon;
}

int OrderNext[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 1 };
int OrderPrev[] = { 12, 12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 1 };

static int WeaponFindNext(PLAYER *pPlayer, int *a2, int bDir)
{
    int weapon = pPlayer->curWeapon;
    do
    {
        if (bDir)
            weapon = OrderNext[weapon];
        else
            weapon = OrderPrev[weapon];
        if (weaponModes[weapon].update && pPlayer->hasWeapon[weapon])
        {
            if (weapon == kWeapLifeLeech)
            {
                if (CheckAmmo(pPlayer, weaponModes[weapon].ammoType, 1))
                    break;
            }
            else
            {
                if (checkAmmo2(pPlayer, weaponModes[weapon].ammoType, 1))
                    break;
            }
        }
    } while (weapon != pPlayer->curWeapon);
    if (weapon == pPlayer->curWeapon)
    {
        if (!weaponModes[weapon].update || !CheckAmmo(pPlayer, weaponModes[weapon].ammoType, 1))
            weapon = kWeapPitchFork;
    }
    if (a2)
        *a2 = 0;
    return weapon;
}

static int WeaponFindLoaded(PLAYER *pPlayer, int *a2)
{
    int v4 = 1;
    int v14 = 0;
    if (weaponModes[pPlayer->curWeapon].update > 1)
    {
        for (int i = 0; i < weaponModes[pPlayer->curWeapon].update; i++)
        {
            if (CheckAmmo(pPlayer, weaponModes[pPlayer->curWeapon].ammoType, 1))
            {
                v14 = i;
                v4 = pPlayer->curWeapon;
                break;
            }
        }
    }
    if (v4 == kWeapPitchFork)
    {
        int vc = 0;
        for (int i = 0; i < 14; i++)
        {
            int weapon = pPlayer->weaponOrder[vc][i];
            if (pPlayer->hasWeapon[weapon])
            {
                for (int j = 0; j < weaponModes[weapon].update; j++)
                {
                    if (CheckWeaponAmmo(pPlayer, weapon, weaponModes[weapon].ammoType, 1))
                    {
                        if (a2)
                            *a2 = j;
                        return weapon;
                    }
                }
            }
        }
    }
    else if (a2)
        *a2 = v14;
    return v4;
}

int processSprayCan(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 5:
        if (!(pPlayer->input.actions & SB_ALTFIRE))
            pPlayer->weaponState = 6;
        return 1;
    case 6:
        if (pPlayer->input.actions & SB_ALTFIRE)
        {
            pPlayer->weaponState = 3;
            pPlayer->fuseTime = pPlayer->weaponTimer;
            StartQAV(pPlayer, kQAVCANDROP, nClientDropCan);
        }
        else if (pPlayer->input.actions & SB_FIRE)
        {
            pPlayer->weaponState = 7;
            pPlayer->fuseTime = 0;
            pPlayer->throwTime = PlayClock;
        }
        return 1;
    case 7:
    {
        pPlayer->throwPower = ClipHigh(DivScale(PlayClock-pPlayer->throwTime,240, 16), 65536);
        if (!(pPlayer->input.actions & SB_FIRE))
        {
            if (!pPlayer->fuseTime)
                pPlayer->fuseTime = pPlayer->weaponTimer;
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVCANTHRO, nClientThrowCan);
        }
        return 1;
    }
    }
    return 0;
}

static bool processTNT(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 4:
        if (!(pPlayer->input.actions & SB_ALTFIRE))
            pPlayer->weaponState = 5;
        return 1;
    case 5:
        if (pPlayer->input.actions & SB_ALTFIRE)
        {
            pPlayer->weaponState = 1;
            pPlayer->fuseTime = pPlayer->weaponTimer;
            StartQAV(pPlayer, kQAVBUNDROP, nClientDropBundle);
        }
        else if (pPlayer->input.actions & SB_FIRE)
        {
            pPlayer->weaponState = 6;
            pPlayer->fuseTime = 0;
            pPlayer->throwTime = PlayClock;
        }
        return 1;
    case 6:
    {
        pPlayer->throwPower = ClipHigh(DivScale(PlayClock-pPlayer->throwTime,240, 16), 65536);
        if (!(pPlayer->input.actions & SB_FIRE))
        {
            if (!pPlayer->fuseTime)
                pPlayer->fuseTime = pPlayer->weaponTimer;
            pPlayer->weaponState = 1;
            StartQAV(pPlayer, kQAVBUNTHRO, nClientThrowBundle);
        }
        return 1;
    }
    }
    return 0;
}

static bool processProxy(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 9:
        pPlayer->throwPower = ClipHigh(DivScale(PlayClock-pPlayer->throwTime,240, 16), 65536);
        pPlayer->weaponTimer = 0;
        pPlayer->qavTimer = 0;
        if (!(pPlayer->input.actions & SB_FIRE))
        {
            pPlayer->weaponState = 8;
            StartQAV(pPlayer, kQAVPROXTHRO, nClientThrowProx);
        }
        break;
    }
    return 0;
}

static bool processRemote(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 13:
        pPlayer->throwPower = ClipHigh(DivScale(PlayClock-pPlayer->throwTime,240, 16), 65536);
        if (!(pPlayer->input.actions & SB_FIRE))
        {
            pPlayer->weaponState = 11;
            StartQAV(pPlayer, kQAVREMTHRO, nClientThrowRemote);
        }
        break;
    }
    return 0;
}

static bool processLeech(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 4:
        pPlayer->weaponState = 6;
        StartQAV(pPlayer, kQAVSTAFIRE1, nClientFireLifeLeech, 1);
        return 1;
    case 6:
        if (!(pPlayer->input.actions & SB_ALTFIRE))
        {
            pPlayer->weaponState = 2;
            StartQAV(pPlayer, kQAVSTAFPOST);
            return 1;
        }
        break;
    case 8:
        pPlayer->weaponState = 2;
        StartQAV(pPlayer, kQAVSTAFPOST);
        return 1;
    }
    return 0;
}

static bool processTesla(PLAYER *pPlayer)
{
    switch (pPlayer->weaponState)
    {
    case 4:
        pPlayer->weaponState = 5;
        if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
            StartQAV(pPlayer, kQAV2SGUNFIR, nClientFireTesla, 1);
        else
            StartQAV(pPlayer, kQAVSGUNFIR1, nClientFireTesla, 1);
        return 1;
    case 5:
        if (!(pPlayer->input.actions & SB_FIRE))
        {
            pPlayer->weaponState = 2;
            if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
                StartQAV(pPlayer, kQAV2SGUNPST);
            else
                StartQAV(pPlayer, kQAVSGUNPOST);
            return 1;
        }
        break;
    case 7:
        pPlayer->weaponState = 2;
        if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
            StartQAV(pPlayer, kQAV2SGUNPST);
        else
            StartQAV(pPlayer, kQAVSGUNPOST);
        return 1;
    }
    return 0;
}

void WeaponProcess(PLAYER *pPlayer) {

    pPlayer->flashEffect = ClipLow(pPlayer->flashEffect - 1, 0);
    
    #ifdef NOONE_EXTENSIONS
    if (gPlayerCtrl[pPlayer->nPlayer].qavScene.index >= 0 && pPlayer->pXSprite->health > 0) {
        playerQavSceneProcess(pPlayer, &gPlayerCtrl[pPlayer->nPlayer].qavScene);
        UpdateAimVector(pPlayer);
        return;
    }
    #endif

    if (pPlayer->pXSprite->health == 0)
    {
        pPlayer->qavLoop = 0;
        sfxKill3DSound(pPlayer->pSprite, 1, -1);
    }
    if (pPlayer->isUnderwater && BannedUnderwater(pPlayer->curWeapon))
    {
        if (checkFired6or7(pPlayer))
        {
            if (pPlayer->curWeapon == kWeapSpraycan)
            {
                pPlayer->fuseTime = pPlayer->weaponTimer;
                DropCan(1, pPlayer);
                pPlayer->weaponState = 3;
            }
            else if (pPlayer->curWeapon == kWeapDynamite)
            {
                pPlayer->fuseTime = pPlayer->weaponTimer;
                DropBundle(1, pPlayer);
                pPlayer->weaponState = 1;
            }
        }
        WeaponLower(pPlayer);
        pPlayer->throwPower = 0;
    }
    WeaponPlay(pPlayer);
    UpdateAimVector(pPlayer);
    pPlayer->weaponTimer -= 4;
    bool bShoot = pPlayer->input.actions & SB_FIRE;
    bool bShoot2 = pPlayer->input.actions & SB_ALTFIRE;
    if ((bShoot || bShoot2) && pPlayer->weaponQav == qavGetCorrectID(kQAVVDIDLE2)) pPlayer->weaponTimer = 0;
    if (pPlayer->qavLoop && pPlayer->pXSprite->health > 0)
    {
        if (bShoot && CheckAmmo(pPlayer, pPlayer->weaponAmmo, 1))
        {
            auto pQAV = getQAV(pPlayer->weaponQav);
            while (pPlayer->weaponTimer <= 0)
            {
                pPlayer->weaponTimer += pQAV->duration;
                pPlayer->qavTimer += pQAV->duration;
            }
        }
        else
        {
            pPlayer->weaponTimer = 0;
            pPlayer->qavTimer = 0;
            pPlayer->qavLoop = 0;
        }
        return;
    }
    pPlayer->weaponTimer = ClipLow(pPlayer->weaponTimer, 0);
    switch (pPlayer->curWeapon)
    {
    case kWeapSpraycan:
        if (processSprayCan(pPlayer))
            return;
        break;
    case kWeapDynamite:
        if (processTNT(pPlayer))
            return;
        break;
    case kWeapProximity:
        if (processProxy(pPlayer))
            return;
        break;
    case kWeapRemote:
        if (processRemote(pPlayer))
            return;
        break;
    }
    if (pPlayer->weaponTimer > 0)
        return;
    if (pPlayer->pXSprite->health == 0 || pPlayer->curWeapon == kWeapNone)
        pPlayer->weaponQav = kQAVNone;
    switch (pPlayer->curWeapon)
    {
    case kWeapLifeLeech:
        if (processLeech(pPlayer))
            return;
        break;
    case kWeapTeslaCannon:
        if (processTesla(pPlayer))
            return;
        break;
    }
    const int prevNewWeaponVal = pPlayer->input.getNewWeapon(); // used to fix scroll issue for banned weapons
    if (VanillaMode())
    {
        if (pPlayer->nextWeapon)
        {
            sfxKill3DSound(pPlayer->pSprite, -1, 441);
            pPlayer->weaponState = 0;
            pPlayer->newWeapon = pPlayer->nextWeapon;
            pPlayer->nextWeapon = kWeapNone;
        }
    }
    if (pPlayer->input.getNewWeapon() == WeaponSel_Next)
    {
        pPlayer->input.setNewWeapon(kWeapNone);
        if (VanillaMode())
        {
            pPlayer->weaponState = 0;
        }
        pPlayer->nextWeapon = kWeapNone;
        int t;
        int weapon = WeaponFindNext(pPlayer, &t, 1);
        pPlayer->weaponMode[weapon] = t;
        if (VanillaMode())
        {
            if (pPlayer->curWeapon)
            {
                WeaponLower(pPlayer);
                pPlayer->nextWeapon = weapon;
                return;
            }
        }
        pPlayer->newWeapon = weapon;
    }
    else if (pPlayer->input.getNewWeapon() == WeaponSel_Prev)
    {
        pPlayer->input.setNewWeapon(kWeapNone);
        if (VanillaMode())
        {
            pPlayer->weaponState = 0;
        }
        pPlayer->nextWeapon = kWeapNone;
        int t;
        int weapon = WeaponFindNext(pPlayer, &t, 0);
        pPlayer->weaponMode[weapon] = t;
        if (VanillaMode())
        {
            if (pPlayer->curWeapon)
            {
                WeaponLower(pPlayer);
                pPlayer->nextWeapon = weapon;
                return;
            }
        }
        pPlayer->newWeapon = weapon;
    }
    else if (pPlayer->input.getNewWeapon() == WeaponSel_Alt)
    {
        int weapon;

        switch (pPlayer->curWeapon)
        {
            case kWeapDynamite:
                weapon = kWeapProximity;
                break;
            case kWeapProximity:
                weapon = kWeapRemote;
                break;
            case kWeapRemote:
                weapon = kWeapDynamite;
                break;
            default:
                return;
        }

        pPlayer->input.setNewWeapon(kWeapNone);
        pPlayer->weaponState = 0;
        pPlayer->nextWeapon = kWeapNone;
        int t = 0;
        pPlayer->weaponMode[weapon] = t;
        if (pPlayer->curWeapon)
        {
            WeaponLower(pPlayer);
            pPlayer->nextWeapon = weapon;
            return;
        }
        pPlayer->newWeapon = weapon;
    }
    if (!VanillaMode())
    {
        if (pPlayer->nextWeapon)
        {
            sfxKill3DSound(pPlayer->pSprite, -1, 441);
            pPlayer->newWeapon = pPlayer->nextWeapon;
            pPlayer->nextWeapon = kWeapNone;
        }
    }
    if (pPlayer->weaponState == -1)
    {
        pPlayer->weaponState = 0;
        int t;
        int weapon = WeaponFindLoaded(pPlayer, &t);
        pPlayer->weaponMode[weapon] = t;
        if (pPlayer->curWeapon)
        {
            WeaponLower(pPlayer);
            pPlayer->nextWeapon = weapon;
            return;
        }
        pPlayer->newWeapon = weapon;
    }
    if (pPlayer->newWeapon)
    {
        if (pPlayer->isUnderwater && BannedUnderwater(pPlayer->newWeapon) && !checkFired6or7(pPlayer) && !VanillaMode()) // skip banned weapons when underwater and using next/prev weapon key inputs
        {
            if (prevNewWeaponVal == WeaponSel_Next || prevNewWeaponVal == WeaponSel_Prev) // if player switched weapons
            {
                PLAYER tmpPlayer = *pPlayer;
                tmpPlayer.curWeapon = pPlayer->newWeapon; // set current banned weapon to curweapon so WeaponFindNext() can find the next weapon
                for (int i = 0; i < 3; i++) // attempt twice to find a new weapon
                {
                    tmpPlayer.curWeapon = WeaponFindNext(&tmpPlayer, NULL, (char)(prevNewWeaponVal == WeaponSel_Next));
                    if (!BannedUnderwater(tmpPlayer.curWeapon)) // if new weapon is not a banned weapon, set to new current weapon
                    {
                        pPlayer->newWeapon = tmpPlayer.curWeapon;
                        pPlayer->weaponMode[pPlayer->newWeapon] = 0;
                        break;
                    }
                }
            }
        }
        if (pPlayer->newWeapon == kWeapDynamite)
        {
            if (pPlayer->curWeapon == kWeapDynamite)
            {
                if (checkAmmo2(pPlayer, 10, 1))
                    pPlayer->newWeapon = kWeapProximity;
                else if (checkAmmo2(pPlayer, 11, 1))
                    pPlayer->newWeapon = kWeapRemote;
            }
            else if (pPlayer->curWeapon == kWeapProximity)
            {
                if (checkAmmo2(pPlayer, 11, 1))
                    pPlayer->newWeapon = kWeapRemote;
                else if (checkAmmo2(pPlayer, 5, 1) && pPlayer->isUnderwater == 0)
                    pPlayer->newWeapon = kWeapDynamite;
            }
            else if (pPlayer->curWeapon == kWeapRemote)
            {
                if (checkAmmo2(pPlayer, 5, 1) && pPlayer->isUnderwater == 0)
                    pPlayer->newWeapon = kWeapDynamite;
                else if (checkAmmo2(pPlayer, 10, 1))
                    pPlayer->newWeapon = kWeapProximity;
            }
            else
            {
                if (checkAmmo2(pPlayer, 5, 1) && pPlayer->isUnderwater == 0)
                    pPlayer->newWeapon = kWeapDynamite;
                else if (checkAmmo2(pPlayer, 10, 1))
                    pPlayer->newWeapon = kWeapVoodooDoll;
                else if (checkAmmo2(pPlayer, 11, 1))
                    pPlayer->newWeapon = kWeapRemote;
            }
        }
        else if ((pPlayer->newWeapon == kWeapSpraycan) && !VanillaMode())
        {
            if ((pPlayer->curWeapon == kWeapSpraycan) && (pPlayer->weaponState == 2)) // fix spray can state glitch when switching from spray to tnt and back quickly
            {
                pPlayer->weaponState = 1;
                pPlayer->newWeapon = kWeapNone;
                return;
            }
        }
        if (pPlayer->pXSprite->health == 0 || pPlayer->hasWeapon[pPlayer->newWeapon] == 0)
        {
            pPlayer->newWeapon = kWeapNone;
            return;
        }
        if (pPlayer->isUnderwater && BannedUnderwater(pPlayer->newWeapon) && !checkFired6or7(pPlayer))
        {
            pPlayer->newWeapon = kWeapNone;
            return;
        }
        int nWeapon = pPlayer->newWeapon;
        int v4c = weaponModes[nWeapon].update;
        if (!pPlayer->curWeapon)
        {
            int nAmmoType = weaponModes[nWeapon].ammoType;
            if (v4c > 1)
            {
                if (CheckAmmo(pPlayer, nAmmoType, 1) || nAmmoType == 11)
                    WeaponRaise(pPlayer);
                pPlayer->newWeapon = kWeapNone;
            }
            else
            {
                if (CheckWeaponAmmo(pPlayer, nWeapon, nAmmoType, 1))
                    WeaponRaise(pPlayer);
                else
                {
                    pPlayer->weaponState = 0;
                    int t;
                    int weapon = WeaponFindLoaded(pPlayer, &t);
                    pPlayer->weaponMode[weapon] = t;
                    if (pPlayer->curWeapon)
                    {
                        WeaponLower(pPlayer);
                        pPlayer->nextWeapon = weapon;
                        return;
                    }
                    pPlayer->newWeapon = weapon;
                }
            }
            return;
        }
        if (nWeapon == pPlayer->curWeapon && v4c <= 1)
        {
            pPlayer->newWeapon = kWeapNone;
            return;
        }
        int i = 0;
        if (nWeapon == pPlayer->curWeapon)
            i = 1;
        for (; i <= v4c; i++)
        {
            int v6c = (pPlayer->weaponMode[nWeapon]+i)%v4c;
            if (CheckWeaponAmmo(pPlayer, nWeapon, weaponModes[nWeapon].ammoType, 1))
            {
                WeaponLower(pPlayer);
                pPlayer->weaponMode[nWeapon] = v6c;
                return;
            }
        }
        pPlayer->newWeapon = kWeapNone;
        return;
    }
    if (pPlayer->curWeapon && !CheckAmmo(pPlayer, pPlayer->weaponAmmo, 1) && pPlayer->weaponAmmo != 11)
    {
        pPlayer->weaponState = -1;
        return;
    }
    if (bShoot)
    {
        switch (pPlayer->curWeapon)
        {
        case kWeapPitchFork:
            StartQAV(pPlayer, kQAVPFORK, nClientFirePitchfork);
            return;
        case kWeapSpraycan:
            switch (pPlayer->weaponState)
            {
            case 3:
                pPlayer->weaponState = 4;
                StartQAV(pPlayer, kQAVCANFIRE, nClientFireSpray, 1);
                return;
            }
            break;
        case kWeapDynamite:
            switch (pPlayer->weaponState)
            {
            case 3:
                pPlayer->weaponState = 6;
                pPlayer->fuseTime = -1;
                pPlayer->throwTime = PlayClock;
                StartQAV(pPlayer, kQAVBUNFUSE, nClientExplodeBundle);
                return;
            }
            break;
        case kWeapProximity:
            switch (pPlayer->weaponState)
            {
            case 7:
                pPlayer->weaponQav = qavGetCorrectID(kQAVPROXIDLE);
                pPlayer->weaponState = 9;
                pPlayer->throwTime = PlayClock;
                return;
            }
            break;
        case kWeapRemote:
            switch (pPlayer->weaponState)
            {
            case 10:
                pPlayer->weaponQav = qavGetCorrectID(kQAVREMIDLE1);
                pPlayer->weaponState = 13;
                pPlayer->throwTime = PlayClock;
                return;
            case 11:
                pPlayer->weaponState = 12;
                StartQAV(pPlayer, kQAVREMFIRE, nClientFireRemote);
                return;
            }
            break;
        case kWeapShotgun:
            switch (pPlayer->weaponState)
            {
            case 7:
                pPlayer->weaponState = 6;
                StartQAV(pPlayer, kQAV2SHOTF2, nClientFireShotgun);
                return;
            case 3:
                pPlayer->weaponState = 2;
                StartQAV(pPlayer, kQAVSHOTF1, nClientFireShotgun);
                return;
            case 2:
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVSHOTF2, nClientFireShotgun);
                return;
            }
            break;
        case kWeapTommyGun:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 3, 2))
                StartQAV(pPlayer, kQAV2TOMFIRE, nClientFireTommy, 1);
            else
                StartQAV(pPlayer, kQAVTOMFIRE, nClientFireTommy, 1);
            return;
        case kWeapFlareGun:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 1, 2))
                StartQAV(pPlayer, kQAVFLAR2FIR, nClientFireFlare);
            else
                StartQAV(pPlayer, kQAVFLARFIR2, nClientFireFlare);
            return;
        case kWeapVoodooDoll:
        {
            static int nChance[] = { 0xa000, 0xc000, 0xe000, 0x10000 };
            int nRand = wrand()*2;
            int i;
            for (i = 0; nChance[i] < nRand; i++)
            {
            }
            pPlayer->voodooTarget = pPlayer->aimTarget;
            if (pPlayer->voodooTarget == -1 || sprite[pPlayer->voodooTarget].statnum != kStatDude)
                i = 4;
            StartQAV(pPlayer,kQAVVDFIRE1 + i, nClientFireVoodoo);
            return;
        }
        case kWeapTeslaCannon:
            switch (pPlayer->weaponState)
            {
            case 2:
                pPlayer->weaponState = 4;
                if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
                    StartQAV(pPlayer, kQAV2SGUNFIR, nClientFireTesla);
                else
                    StartQAV(pPlayer, kQAVSGUNFIR1, nClientFireTesla);
                return;
            case 5:
                if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
                    StartQAV(pPlayer, kQAV2SGUNFIR, nClientFireTesla);
                else
                    StartQAV(pPlayer, kQAVSGUNFIR1, nClientFireTesla);
                return;
            }
            break;
        case kWeapNapalm:
            if (powerupCheck(pPlayer, kPwUpTwoGuns))
                StartQAV(pPlayer, kQAV2NAPFIRE, nClientFireNapalm);
            else
                StartQAV(pPlayer, kQAVNAPFIRE, nClientFireNapalm);
            return;
        case kWeapLifeLeech:
            sfxPlay3DSound(pPlayer->pSprite, 494, 2, 0);
            StartQAV(pPlayer, kQAVSTAFIRE4, nClientFireLifeLeech);
            return;
        case kWeapBeast:
            StartQAV(pPlayer, kQAVBSTATAK1 + Random(4), nClientFireBeast);
            return;
        }
    }
    if (bShoot2)
    {
        switch (pPlayer->curWeapon)
        {
        case kWeapPitchFork:
            StartQAV(pPlayer, kQAVPFORK, nClientFirePitchfork);
            return;
        case kWeapSpraycan:
            switch (pPlayer->weaponState)
            {
            case 3:
                pPlayer->weaponState = 5;
                StartQAV(pPlayer, kQAVCANFIRE2, nClientExplodeCan);
                return;
            }
            break;
        case kWeapDynamite:
            switch (pPlayer->weaponState)
            {
            case 3:
                pPlayer->weaponState = 4;
                StartQAV(pPlayer, kQAVBUNFUSE, nClientExplodeBundle);
                return;
            case 7:
                pPlayer->weaponState = 8;
                StartQAV(pPlayer, kQAVPROXDROP, nClientDropProx);
                return;
            case 10:
                pPlayer->weaponState = 11;
                StartQAV(pPlayer, kQAVREMDROP, nClientDropRemote);
                return;
            case 11:
                if (pPlayer->ammoCount[11] > 0)
                {
                    pPlayer->weaponState = 10;
                    StartQAV(pPlayer, kQAVREMUP1);
                }
                return;
            }
            break;
        case kWeapProximity:
            switch (pPlayer->weaponState)
            {
            case 7:
                pPlayer->weaponState = 8;
                StartQAV(pPlayer, kQAVPROXDROP, nClientDropProx);
                return;
            }
            break;
        case kWeapRemote:
            switch (pPlayer->weaponState)
            {
            case 10:
                pPlayer->weaponState = 11;
                StartQAV(pPlayer, kQAVREMDROP, nClientDropRemote);
                return;
            case 11:
                if (pPlayer->ammoCount[11] > 0)
                {
                    pPlayer->weaponState = 10;
                    StartQAV(pPlayer, kQAVREMUP1);
                }
                return;
            }
            break;
        case kWeapShotgun:
            switch (pPlayer->weaponState)
            {
            case 7:
                pPlayer->weaponState = 6;
                StartQAV(pPlayer, kQAV2SHOTFIR, nClientFireShotgun);
                return;
            case 3:
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVSHOTF3, nClientFireShotgun);
                return;
            case 2:
                pPlayer->weaponState = 1;
                StartQAV(pPlayer, kQAVSHOTF2, nClientFireShotgun);
                return;
            }
            break;
        case kWeapTommyGun:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 3, 2))
                StartQAV(pPlayer, kQAV2TOMALT, nClientAltFireSpread2);
            else
                StartQAV(pPlayer, kQAVTOMSPRED, nClientAltFireSpread2);
            return;
        case kWeapVoodooDoll:
            sfxPlay3DSound(pPlayer->pSprite, 461, 2, 0);
            StartQAV(pPlayer, kQAVVDSPEL1, nClientAltFireVoodoo);
            return;
#if 0
        case kWeapFlareGun:
            if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 1, 2))
                StartQAV(pPlayer, kQAVFLAR2FIR, nClientFireFlare, 0);
            else
                StartQAV(pPlayer, kQAVFLARFIR2, nClientFireFlare, 0);
            return;
#endif
        case kWeapTeslaCannon:
            if (checkAmmo2(pPlayer, 7, 35))
            {
                if (checkAmmo2(pPlayer, 7, 70) && powerupCheck(pPlayer, kPwUpTwoGuns))
                    StartQAV(pPlayer, kQAV2SGUNALT, nClientFireTesla);
                else
                    StartQAV(pPlayer, kQAVSGUNFIR4, nClientFireTesla);
            }
            else
            {
                if (checkAmmo2(pPlayer, 7, 10) && powerupCheck(pPlayer, kPwUpTwoGuns))
                    StartQAV(pPlayer, kQAV2SGUNFIR, nClientFireTesla);
                else
                    StartQAV(pPlayer, kQAVSGUNFIR1, nClientFireTesla);
            }
            return;
        case kWeapNapalm:
            if (powerupCheck(pPlayer, kPwUpTwoGuns))
                // by NoOne: allow napalm launcher alt fire act like in v1.0x versions
                if (gGameOptions.weaponsV10x && !VanillaMode()) StartQAV(pPlayer, kQAV2NAPFIR2, nClientFireNapalm2);
                else StartQAV(pPlayer, kQAV2NAPFIRE, nClientAltFireNapalm);
            else
                StartQAV(pPlayer, kQAVNAPFIRE, (gGameOptions.weaponsV10x && !VanillaMode()) ? nClientFireNapalm : nClientAltFireNapalm);
            return;
        case kWeapFlareGun:
            if (CheckAmmo(pPlayer, 1, 8))
            {
                if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 1, 16))
                    StartQAV(pPlayer, kQAVFLAR2FIR, nClientAltFireFlare);
                else
                    StartQAV(pPlayer, kQAVFLARFIR2, nClientAltFireFlare);
            }
            else
            {
                if (powerupCheck(pPlayer, kPwUpTwoGuns) && checkAmmo2(pPlayer, 1, 2))
                    StartQAV(pPlayer, kQAVFLAR2FIR, nClientFireFlare);
                else
                    StartQAV(pPlayer, kQAVFLARFIR2, nClientFireFlare);
            }
            return;
        case kWeapLifeLeech:
            if (gGameOptions.nGameType <= 1 && !checkAmmo2(pPlayer, 8, 1) && pPlayer->pXSprite->health < (25 << 4))
            {
                sfxPlay3DSound(pPlayer->pSprite, 494, 2, 0);
                StartQAV(pPlayer, kQAVSTAFIRE4, nClientFireLifeLeech);
            }
            else
            {
                StartQAV(pPlayer, kQAVSTAFDOWN);
                AltFireLifeLeech(1, pPlayer);
                pPlayer->weaponState = -1;
            }
            return;
        }
    }
    WeaponUpdateState(pPlayer);
}

void teslaHit(spritetype *pMissile, int a2)
{
    uint8_t sectmap[(kMaxSectors+7)>>3];
    int x = pMissile->x;
    int y = pMissile->y;
    int z = pMissile->z;
    int nDist = 300;
    int nSector = pMissile->sectnum;
    int nOwner = pMissile->owner;
    const bool newSectCheckMethod = !cl_bloodvanillaexplosions && !VanillaMode(); // use new sector checking logic
    GetClosestSpriteSectors(nSector, x, y, nDist, sectmap, nullptr, newSectCheckMethod);
    bool v4 = true;
    DBloodActor* actor = nullptr;
    actHitcodeToData(a2, &gHitInfo, &actor);
    if (a2 == 3 && actor && actor->s().statnum == kStatDude)
        v4 = false;
    int nSprite;
    StatIterator it(kStatDude);
    while ((nSprite = it.NextIndex()) >= 0)
    {
        if (nSprite != nOwner || v4)
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite->flags&32)
                continue;
            if (TestBitString(sectmap, pSprite->sectnum) && CheckProximity(pSprite, x, y, z, nSector, nDist))
            {
                int dx = pMissile->x-pSprite->x;
                int dy = pMissile->y-pSprite->y;
                int nDamage = ClipLow((nDist-(ksqrt(dx*dx+dy*dy)>>4)+20)>>1, 10);
                if (nSprite == nOwner)
                    nDamage /= 2;
                actDamageSprite(nOwner, pSprite, kDamageTesla, nDamage<<4);
            }
        }
    }
    it.Reset(kStatThing);
    while ((nSprite = it.NextIndex()) >= 0)
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->flags&32)
            continue;
        if (TestBitString(sectmap, pSprite->sectnum) && CheckProximity(pSprite, x, y, z, nSector, nDist))
        {
            XSPRITE *pXSprite = &xsprite[pSprite->extra];
            if (!pXSprite->locked)
            {
                int dx = pMissile->x-pSprite->x;
                int dy = pMissile->y-pSprite->y;
                int nDamage = ClipLow(nDist-(ksqrt(dx*dx+dy*dy)>>4)+20, 20);
                actDamageSprite(nOwner, pSprite, kDamageTesla, nDamage<<4);
            }
        }
    }
}

END_BLD_NS
