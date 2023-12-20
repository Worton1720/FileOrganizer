// { g++ main.cpp -o main -lole32 -loleaut32 -luuid -lshlwapi }
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept> // �������� ������������ ���� ��� std::runtime_error
#include <windows.h> // �������� ������������ ���� ��� SetConsoleTextAttribute
#include <shlobj.h>
#include <ObjBase.h>
#include "filesystem.hpp"
#include "fs_std.hpp"
// ���������� ����� fs::*
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
using namespace std;

// ��������� ��� �������� ��������� � ��������������� ����������
struct Category
{
    string name;
    vector<string> extensions;
};

// ������ ���������
vector<Category> categories{
    {"Video", {".mp4", ".mov", ".avi", ".mkv", ".wmv", ".3gp", ".3g2", ".mpg", ".mpeg", ".m4v", ".h264", ".flv", ".rm", ".swf", ".vob"}},
    {"Code", {".py", ".js", ".html", ".jar", ".json", ".java", ".cpp", ".css", ".php"}},
    {"Data", {".sql", ".sqlite", ".sqlite3", ".csv", ".dat", ".db", ".log", ".mdb", ".sav", ".tar", ".xml", ".xlsx", ".xls", ".xlsm", ".ods"}},
    {"Audio", {".mp3", ".wav", ".ogg", ".flac", ".aif", ".mid", ".midi", ".mpa", ".wma", ".wpl", ".cda", ".aac", ".m4a"}},
    {"Images", {".jpg", ".png", ".bmp", ".ai", ".psd", ".ico", ".jpeg", ".ps", ".svg", ".tif", ".tiff", ".gif", ".eps"}},
    {"Archives", {".zip", ".rar", ".7z", ".z", ".gz", ".rpm", ".arj", ".pkg", ".deb", ".tar.gz", ".tar.bz2"}},
    {"Text", {".pdf", ".txt", ".doc", ".docx", ".rtf", ".tex", ".wpd", ".odt"}},
    {"Presentations", {".pptx", ".ppt", ".pps", ".key", ".odp"}},
    {"Fonts", {".otf", ".ttf", ".fon", ".fnt"}},
    {"Installers", {".torrent", ".msi", ".exe"}},
    {"Mobile", {".apk", ".obb"}},
    {"Backups", {".bak", ".bak2"}},
    {"Other", {}}};

// ��������� ��� ����� ������ ����
const char PATH_SEPARATOR = '/';
const string DEFAULT_CATEGORY = "Other";

// ������� ��� �������� �������������� �����
void createCategoryFolders(const fs::path &folderPath)
{
    for (const auto &cat : categories)
    {
        fs::path path = folderPath / cat.name;
        fs::create_directory(path);
    }
}


// ������� ��� ����������� ����� � ��������������� �����
void moveFileToCategory(const fs::path &filePath)
{
    try
    {
        // �������� ��� ����� � ��� ����������
        string fileName = filePath.filename().string();
        string extension = filePath.extension().string();

        // ���� ���������, ��������������� ���������� �����
        auto it = find_if(categories.begin(), categories.end(),
                          [&](const Category &c)
                          {
                              return find(c.extensions.begin(),
                                          c.extensions.end(),
                                          extension) != c.extensions.end();
                          });

        // ���� ��������� �� �������, ���������� ��������� �� ���������
        Category *category = it != categories.end() ? &*it : nullptr;

        if (category == nullptr)
        {
            category = &categories.back();
        }

        // ������ ���� � ������ ����� �����
        fs::path destPath = filePath.parent_path() / category->name;

        // ���������� ����� � ���� (���� ����������)
        destPath = destPath.make_preferred();

        // ���� ����� �� ����������, ������� �
        if (!fs::exists(destPath))
        {
            fs::create_directory(destPath);
        }

        // ���������, �� ���������� �� ��� ���� � ����� ������ � �����
        for (int i = 1; fs::exists(destPath / fileName); ++i)
        {
            fileName = filePath.stem().string() + "_" + to_string(i) + extension;
        }

        // ������ ������������� ���� ��� �����
        fs::path finalPath = destPath / fileName;
        finalPath = finalPath.make_preferred();

        // ��������������� ����
        fs::rename(filePath, finalPath);

        // ������� ���������� � ����������� �����
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
        cout << "Move: " << fileName << " to " << category->name << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
    catch (const exception &e)
    {
        // ������� ��������� �� ������ ��� ������������� ����������
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while moving file: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
        // ����� �������� �������������� ��������, ����� ��� ���������������� ��� ����� ������������
    }
}

// ������� ��� ������������ �������� ������ ����� � �������� ����������
void removeEmptyFolders(const fs::path &folderPath)
{
    try
    {
        // �������� �� ���� ��������� � ������� ����������
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            // ���� ������� ������� �������� �����������
            if (entry.is_directory())
            {
                // ���������� �������� removeEmptyFolders ��� ��������� ����������
                removeEmptyFolders(entry.path());
            }
        }

        // ���������, �������� �� ������� ���������� ������
        if (fs::is_empty(folderPath))
        {
            // �������� ��� ������� ��������� �� ����� ����������
            auto categoryName = folderPath.filename().string();

            // ����� ��������������� ��������� �� �����
            auto it = find_if(categories.begin(), categories.end(), [&](const Category &cat) {
                return cat.name == categoryName;
            });

            // ������� ������� ���������� � ������� ���������, ���� ��� �� �������������� ����������
            fs::remove_all(folderPath);
            if (it == categories.end())
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
                cout << "Remove empty folder: " << folderPath.filename() << endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
            }
        }
    }
    catch (const exception &e)
    {
        // ������� ��������� �� ������ ��� ������������� ����������
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while removing empty folders: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
}


