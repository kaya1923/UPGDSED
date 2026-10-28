#define UNICODE
#define _UNICODE
#include <windows.h>
#include <dia2.h>
#include <cstdio>
#include <cwchar>

#pragma comment(lib, "diaguids.lib")

template <typename T>
static void Release(T*& value)
{
    if (value) {
        value->Release();
        value = nullptr;
    }
}

static bool PrintMemberOffset(IDiaSymbol* global, const wchar_t* typeName,
                              const wchar_t* memberName, LONG expected)
{
    IDiaEnumSymbols* types = nullptr;
    IDiaSymbol* type = nullptr;
    IDiaEnumSymbols* members = nullptr;
    IDiaSymbol* member = nullptr;
    ULONG fetched = 0;
    LONG offset = -1;

    HRESULT hr = global->findChildren(SymTagUDT, typeName, nsCaseSensitive, &types);
    if (FAILED(hr) || types->Next(1, &type, &fetched) != S_OK) {
        std::wprintf(L"ERROR: type %ls not found\n", typeName);
        Release(types);
        return false;
    }

    hr = type->findChildren(SymTagData, memberName, nsCaseSensitive, &members);
    if (FAILED(hr) || members->Next(1, &member, &fetched) != S_OK ||
        FAILED(member->get_offset(&offset))) {
        std::wprintf(L"ERROR: member %ls.%ls not found\n", typeName, memberName);
        Release(member);
        Release(members);
        Release(type);
        Release(types);
        return false;
    }

    const bool match = offset == expected;
    std::wprintf(L"%ls.%ls = 0x%lX (expected 0x%lX) %ls\n",
                 typeName, memberName, offset, expected, match ? L"OK" : L"MISMATCH");

    Release(member);
    Release(members);
    Release(type);
    Release(types);
    return match;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 3) {
        std::fwprintf(stderr, L"usage: verify_offsets_dia.exe <ntkrnlmp.pdb> <msdia140.dll>\n");
        return 2;
    }

    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        std::fwprintf(stderr, L"CoInitializeEx failed\n");
        return 2;
    }

    IDiaDataSource* source = nullptr;
    IDiaSession* session = nullptr;
    IDiaSymbol* global = nullptr;
    HMODULE diaModule = LoadLibraryW(argv[2]);
    using DllGetClassObjectFn = HRESULT(STDAPICALLTYPE*)(REFCLSID, REFIID, LPVOID*);
    auto getClassObject = diaModule ? reinterpret_cast<DllGetClassObjectFn>(
        GetProcAddress(diaModule, "DllGetClassObject")) : nullptr;
    IClassFactory* factory = nullptr;
    HRESULT hr = getClassObject ? getClassObject(__uuidof(DiaSource), IID_IClassFactory,
                                                 reinterpret_cast<void**>(&factory))
                                : HRESULT_FROM_WIN32(GetLastError());
    if (SUCCEEDED(hr))
        hr = factory->CreateInstance(nullptr, __uuidof(IDiaDataSource),
                                     reinterpret_cast<void**>(&source));
    Release(factory);
    if (SUCCEEDED(hr))
        hr = source->loadDataFromPdb(argv[1]);
    if (SUCCEEDED(hr))
        hr = source->openSession(&session);
    if (SUCCEEDED(hr))
        hr = session->get_globalScope(&global);
    if (FAILED(hr)) {
        std::fwprintf(stderr, L"DIA could not load the PDB (HRESULT 0x%08lX)\n", hr);
        Release(global);
        Release(session);
        Release(source);
        if (diaModule)
            FreeLibrary(diaModule);
        CoUninitialize();
        return 2;
    }

    bool ok = true;
    ok &= PrintMemberOffset(global, L"_KTHREAD", L"MiscFlags", 0x74);
    ok &= PrintMemberOffset(global, L"_ETHREAD", L"CrossThreadFlags", 0x510);
    ok &= PrintMemberOffset(global, L"_ETHREAD", L"PicoContext", 0x5E0);
    ok &= PrintMemberOffset(global, L"_EPROCESS", L"Flags2", 0x460);
    ok &= PrintMemberOffset(global, L"_EPROCESS", L"SeAuditProcessCreationInfo", 0x5C0);

    Release(global);
    Release(session);
    Release(source);
    FreeLibrary(diaModule);
    CoUninitialize();
    return ok ? 0 : 1;
}
