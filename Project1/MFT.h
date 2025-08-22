#pragma once
#include "Entry.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <regex>
#include <filesystem>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#include <Aclapi.h>
#endif

class MFT {
public:
    // Add a new entry
    void addEntry(const Entry& entry) {
        entries.push_back(entry);
    }

    // Search for an entry by filename
    const Entry* findEntry(const std::string& filename) const {
        for (const auto& entry : entries) {
            if (entry.filename == filename) {
                return &entry;
            }
        }
        return nullptr;
    }

    // Print all entries sorted by filename
    void printAllSorted() const {
        std::vector<Entry> sortedEntries = entries;
        std::sort(sortedEntries.begin(), sortedEntries.end(),
            [](const Entry& a, const Entry& b) {
                return a.filename < b.filename;
            });
        for (const auto& entry : sortedEntries) {
            std::cout << "Filename: " << entry.filename << "\n";
        }
    }

    // Search for entries by filename with wildcard support
    std::vector<const Entry*> findEntriesWildcard(const std::string& pattern) const {
        // Convert wildcard pattern to regex
		std::string regexPattern = "^"; // Start of the regex pattern
		for (char c : pattern) { // Convert each character
			if (c == '*') regexPattern += ".*"; // '*' matches any sequence of characters
			else if (c == '?') regexPattern += "."; // '?' matches any single character
			else if (c == '.') regexPattern += "\\."; // '.' is a special character in regex, escape it
			else regexPattern += c; // Any other character is added as is
        }
		regexPattern += "$"; // End of the regex pattern
		std::regex re(regexPattern, std::regex::icase); // Case-insensitive regex
		std::vector<const Entry*> results; // Vector to hold matching entries
		for (const auto& entry : entries) { //find everything that matches
            if (std::regex_match(entry.filename, re)) {
                results.push_back(&entry);
            }
        }
        return results;
    }

    // Add all entries from a directory (absolute path)
    void addDirectory(const std::string& dirPath) {
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            Entry e;
            e.filename = entry.path().filename().string();
            e.isFolder = entry.is_directory();
            e.isSystem = false;
            e.isHidden = false;
            e.isReadOnly = false;
            e.owner = "";
            e.physicalSize = entry.is_regular_file() ? fs::file_size(entry.path()) : 0;
            e.logicalSize = e.physicalSize;
            auto fstatus = entry.status();
#ifdef _WIN32 // Windows-specific code to get file attributes and owner
            DWORD attrs = GetFileAttributesA(entry.path().string().c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES) {
                e.isSystem = (attrs & FILE_ATTRIBUTE_SYSTEM);
                e.isHidden = (attrs & FILE_ATTRIBUTE_HIDDEN);
                e.isReadOnly = (attrs & FILE_ATTRIBUTE_READONLY);
            }
            // Get owner
            PSECURITY_DESCRIPTOR sd = NULL;
            if (GetFileSecurityA(entry.path().string().c_str(), OWNER_SECURITY_INFORMATION, NULL, 0, &attrs)) {
                DWORD sdSize = 0;
                GetFileSecurityA(entry.path().string().c_str(), OWNER_SECURITY_INFORMATION, NULL, 0, &sdSize);
                sd = (PSECURITY_DESCRIPTOR)malloc(sdSize);
                if (GetFileSecurityA(entry.path().string().c_str(), OWNER_SECURITY_INFORMATION, sd, sdSize, &sdSize)) {
                    PSID ownerSid = NULL;
                    BOOL ownerDefaulted = FALSE;
                    if (GetSecurityDescriptorOwner(sd, &ownerSid, &ownerDefaulted)) {
                        char name[256], domain[256];
                        DWORD nameLen = 256, domainLen = 256;
                        SID_NAME_USE snu;
                        if (LookupAccountSidA(NULL, ownerSid, name, &nameLen, domain, &domainLen, &snu)) {
                            e.owner = name;
                        }
                    }
                }
                free(sd);
            }
            #endif
            // Dates
            std::error_code ec;
            auto ftime = fs::last_write_time(entry.path(), ec);
            if (!ec) {
                auto sctp = std::chrono::system_clock::time_point(
                    std::chrono::duration_cast<std::chrono::system_clock::duration>(
                        ftime.time_since_epoch()
                    )
                );
                e.lastWrittenDate = sctp;
                e.createdDate = sctp;
                e.modifiedDate = sctp;
                e.accessedDate = sctp;
            }
            addEntry(e);
        }
    }

    // Get all entries (const access)
    const std::vector<Entry>& getEntries() const {
        return entries;
    }

private:
    std::vector<Entry> entries;
};