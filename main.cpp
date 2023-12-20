// { g++ main.cpp -o main -lole32 -loleaut32 -luuid -lshlwapi }
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept> // Добавлен заголовочный файл для std::runtime_error
#include <windows.h> // Добавлен заголовочный файл для SetConsoleTextAttribute
#include <shlobj.h>
#include <ObjBase.h>
#include "filesystem.hpp"
#include "fs_std.hpp"
// обращаться через fs::*
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
using namespace std;

// Структура для хранения категорий и соответствующих расширений
struct Category
{
    string name;
    vector<string> extensions;
};

// Список категорий
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

// Константы для более ясного кода
const char PATH_SEPARATOR = '/';
const string DEFAULT_CATEGORY = "Other";

// Функция для создания категориальных папок
void createCategoryFolders(const fs::path &folderPath)
{
    for (const auto &cat : categories)
    {
        fs::path path = folderPath / cat.name;
        fs::create_directory(path);
    }
}


// Функция для перемещения файла в соответствующую папку
void moveFileToCategory(const fs::path &filePath)
{
    try
    {
        // Получаем имя файла и его расширение
        string fileName = filePath.filename().string();
        string extension = filePath.extension().string();

        // Ищем категорию, соответствующую расширению файла
        auto it = find_if(categories.begin(), categories.end(),
                          [&](const Category &c)
                          {
                              return find(c.extensions.begin(),
                                          c.extensions.end(),
                                          extension) != c.extensions.end();
                          });

        // Если категория не найдена, используем категорию по умолчанию
        Category *category = it != categories.end() ? &*it : nullptr;

        if (category == nullptr)
        {
            category = &categories.back();
        }

        // Строим путь к новому месту файла
        fs::path destPath = filePath.parent_path() / category->name;

        // Исправляем слеши в пути (если необходимо)
        destPath = destPath.make_preferred();

        // Если папка не существует, создаем её
        if (!fs::exists(destPath))
        {
            fs::create_directory(destPath);
        }

        // Проверяем, не существует ли уже файл с таким именем в папке
        for (int i = 1; fs::exists(destPath / fileName); ++i)
        {
            fileName = filePath.stem().string() + "_" + to_string(i) + extension;
        }

        // Строим окончательный путь для файла
        fs::path finalPath = destPath / fileName;
        finalPath = finalPath.make_preferred();

        // Переименовываем файл
        fs::rename(filePath, finalPath);

        // Выводим информацию о перемещении файла
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
        cout << "Move: " << fileName << " to " << category->name << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
    catch (const exception &e)
    {
        // Выводим сообщение об ошибке при возникновении исключения
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while moving file: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
        // Можно добавить дополнительные действия, такие как протоколирование или вывод пользователю
    }
}

// Функция для рекурсивного удаления пустых папок в заданной директории
void removeEmptyFolders(const fs::path &folderPath)
{
    try
    {
        // Проходим по всем элементам в текущей директории
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            // Если текущий элемент является директорией
            if (entry.is_directory())
            {
                // Рекурсивно вызываем removeEmptyFolders для вложенной директории
                removeEmptyFolders(entry.path());
            }
        }

        // Проверяем, является ли текущая директория пустой
        if (fs::is_empty(folderPath))
        {
            // Получаем имя текущей категории из имени директории
            auto categoryName = folderPath.filename().string();

            // Поиск соответствующей категории по имени
            auto it = find_if(categories.begin(), categories.end(), [&](const Category &cat) {
                return cat.name == categoryName;
            });

            // Удаляем текущую директорию и выводим сообщение, если это не категориальная директория
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
        // Выводим сообщение об ошибке при возникновении исключения
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while removing empty folders: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
}


// Функция для сортировки файлов в папке
void sortFiles(const fs::path &folderPath)
{
    try
    {
        // Создаем категориальные папки
        createCategoryFolders(folderPath);

        // Перемещаем файлы
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                moveFileToCategory(entry.path());
            }
        }

        // Удаляем все пустые папки, кроме категориальных
        removeEmptyFolders(folderPath);
    }
    catch (const exception &e)
    {
        // Выводим сообщение об ошибке при возникновении исключения
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        cerr << "Error while sorting files: " << e.what() << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
    }
}

fs::path getFolderPathFromExplorer()
{
    // Открытие диалогового окна проводника для выбора папки
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IFileOpenDialog *pFileOpen;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        pFileOpen->GetOptions(&dwOptions);
        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

        // Отображение диалогового окна
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
    return fs::path(); // Возвращаем пустой путь, если выбор папки отменен или произошла ошибка
}


int main()
{
    system("chcp 1251");

    while (true)
    {
        system("cls");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_YELLOW);
        std::cout << "\n------- Начало работы -------\n";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);


        std::cout << "Введите путь к папке. Нажмите enter для выбора папки Download. \n";
        std::string mainPath = "";

        while (mainPath.empty())
        {
            std::cout << "Путь к папке: ";
            std::getline(std::cin, mainPath);

            
            if (mainPath.empty())
            {
                CoInitialize(nullptr);

                CHAR linkPathDef[MAX_PATH];
                HRESULT result = SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, linkPathDef);

                if (result != S_OK)
                    std::cout << "Error: " << result << "\n";
                else
                    strcat_s(linkPathDef, "\\Downloads"); // Добавим к пути "Downloads"

                mainPath = linkPathDef;
                std::cout << "Выбрана папка Download!\n";

                CoUninitialize();
            }
            else
            {
                // Проверка наличия кавычек в начале и конце пути
                if (!mainPath.empty() && (mainPath.front() == '\"' || mainPath.front() == '\'') &&
                    (mainPath.back() == '\"' || mainPath.back() == '\''))
                {
                    // Убираем кавычки из начала и конца пути
                    mainPath = mainPath.substr(1, mainPath.length() - 2);
                }
            }

            cout << mainPath << endl;
            if (!fs::path(mainPath).is_absolute())
            {
                std::cout << "Введенный путь не является действительным. Попробуйте снова.\n";
                mainPath = "";
                continue;
            }

            // Преобразуем строку в объект std::filesystem::path
            fs::path fsMainPath = mainPath;

            // Преобразовываем путь к предпочтительному формату
            fsMainPath.make_preferred();

            // Добавлена проверка на существование папки
            if (!mainPath.empty() && !fs::exists(fsMainPath))
            {
                cerr << "Error: Folder does not exist." << endl;
                return 1;
            }

            // Вызываем функцию сортировки, передавая std::filesystem::path
            sortFiles(fsMainPath);

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_YELLOW);
            std::cout << "------- Конец работы -------\n";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
        }

        std::cout << "\nЖелаете продолжить? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice != "y" && choice != "н")
        {
            system("cls");
            break;
        }
    }

    return 0;
}