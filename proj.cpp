#include <iostream>
#include <filesystem>
#include <map>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;
using namespace std;

// Function to compare two files byte by byte
bool areFilesEqual(const string& file1, const string& file2) {
    ifstream f1(file1, ios::binary);
    ifstream f2(file2, ios::binary);

    if (!f1 || !f2) return false;

    char c1, c2;
    while (f1.get(c1) && f2.get(c2)) {
        if (c1 != c2)
            return false;
    }

    return true;
}

int main() {
    string path;
    cout << "Enter folder path: ";
    cin >> path;

    map<long long, vector<string>> fileMap;

    // Step 1: Group files by size
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry)) {
            long long size = fs::file_size(entry);
            fileMap[size].push_back(entry.path().string());
        }
    }

    // Step 2: Compare files with same size
    bool found = false;

    for (auto& pair : fileMap) {
        vector<string>& files = pair.second;

        if (files.size() > 1) {
            for (int i = 0; i < files.size(); i++) {
                for (int j = i + 1; j < files.size(); j++) {

                    if (areFilesEqual(files[i], files[j])) {
                        if (!found) {
                            cout << "\nDuplicate files found:\n";
                            found = true;
                        }

                        cout << files[i] << endl;
                        cout << files[j] << endl;
                        cout << "----------------------\n";
                    }
                }
            }
        }
    }

    if (!found) {
        cout << "\nNo duplicate files found.\n";
    }

    return 0;
}
