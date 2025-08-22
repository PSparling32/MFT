#pragma once
#include <string>
#include <chrono>

struct Entry {
    std::string filename;
    std::string owner;
    bool isSystem;
    bool isHidden;
    bool isReadOnly;
    bool isFolder;
    std::chrono::system_clock::time_point createdDate;
    std::chrono::system_clock::time_point modifiedDate;
    std::chrono::system_clock::time_point accessedDate;
    std::chrono::system_clock::time_point lastWrittenDate;
    uintmax_t physicalSize; // Actual size on disk
    uintmax_t logicalSize;  // Reported file size
};