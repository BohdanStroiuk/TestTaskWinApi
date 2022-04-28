#pragma once
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <list>
#include <string>
#include <fstream>
#include <math.h>
#include <omp.h>
#include <conio.h>

#pragma pack(push, 2)
typedef struct {
    WORD Reserved1;       // reserved, must be 0
    WORD ResourceType;    // type is 1 for icons
    WORD ImageCount;      // number of icons in structure (1)
    BYTE Width;           // icon width (32)
    BYTE Height;          // icon height (32)
    BYTE Colors;          // colors (0 means more than 8 bits per pixel)
    BYTE Reserved2;       // reserved, must be 0
    WORD Planes;          // color planes
    WORD BitsPerPixel;    // bit depth
    DWORD ImageSize;      // size of structure
    WORD ResourceID;      // resource ID
} ICON_ENTRY;
#pragma pack(pop)

class DumpFile
{
private:
    LPCWSTR pathToExe;
    LPCWSTR pathToIcon;
    ICON_ENTRY iconEntry;
   
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    ULONG ulSize;
    PIMAGE_THUNK_DATA thunk;
    PIMAGE_IMPORT_BY_NAME pOrdinalName;
    PIMAGE_DOS_HEADER pDOSHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_IMPORT_DESCRIPTOR importDesc;
    PIMAGE_SECTION_HEADER pSection;
    int  countWinApiW;

    

    std::list<std::string> dllNames;
    std::list<std::string> winAPINames;
    PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD, PIMAGE_NT_HEADERS);
    void ReadNames();
    void CalcCountWinApiW();
    void ChangeIcon();
    void InitIconEntry(size_t);

public:
    DumpFile(LPCWSTR, LPCWSTR);
    ~DumpFile();
    void WriteDllNames();
    void WriteWinApiNames();
    static void CalcEntropy(std::string);
    std::list<std::string> GetDllNames();
    std::list<std::string> GetWinApiNames();
    int GetCountWinApiW();

};

