#pragma once

namespace NSIFF
{
	enum FileType
	{
		FileType_Unknown,
		FileType_FORM,
		FileType_RIFF,
		FileType_RIFX,
	};

	enum FileMode
	{
		FileMode_Unknown,
		FileMode_Flag_Known = (1 << 0),
		FileMode_Flag_64 = (1 << 1),
		FileMode_Flag_BigEndian  = (1 << 2),
		
		FileMode_32LE = FileMode_Flag_Known, // 32bit Little Endian (e.g. RIFF)
		FileMode_32BE = FileMode_Flag_Known|FileMode_Flag_BigEndian, // 32bit Big Endian (e.g. AIFF)
		FileMode_64LE = FileMode_Flag_Known|FileMode_Flag_64, // 64bit Little Endian
		FileMode_64BE = FileMode_Flag_Known|FileMode_Flag_64|FileMode_Flag_BigEndian, // 64bit Big Endian
	};

	static bool FileMode_IsLittleEndian(FileMode mode) { return (mode & FileMode_Flag_BigEndian) == 0; }
	static bool FileMode_IsBigEndian(FileMode mode) { return (mode & FileMode_Flag_BigEndian) == FileMode_Flag_BigEndian; }
	static bool FileMode_Is32(FileMode mode) { return (mode & FileMode_Flag_64) == 0; }
	static bool FileMode_Is64(FileMode mode) { return (mode & FileMode_Flag_64) == FileMode_Flag_64; }
}