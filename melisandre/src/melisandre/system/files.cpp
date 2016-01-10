#include "files.hpp"

#include <fstream>

#ifdef __GNUC__

// Use Posix API
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#else

#ifdef _WIN32

#include <windows.h>
#include <windef.h>
#include <shlwapi.h>

#endif

#endif

namespace mls {

// Copy a given file
void copyFile(const FilePath& srcFilePath, const FilePath& dstFilePath) {
    std::ifstream src(srcFilePath.c_str(), std::ios::binary);
    std::ofstream dst(dstFilePath.c_str(), std::ios::binary);

    if (!src.is_open()) {
        // LOG
        return;
    }

    dst << src.rdbuf();
}


#ifdef __GNUC__

class Directory {
public:
    Directory(const FilePath& path) :
        m_Path(path), m_pDir(opendir(m_Path.c_str())) {
    }

    ~Directory() {
        if (m_pDir) {
            closedir(m_pDir);
        }
    }

    operator bool() const {
        return m_pDir;
    }

    const FilePath& path() const {
        return m_Path;
    }

    std::vector<FilePath> files() const {
        std::vector<FilePath> container;

        rewinddir(m_pDir);
        struct dirent* entry = nullptr;
        while (nullptr != (entry = readdir(m_pDir))) {
            FilePath file(entry->d_name);
            if (file != ".." && file != ".") {
                container.emplace_back(file);
            }
        }
        rewinddir(m_pDir);

        return container;
    }

private:
    FilePath m_Path;
    DIR* m_pDir;
};

inline bool exists(const FilePath& path) {
    struct stat s;
    return 0 == stat(path.c_str(), &s);
}

inline bool isDirectory(const FilePath& path) {
    struct stat s;
    return 0 == stat(path.c_str(), &s) && S_ISDIR(s.st_mode);
}

inline bool isRegularFile(const FilePath& path) {
    struct stat s;
    return 0 == stat(path.c_str(), &s) && S_ISREG(s.st_mode);
}

inline void createDirectory(const FilePath& path) {
    mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IWOTH | S_IXOTH);
}

void foreachFile(const FilePath& directoryPath, const std::function<bool(const FilePath&)>& functor, bool relativePath) {

}

std::vector<FilePath> getContainedFiles(const FilePath& directoryPath, bool relativePath) {

}

#else

#ifdef _WIN32

bool exists(const FilePath& path) {
    return PathFileExists(path.c_str()) ? true : false;
}

bool isDirectory(const FilePath& path) {
    return PathIsDirectory(path.c_str()) ? true : false;
}

bool isRegularFile(const FilePath& path) {
    return exists(path) && !isDirectory(path);
}

void createDirectory(const FilePath& path) {
    CreateDirectory(path.c_str(), 0);
}

void foreachFile(const FilePath& directoryPath, const std::function<void(const FilePath&)>& functor, bool relativePath) {
    HANDLE handle{ CreateFile(directoryPath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0) };

    if (handle == INVALID_HANDLE_VALUE) {
        // LOG
        return;
    }

    std::string path = directoryPath.str() + "\\*";
    
    WIN32_FIND_DATA ffd;
    auto hFind = FindFirstFile(path.c_str(), &ffd);
    
    if (INVALID_HANDLE_VALUE == hFind) {
        // LOG
        return;
    }
    
    do {
        FilePath file(ffd.cFileName);
        if (file != ".." && file != ".") {
            functor(relativePath ? file : directoryPath + file);
        }
    } while (FindNextFile(hFind, &ffd) != 0);
    
    FindClose(hFind);

    CloseHandle(handle);
}

std::vector<FilePath> getContainedFiles(const FilePath& directoryPath, bool relativePath) {
    std::vector<FilePath> files;
    foreachFile(directoryPath, [&](const FilePath& path) {
        files.emplace_back(path);
    }, relativePath);
    return files;
}

#endif

#endif

}