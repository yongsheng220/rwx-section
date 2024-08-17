#include <iostream>
#include <filesystem>
#include <fstream>
#include <Windows.h>

using namespace std;
namespace fs = std::filesystem;

bool hasRWXSection(const string& filePath) {
    ifstream peFile(filePath, ios::binary);
    if (!peFile) {return false;}
    IMAGE_DOS_HEADER dosHeader;
    peFile.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {return false;}
    peFile.seekg(dosHeader.e_lfanew, ios::beg);
    IMAGE_NT_HEADERS ntHeaders;
    peFile.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {return false;}

    for (int i = 0; i < ntHeaders.FileHeader.NumberOfSections; ++i) {
        IMAGE_SECTION_HEADER sectionHeader;
        peFile.read(reinterpret_cast<char*>(&sectionHeader), sizeof(sectionHeader));

        // 检查是否有 RWX 权限
        if ((sectionHeader.Characteristics & IMAGE_SCN_MEM_READ) &&
            (sectionHeader.Characteristics & IMAGE_SCN_MEM_WRITE) &&
            (sectionHeader.Characteristics & IMAGE_SCN_MEM_EXECUTE)) {
            return true;
        }
    }
    return false;
}


int main(int argc, char* argv[]) {
    string path;
    if (argc > 1) {
        path = argv[1];
        cout << "[*]Current Path: " << path << endl;
    }
    else {
        cerr << "[-]参数错误：rwx-section.exe C:/Windows/" << endl;
        return 1;
    }

    fs::path dirPath(path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                string extension = entry.path().extension().string();
                string filename = entry.path().string();
                // 检查扩展名是否为 .exe 或 .dll
                if (extension == ".exe" || extension == ".dll") {
                    // 检查 section 权限
                    if (!hasRWXSection(filename)) {  continue; }
                    cout << "[+]" << filename << endl;
                }
            }
        }
    }
    else {
        cerr << "[-]输入的路径不是一个有效的目录" << endl;
    }

    return 0;
}
