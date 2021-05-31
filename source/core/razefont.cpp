/*
** razefont.cpp
**
**---------------------------------------------------------------------------
** Copyright 2021 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/  

#include "razefont.h"
#include "gamecontrol.h"
#include "c_cvars.h"
#include "i_interface.h"
#include "vm.h"
#include "gstrings.h"

FGameTexture* GetBaseForChar(FGameTexture* t);
void FontCharCreated(FGameTexture* base, FGameTexture* glyph);


FFont* IndexFont;
FFont* DigiFont;
FFont* BigFont13, * BigFont15;

CUSTOM_CVAR(Int, duke_menufont, -1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG | CVAR_NOINITCALL)
{
	if (!(g_gameType & GAMEFLAG_DUKE) || !BigFont13 || !BigFont15) return;
	if (self < -1 || self > 1) self = -1;
	else
	{
		// Font info must be copied so that BigFont does not change its address.
		if (self == 0 || (self == -1 && isPlutoPak())) BigFont->CopyFrom(*BigFont15);
		else if (self == 1 || (self == -1 && !isPlutoPak())) BigFont->CopyFrom(*BigFont13);
	}
}


static void SetupHires(FFont *font)
{
	if (!font) return;
	auto altfont = font->AltFont();
	if (!altfont) return;
	// Set up hightile links for the font.
	for (int i = 33; i < 127; i++)
	{
		auto mycode = font->GetCharCode(i, true);
		if (mycode != i) continue;
		auto mychar = font->GetChar(i, CR_UNDEFINED, nullptr);
		if (mychar == nullptr) continue;

		auto altcode = altfont->GetCharCode(i, true);
		if (altcode != i) continue;
		auto altchar = altfont->GetChar(i, CR_UNDEFINED, nullptr);
		if (altchar == nullptr) continue;

		auto base = GetBaseForChar(altchar);
		if (base == nullptr) continue;
		FontCharCreated(base, mychar);
	}
}

void InitFont()
{
	V_InitFonts();
	BigFont = V_GetFont("BigFont");
	SmallFont = V_GetFont("SmallFont");
	SmallFont2 = V_GetFont("SmallFont2");
	IndexFont = V_GetFont("IndexFont");
	DigiFont = V_GetFont("DigiFont");

	SetupHires(BigFont);
	SetupHires(SmallFont);

	if (g_gameType & GAMEFLAG_DUKE)
	{
		BigFont13 = V_GetFont("BigFont13");
		BigFont15 = V_GetFont("BigFont15");
		BigFont = new FFont(0, "BigFont");
	}

	// todo: Compare small and big fonts with the base font and decide which one to use.
}

FFont* PickBigFont(const char* txt)
{
	if (generic_ui) return NewSmallFont; // Note: Support is incomplete. Translations do not exist anyway for most content.
	if (!OriginalBigFont || OriginalBigFont == BigFont) return BigFont;
	if (txt && *txt == '$') txt = GStrings[txt + 1];
	if (!txt || !*txt) txt = GStrings["REQUIRED_CHARACTERS"];
	if (!txt || !*txt || BigFont->CanPrint(txt)) return BigFont;
	return OriginalBigFont;
}

static FFont* PickBigFont_(const FString& str)
{
	return PickBigFont(str.GetChars());
}

DEFINE_ACTION_FUNCTION_NATIVE(_Raze, PickBigFont, PickBigFont_)
{
	PARAM_PROLOGUE;
	PARAM_STRING(text);
	//PARAM_POINTER(cr, int);
	ACTION_RETURN_POINTER(PickBigFont(text));
}

FFont* PickSmallFont(const char* txt)
{
	if (generic_ui) return NewSmallFont; // Note: Support is incomplete. Translations do not exist anyway for most content.
	if (!OriginalSmallFont || OriginalSmallFont == SmallFont) return SmallFont;
	if (txt && *txt == '$') txt = GStrings[txt + 1];
	if (!txt || !*txt) txt = GStrings["REQUIRED_CHARACTERS"];
	if (!txt || !*txt || SmallFont->CanPrint(txt)) return SmallFont;
	return OriginalSmallFont;
}

static FFont* PickSmallFont_(const FString& str)
{
	return PickSmallFont(str.GetChars());
}

DEFINE_ACTION_FUNCTION_NATIVE(_Raze, PickSmallFont, PickSmallFont_)
{
	PARAM_PROLOGUE;
	PARAM_STRING(text);
	//PARAM_POINTER(cr, int);
	ACTION_RETURN_POINTER(PickBigFont(text));
}