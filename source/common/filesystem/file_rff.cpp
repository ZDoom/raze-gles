/*
** file_rff.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2009 Randy Heit
** Copyright 2005-2009 Christoph Oelckers
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
#include <algorithm>
#include "resourcefile.h"

//==========================================================================
//
//
//
//==========================================================================

struct RFFInfo
{
	// Should be "RFF\x18"
	uint32_t		Magic;
	uint32_t		Version;
	uint32_t		DirOfs;
	uint32_t		NumLumps;
};

struct RFFLump
{
	uint32_t		DontKnow1[4];
	uint32_t		FilePos;
	uint32_t		Size;
	uint32_t		DontKnow2;
	uint32_t		Time;
	uint8_t		Flags;
	char		Extension[3];
	char		Name[8];
};

//==========================================================================
//
// Blood RFF lump (uncompressed lump with encryption)
//
//==========================================================================

struct FRFFLump : public FUncompressedLump
{
	virtual FileReader *GetReader();
	int ValidataCache() override;

	uint32_t		IndexNum;
};

//==========================================================================
//
// BloodCrypt
//
//==========================================================================

void BloodCrypt (void *data, int key, int len)
{
	int p = (uint8_t)key, i;

	for (i = 0; i < len; ++i)
	{
		((uint8_t *)data)[i] ^= (unsigned char)(p+(i>>1));
	}
}


//==========================================================================
//
// Blood RFF file
//
//==========================================================================

class FRFFFile : public FResourceFile
{
	TArray<FRFFLump> Lumps;

public:
	FRFFFile(const char * filename, FileReader &file);
	virtual bool Open(bool quiet);
	virtual FResourceLump *GetLump(int no) { return ((unsigned)no < NumLumps)? &Lumps[no] : NULL; }
};


//==========================================================================
//
// Initializes a Blood RFF file
//
//==========================================================================

FRFFFile::FRFFFile(const char *filename, FileReader &file)
: FResourceFile(filename, file)
{
}

//==========================================================================
//
// Initializes a Blood RFF file
//
//==========================================================================

bool FRFFFile::Open(bool quiet)
{
	RFFLump *lumps;
	RFFInfo header;

	Reader.Read(&header, sizeof(header));

	NumLumps = LittleLong(header.NumLumps);
	header.DirOfs = LittleLong(header.DirOfs);
	lumps = new RFFLump[header.NumLumps];
	Reader.Seek (header.DirOfs, FileReader::SeekSet);
	Reader.Read (lumps, header.NumLumps * sizeof(RFFLump));
	BloodCrypt (lumps, header.DirOfs, header.NumLumps * sizeof(RFFLump));

	Lumps.Grow(NumLumps);

	//if (!quiet) Printf(", %d lumps\n", NumLumps);
	for (uint32_t i = 0; i < NumLumps; ++i)
	{
		Lumps.Reserve(1);
		auto& Lump = Lumps.Last();
		
		Lump.Position = LittleLong(lumps[i].FilePos);
		Lump.LumpSize = LittleLong(lumps[i].Size);
		Lump.Owner = this;
		if (lumps[i].Flags & 0x10)
		{
			Lump.Flags |= LUMPF_BLOODCRYPT;
		}
		Lump.ResourceId = LittleLong(lumps[i].ResourceId);
		// Rearrange the name and extension to construct the fullname.
		char name[13];
		strncpy(name, lumps[i].Name, 8);
		name[8] = 0;
		size_t len = strlen(name);
		assert(len + 4 <= 12);
		name[len+0] = '.';
		name[len+1] = lumps[i].Extension[0];
		name[len+2] = lumps[i].Extension[1];
		name[len+3] = lumps[i].Extension[2];
		name[len+4] = 0;
		Lump.LumpNameSetup(name);
	}
	delete[] lumps;
	return true;
}


//==========================================================================
//
// Get reader (only returns non-NULL if not encrypted)
//
//==========================================================================

FileReader *FRFFLump::GetReader()
{
	// Don't return the reader if this lump is encrypted
	// In that case always force caching of the lump
	if (!(Flags & LUMPF_BLOODCRYPT))
	{
		return FUncompressedLump::GetReader();
	}
	else
	{
		return NULL;
	}
}

//==========================================================================
//
// Fills the lump cache and performs decryption
//
//==========================================================================

int FRFFLump::ValidateCache()
{
	int res = FUncompressedLump::ValidateCache();

	if (res && )(Flags & LUMPF_BLOODCRYPT))
	{
		int cryptlen = std::min<int> (LumpSize, 256);
		uint8_t *data = Cache.Data();
		
		for (int i = 0; i < cryptlen; ++i)
		{
			data[i] ^= i >> 1;
		}
	}
	return res;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckRFF(const char *filename, FileReader &file, bool quiet)
{
	char head[4];

	if (file.GetLength() >= 16)
	{
		file.Seek(0, FileReader::SeekSet);
		file.Read(&head, 4);
		file.Seek(0, FileReader::SeekSet);
		if (!memcmp(head, "RFF\x1a", 4))
		{
			FResourceFile *rf = new FRFFFile(filename, file);
			if (rf->Open(quiet)) return rf;

			file = std::move(rf->Reader); // to avoid destruction of reader
			delete rf;
		}
	}
	return NULL;
}



