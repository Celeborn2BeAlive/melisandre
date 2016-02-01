#pragma once

#include <functional>
#include <string>
#include <ostream>
#include <vector>

namespace mls {

// A class to represent a path in the filesystem
class FilePath {
public:
#ifdef _WIN32
    static const char PATH_SEPARATOR = '\\';
#else
    static const char PATH_SEPARATOR = '/';
#endif
    FilePath() = default;

    explicit FilePath(const char* filepath) : m_FilePath(filepath) {
        format();
    }

    explicit FilePath(const std::string& filepath) : m_FilePath(filepath) {
        format();
    }

    const std::string& str() const { return m_FilePath; }

    const char* c_str() const { return m_FilePath.c_str(); }

    bool empty() const {
        return m_FilePath.empty();
    }

    /*! returns the containing directory path of a file */
    FilePath directory() const {
        size_t pos = m_FilePath.find_last_of(PATH_SEPARATOR);
        if (pos == std::string::npos) { return FilePath(); }
        return FilePath{ m_FilePath.substr(0, pos) };
    }

    /*! returns the file name of a filepath  */
    std::string filename() const {
        size_t pos = m_FilePath.find_last_of(PATH_SEPARATOR);
        if (pos == std::string::npos) { return m_FilePath; }
        return m_FilePath.substr(pos + 1);
    }

    /*! returns the file extension */
    std::string ext() const {
        size_t pos = m_FilePath.find_last_of('.');
        if (pos == std::string::npos || pos == 0) { return ""; }
        return m_FilePath.substr(pos + 1);
    }

    // Returns true if the file path ends with ext
    bool endsWith(const std::string& ext) const {
        return m_FilePath.size() >= ext.size() && m_FilePath.substr(m_FilePath.size() - ext.size(), ext.size()) == ext;
    }

    /*! adds file extension */
    FilePath addExt(const std::string& ext) const {
        return FilePath(m_FilePath + "." + ext);
    }

    // Concatenate other to this file path
    FilePath& operator +=(const FilePath& other) {
        if (m_FilePath.empty()) {
            *this = other;
        } else if (!other.empty()) {
            if (other.m_FilePath.front() != PATH_SEPARATOR) {
                m_FilePath += PATH_SEPARATOR;
            }
            m_FilePath += other.m_FilePath;
        }
        return *this;
    }

    bool operator ==(const FilePath& other) const {
        return other.m_FilePath == m_FilePath;
    }

    bool operator ==(const std::string& other) const {
        return other == m_FilePath;
    }

    bool operator ==(const char* other) const {
        return FilePath { other } == m_FilePath;
    }

    template<typename T>
    friend bool operator !=(const FilePath& lhs, T&& rhs) {
        return !(lhs == std::forward<T>(rhs));
    }

    template<typename T>
    friend bool operator !=(T&& lhs, const FilePath& rhs) {
        return rhs != lhs;
    }

    /*! output operator */
    friend std::ostream& operator<<(std::ostream& cout, const FilePath& filepath) {
        return (cout << filepath.m_FilePath);
    }

private:
    void format() {
        for (auto& c : m_FilePath) {
            if (c == '\\' || c == '/') {
                c = PATH_SEPARATOR;
            }
        }
        while (!m_FilePath.empty() && m_FilePath.back() == PATH_SEPARATOR) {
            m_FilePath.pop_back();
        }
    }

    std::string m_FilePath;
};

inline FilePath operator +(FilePath lhs, const FilePath& rhs) {
    return (lhs += rhs);
}

// Copy a given file
void copyFile(const FilePath& srcFilePath, const FilePath& dstFilePath);

// Return true if a file pointed by path exists
bool exists(const FilePath& path);

// Return true if the file pointed by path is a directory
bool isDirectory(const FilePath& path);

// Return true if the file pointed by path is regular
bool isRegularFile(const FilePath& path);

// Create a directory. If it already exists, nothing is done.
void createDirectory(const FilePath& path);

// Iterate over the files contained in the directory pointed by directoryPath, calling functor for each one
// The path given to functor is relative to directoryPath if relativePath = true. Otherwise, the path given to functor is directoryPath + path of file
// If functor returns false, the function returns
void foreachFile(const FilePath& directoryPath, const std::function<void(const FilePath&)>& functor, bool relativePath = true);

std::vector<FilePath> getContainedFiles(const FilePath& directoryPath, bool relativePath = true);

}