#include <iostream>
#include <dirent.h>
#include <iostream>
#include <filesystem>

int get_roads_number() {
    const char* folder_path = "./data/";
    int count = 0;
    DIR* dirp = opendir(folder_path);

    if (dirp == NULL) {
        std::cerr << "Error opening directory " << folder_path << std::endl;
        return 1;
    }

    struct dirent* entry;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_DIR) {
            count++;
        }
    }

    closedir(dirp);

    return count-2; //2 because of . and ..
}

int main() {
    int count;
    count = get_roads_number();
    std::cout << count;
}