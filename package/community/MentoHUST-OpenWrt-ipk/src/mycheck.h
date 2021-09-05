/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mycheck.h
* 摘	要：客户端校验算法
* 作	者：kkHAIKE
*/
#ifndef MYCHECK_H
#define MYCHECK_H

#include "types.h"

typedef struct _IMAGE_DOS_HEADER {	// DOS .EXE header
	WORD	e_magic;				// Magic number
	WORD	e_cblp;					// Bytes on last page of file
	WORD	e_cp;					// Pages in file
	WORD	e_crlc;					// Relocations
	WORD	e_cparhdr;				// Size of header in paragraphs
	WORD	e_minalloc;				// Minimum extra paragraphs needed
	WORD	e_maxalloc;				// Maximum extra paragraphs needed
	WORD	e_ss;					// Initial (relative) SS value
	WORD	e_sp;					// Initial SP value
	WORD	e_csum;					// Checksum
	WORD	e_ip;					// Initial IP value
	WORD	e_cs;					// Initial (relative) CS value
	WORD	e_lfarlc;				// File address of relocation table
	WORD	e_ovno;					// Overlay number
	WORD	e_res[4];				// Reserved words
	WORD	e_oemid;				// OEM identifier (for e_oeminfo)
	WORD	e_oeminfo;				// OEM information; e_oemid specific
	WORD	e_res2[10];				// Reserved words
	LONG	e_lfanew;				// File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
	WORD	Machine;
	WORD	NumberOfSections;
	DWORD	TimeDateStamp;
	DWORD	PointerToSymbolTable;
	DWORD	NumberOfSymbols;
	WORD	SizeOfOptionalHeader;
	WORD	Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
	DWORD	VirtualAddress;
	DWORD	Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES	16
typedef struct _IMAGE_OPTIONAL_HEADER {
	//
	// Standard fields.
	//
	
	WORD	Magic;
	BYTE	MajorLinkerVersion;
	BYTE	MinorLinkerVersion;
	DWORD	SizeOfCode;
	DWORD	SizeOfInitializedData;
	DWORD	SizeOfUninitializedData;
	DWORD	AddressOfEntryPoint;
	DWORD	BaseOfCode;
	DWORD	BaseOfData;
	
	//
	// NT additional fields.
	//
	
	DWORD	ImageBase;
	DWORD	SectionAlignment;
	DWORD	FileAlignment;
	WORD	MajorOperatingSystemVersion;
	WORD	MinorOperatingSystemVersion;
	WORD	MajorImageVersion;
	WORD	MinorImageVersion;
	WORD	MajorSubsystemVersion;
	WORD	MinorSubsystemVersion;
	DWORD	Win32VersionValue;
	DWORD	SizeOfImage;
	DWORD	SizeOfHeaders;
	DWORD	CheckSum;
	WORD	Subsystem;
	WORD	DllCharacteristics;
	DWORD	SizeOfStackReserve;
	DWORD	SizeOfStackCommit;
	DWORD	SizeOfHeapReserve;
	DWORD	SizeOfHeapCommit;
	DWORD	LoaderFlags;
	DWORD	NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_SHORT_NAME			8
typedef struct _IMAGE_SECTION_HEADER {
	char	Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD	PhysicalAddress;
		DWORD	VirtualSize;
	} Misc;
	DWORD	VirtualAddress;
	DWORD	SizeOfRawData;
	DWORD	PointerToRawData;
	DWORD	PointerToRelocations;
	DWORD	PointerToLinenumbers;
	WORD	NumberOfRelocations;
	WORD	NumberOfLinenumbers;
	DWORD	Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _PE_HEADER_MAP
{
	DWORD signature;
	IMAGE_FILE_HEADER _head;
	IMAGE_OPTIONAL_HEADER opt_head;
	IMAGE_SECTION_HEADER section_header[8];
}PE_HEADER_MAP,*PPE_HEADER_MAP;

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
#ifdef WORDS_BIGENDIAN
	union {
		struct {
			DWORD NameIsString:1;
			DWORD NameOffset:31;
		};
		DWORD	Name;
		struct {
			WORD	Id_unuse;
			WORD	Id;
		};
	};
	union {
		DWORD	OffsetToData;
		struct {
			DWORD	DataIsDirectory:1;
			DWORD	OffsetToDirectory:31;
		};
	};
#else
	union {
		struct {
			DWORD NameOffset:31;
			DWORD NameIsString:1;
		};
		DWORD	Name;
		WORD	Id;
	};
	union {
		DWORD	OffsetToData;
		struct {
			DWORD	OffsetToDirectory:31;
			DWORD	DataIsDirectory:1;
		};
	};
#endif
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

typedef struct _IMAGE_RESOURCE_DIRECTORY {
	DWORD	Characteristics;
	DWORD	TimeDateStamp;
	WORD	MajorVersion;
	WORD	MinorVersion;
	WORD	NumberOfNamedEntries;
	WORD	NumberOfIdEntries;
  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
	DWORD	OffsetToData;
	DWORD	Size;
	DWORD	CodePage;
	DWORD	Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

typedef struct tagVS_FIXEDFILEINFO {
	DWORD	dwSignature;			/* e.g. 0xfeef04bd */
	DWORD	dwStrucVersion;			/* e.g. 0x00000042 = "0.42" */
	DWORD	dwFileVersionMS;		/* e.g. 0x00030075 = "3.75" */
	DWORD	dwFileVersionLS;		/* e.g. 0x00000031 = "0.31" */
	DWORD	dwProductVersionMS;		/* e.g. 0x00030010 = "3.10" */
	DWORD	dwProductVersionLS;		/* e.g. 0x00000031 = "0.31" */
	DWORD	dwFileFlagsMask;		/* = 0x3F for version "0.42" */
	DWORD	dwFileFlags;			/* e.g. VFF_DEBUG | VFF_PRERELEASE */
	DWORD	dwFileOS;				/* e.g. VOS_DOS_WINDOWS16 */
	DWORD	dwFileType;				/* e.g. VFT_DRIVER */
	DWORD	dwFileSubtype;			/* e.g. VFT2_DRV_KEYBOARD */
	DWORD	dwFileDateMS;			/* e.g. 0 */
	DWORD	dwFileDateLS;			/* e.g. 0 */
} VS_FIXEDFILEINFO;

typedef struct _VS_VERSIONINFO { 
	WORD	wLength;
	WORD	wValueLength;
	WORD	wType;
	WORD	szKey[16];
	WORD	Padding1[1];
	VS_FIXEDFILEINFO Value;
} VS_VERSIONINFO, *PVS_VERSIONINFO;

#define IMAGE_SCN_CNT_CODE				0x00000020	// Section contains code
#define IMAGE_SCN_MEM_EXECUTE			0x20000000	// Section is executable

#ifdef WORDS_BIGENDIAN
#define LTOBS(x) ltobs(x)
#define LTOBL(x) ltobl(x)
WORD ltobs(WORD x);
DWORD ltobl(DWORD x);
#else
#define LTOBS(x) (x)
#define LTOBL(x) (x)
#endif

int check_init(const char *dataFile);
void V2_check(const BYTE *seed, char *final_str);
void check_free();
DWORD getVer(const char *file);

#endif
