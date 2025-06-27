#include "LibraryUtil.h"

#ifdef WIN32
#include <locale>
#include <codecvt>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>
#define IMAGEHLP_DEFERRED_LOADS
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

DWORD RvaToOffset(DWORD rva, PIMAGE_NT_HEADERS ntHeaders, PIMAGE_SECTION_HEADER sections)
{
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
    {
        DWORD sectionVA = sections[i].VirtualAddress;
        DWORD sectionSize = sections[i].SizeOfRawData;

        if (rva >= sectionVA && rva < sectionVA + sectionSize)
        {
            DWORD delta = rva - sectionVA;
            return sections[i].PointerToRawData + delta;
        }
    }
    return 0; // Invalid RVA
}

std::vector<std::string> getImportedDlls(const std::wstring& dllPath)
{
    std::vector<std::string> importedDlls;

    HANDLE hFile = CreateFileW(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Failed to open DLL file");
    }

    HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping)
    {
        CloseHandle(hFile);
        throw std::runtime_error("Failed to create file mapping");
    }

    LPVOID lpBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!lpBase)
    {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        throw std::runtime_error("Failed to map view of file");
    }

    try
    {
        auto* dos = (PIMAGE_DOS_HEADER)lpBase;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) throw std::runtime_error("Invalid DOS header");

        auto* nt = (PIMAGE_NT_HEADERS)((BYTE*)lpBase + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) throw std::runtime_error("Invalid NT header");

        auto* sections = IMAGE_FIRST_SECTION(nt);

        DWORD importRva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (!importRva) return importedDlls;

        DWORD importOffset = RvaToOffset(importRva, nt, sections);
        if (!importOffset) throw std::runtime_error("Invalid import directory offset");

        auto* desc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)lpBase + importOffset);

        constexpr int maxImports = 512;
        int count = 0;

        while (desc->Name && count++ < maxImports)
        {
            DWORD nameOffset = RvaToOffset(desc->Name, nt, sections);
            if (!nameOffset) break;

            const char* name = (const char*)((BYTE*)lpBase + nameOffset);
            importedDlls.emplace_back(name);

            ++desc;
        }
    }
    catch (...)
    {
        UnmapViewOfFile(lpBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        throw;
    }

    UnmapViewOfFile(lpBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    return importedDlls;
}

static std::string wstringToString(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

std::wstring stringToWString(const std::string& str)
{
    if (str.empty()) return {};

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstr(sizeNeeded, 0);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], sizeNeeded);
    return wstr;
}

std::vector<std::string> getMissingDependencies(const std::string& dllPath)
{
    std::vector<std::string> dependencies = getImportedDlls(stringToWString(dllPath));
    std::vector<std::string> missingDependencies;
    for (const auto& dep : dependencies)
    {
        HMODULE depHandle = LoadLibrary(dep.c_str());
        if (!depHandle)
        {
            missingDependencies.push_back(dep);
        }
        else
        {
            FreeLibrary(depHandle);
        }
    }

    return missingDependencies;
}

#else
std::vector<std::string> getMissingDependencies(const std::string& dllPath)
{
    // TODO: implement for operating system
    return {};
}
#endif