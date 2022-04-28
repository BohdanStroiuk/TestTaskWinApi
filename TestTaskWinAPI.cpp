#include "DumpFile.h"
  
int main()
{
    std::string pathToExe = "D://TestTaskC++//TestFiles//file.exe";
    std::string pathToIcon = "D://TestTaskC++//TestFiles//icon2.ico";

 
    std::cout << "Enter the full path to the exe file: ";
    std::cin >> pathToExe;

    std::cout << "Enter the full path to the icon file: ";
    std::cin >> pathToIcon;

// D://TestTaskC++//TestFiles//file.exe
// D://TestTaskC++//TestFiles//file.exe


    DumpFile* file = new DumpFile(std::wstring(pathToExe.begin(), pathToExe.end()).c_str(), std::wstring(pathToIcon.begin(), pathToIcon.end()).c_str());

    file->WriteDllNames();
    file->WriteWinApiNames();

    std::cout << "\nCount WinApi with 'W': " << file->GetCountWinApiW() <<std::endl;

   
//    file->CalcEntropy(pathToIcon.c_str());

    delete file;

    DumpFile::CalcEntropy(pathToExe);
    DumpFile::CalcEntropy(pathToIcon);

    getchar();
}