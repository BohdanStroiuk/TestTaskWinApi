#include "DumpFile.h"

PIMAGE_SECTION_HEADER DumpFile::GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    for (unsigned i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
    {
        if ((rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }
    return 0;
}

int DumpFile::GetCountWinApiW()
{
    return countWinApiW;
}

void DumpFile::CalcCountWinApiW()
{
    for (std::string name : winAPINames)
    {
        if (name.find("W") != std::string::npos) countWinApiW++;
    }
}

void DumpFile::WriteDllNames()
{
    std::cout << "______DLL names:\n";
    for(std::string name : dllNames)
    {
        std::cout << name << "\n";
    }
}

void DumpFile::WriteWinApiNames()
{
    std::cout << "______WinApi names:\n";
    for (std::string name : winAPINames)
    {
        std::cout << name << "\n";
    }
}

void DumpFile::ReadNames()
{
    int importsStartRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!importsStartRVA) return;
    //определяем адрес секции
    pSection = GetEnclosingSectionHeader(importsStartRVA, pNTHeader);
    if (!pSection) return;
    int delta = pSection->VirtualAddress - pSection->PointerToRawData;
    importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(importsStartRVA - delta + (DWORD)lpFileBase);
    //перебираем список dll
    while (importDesc->TimeDateStamp || importDesc->Name)
    {
        //название dll
        dllNames.push_back(reinterpret_cast<char const*>((PBYTE)(importDesc->Name) - delta + (DWORD)lpFileBase));
 //       printf("%s\n", s.c_str());
        //RVA-смещение на массив указателей на функции
        thunk = (PIMAGE_THUNK_DATA)importDesc->Characteristics;
        if (!thunk) thunk = (PIMAGE_THUNK_DATA)importDesc->FirstThunk;
        thunk = (PIMAGE_THUNK_DATA)((PBYTE)thunk - delta + (DWORD)lpFileBase);
        //перебираем функции
        while (thunk->u1.AddressOfData)
        {
            if (!(thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
            {
                pOrdinalName = (PIMAGE_IMPORT_BY_NAME)thunk->u1.AddressOfData;
                pOrdinalName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pOrdinalName + (DWORD)lpFileBase - delta);
                //выводим имя функции
                winAPINames.push_back(pOrdinalName->Name);
            }
            thunk++;
        }
        importDesc++;
    }
}

void DumpFile::InitIconEntry(size_t bufferSize)
{
    iconEntry.Reserved1 = 0;       // reserved, must be 0
    iconEntry.ResourceType = 1;    // type is 1 for icons
    iconEntry.ImageCount = 1;      // number of icons in structure (1)
    iconEntry.Width = 32;           // icon width (32)
    iconEntry.Height = 32;          // icon height (32)
    iconEntry.Colors = 0;          // colors (0 means more than 8 bits per pixel)
    iconEntry.Reserved2 = 0;       // reserved, must be 0
    iconEntry.Planes = 2;          // color planes
    iconEntry.BitsPerPixel = 32;    // bit depth
    iconEntry.ImageSize = bufferSize - 22;      // size of structure
    iconEntry.ResourceID = 0;      // resource ID
        
}

void DumpFile::CalcEntropy(std::string path)
{
    long code[256] = { 0 };
    char ch;
    std::fstream f;
    long total = 0;
   
    float entr = 0, prob = 0;;

    double t1, t2;

    f.open(path, std::ios::in | std::ios::binary);

    if (!f)
    {
        std::cout << "Error 1: open input file " << path.c_str() << std::endl;
    }

    t1 = omp_get_wtime();
    

    while (!f.eof())
    {
        
        f >> ch;
        if (!f.eof())
        {
            if ((int)ch > 0 && (int)ch < 256)code[(int)ch]++;
            total++;
        }
    }
    f.close();

    for (int i = 0; i < 256; i++)
    {
        if (code[i] == 0) continue;
        prob = code[i] / (float)total;
        entr -= prob * log(prob) / log(2.0f);
    }

    t2 = omp_get_wtime();

    std::cout << "Entropy of the file: " << path.c_str() << std::endl;
    std::cout << "Bytes: " << total << std::endl;

    std::cout.setf(std::ios::binary);
    std::cout.precision(3);

    std::cout << "Entropy: " << entr << std::endl;
    std::cout.precision(10);

    std::cout << "Time: " << t2 - t1 << std::endl;



}

void DumpFile::ChangeIcon()
{
    HANDLE resourceHandle = BeginUpdateResource(pathToExe, FALSE);

    char* buffer;
    size_t bufferSize;

    std::ifstream icon(pathToIcon, std::ios::in | std::ios::binary);

    if (icon.good())
    {
        //get buffer size
        icon.seekg(0, icon.end);
        bufferSize = icon.tellg();

        //copy icon binary into buffer
        buffer = new char[bufferSize];
        icon.seekg(0, icon.beg);
        icon.read(buffer, bufferSize);
        icon.close();

        //update icon image resourse handle

        UpdateResource(resourceHandle, RT_ICON, MAKEINTRESOURCE(1), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), buffer + 22, bufferSize - 22);
        
        //init icon data
        InitIconEntry(bufferSize);

        //update the icon entry resource
        UpdateResource(resourceHandle, RT_GROUP_ICON, L"MAINICON", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), &iconEntry, sizeof(ICON_ENTRY));

        EndUpdateResource(resourceHandle, FALSE);
        delete buffer;

       
    }

   
}

DumpFile::~DumpFile()
{
    
    dllNames.clear();
    winAPINames.clear();
    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
}

DumpFile::DumpFile(LPCWSTR filename, LPCWSTR newIconName)
{
    countWinApiW = 0;
    pathToExe = filename;
    pathToIcon = newIconName;
    
    //открываем файл
    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) return;
    //отображаем файл в память
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hFileMapping)
    {
        CloseHandle(hFile);
        return;
    }
    // преобразовать в указатель
    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (!lpFileBase)
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return;
    }
 
    //указатель на DOS-заголовок
    pDOSHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) return;
    //находим адрес NT заголовка
    pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDOSHeader + pDOSHeader->e_lfanew);
    if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) return;
    //RVA-адрес таблицы импорта
    
    ReadNames();
    CalcCountWinApiW();
    ChangeIcon();
}