// ������� ��� ���������� ������ � �����
void sortFiles(const fs::path &folderPath)
{
    try
    {
        // ������� �������������� �����
        createCategoryFolders(folderPath);

        // ���������� �����
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                moveFileToCategory(entry.path());
            }
        }

        // ������� ��� ������ �����, ����� ��������������
        removeEmptyFolders(folderPath);
    }
    catch (const exception &e)
    {
        // ������� ��������� �� ������ ��� ������������� ����������
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while sorting files: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
}

fs::path getFolderPathFromExplorer()
{
    // �������� ����������� ���� ���������� ��� ������ �����
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IFileOpenDialog *pFileOpen;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        pFileOpen->GetOptions(&dwOptions);
        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

        // ����������� ����������� ����
        hr = pFileOpen->Show(NULL);

        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);

            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr))
                {
                    fs::path selectedPath(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                    pItem->Release();
                    pFileOpen->Release();
                    return selectedPath;
                }
            }
        }

        pFileOpen->Release();
    }

    CoUninitialize();
    return fs::path(); // ���������� ������ ����, ���� ����� ����� ������� ��� ��������� ������
}


int main()
{
    system("chcp 1251");

    while (true)
    {
        system("cls");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_YELLOW);
        std::cout << "\n------- ������ ������ -------\n";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);


        std::cout << "������� ���� � �����. ������� enter ��� ������ ����� Download. \n";
        std::string mainPath = "";

        while (mainPath.empty())
        {
            std::cout << "���� � �����: ";
            std::getline(std::cin, mainPath);

            
            if (mainPath.empty())
            {
                CoInitialize(nullptr);

                CHAR linkPathDef[MAX_PATH];
                HRESULT result = SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, linkPathDef);

                if (result != S_OK)
                    std::cout << "Error: " << result << "\n";
                else
                    strcat_s(linkPathDef, "\\Downloads"); // ������� � ���� "Downloads"

                mainPath = linkPathDef;
                std::cout << "������� ����� Download!\n";

                CoUninitialize();
            }
            else
            {
                // �������� ������� ������� � ������ � ����� ����
                if (!mainPath.empty() && (mainPath.front() == '\"' || mainPath.front() == '\'') &&
                    (mainPath.back() == '\"' || mainPath.back() == '\''))
                {
                    // ������� ������� �� ������ � ����� ����
                    mainPath = mainPath.substr(1, mainPath.length() - 2);
                }
            }

            cout << mainPath << endl;
            if (!fs::path(mainPath).is_absolute())
            {
                std::cout << "��������� ���� �� �������� ��������������. ���������� �����.\n";
                mainPath = "";
                continue;
            }

            // ����������� ������ � ������ std::filesystem::path
            fs::path fsMainPath = mainPath;

            // ��������������� ���� � ����������������� �������
            fsMainPath.make_preferred();

            // ��������� �������� �� ������������� �����
            if (!mainPath.empty() && !fs::exists(fsMainPath))
            {
                cerr << "Error: Folder does not exist." << endl;
                return 1;
            }

            // �������� ������� ����������, ��������� std::filesystem::path
            sortFiles(fsMainPath);

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_YELLOW);
            std::cout << "------- ����� ������ -------\n";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
        }

        std::cout << "\n������� ����������? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice != "y" && choice != "�")
        {
            system("cls");
            break;
        }
    }

    return 0;
}