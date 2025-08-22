#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include "MFT.h"
#include <chrono>
#include <ctime>

class Command {
public:
    virtual ~Command() {}
    virtual void execute() = 0;
};

class WaitForEnterCommand : public Command {
public:
    void execute() override {
        std::cout << "\nPress Enter to return to menu...";
        std::cin.ignore();
    }
};

class AddCommand : public Command {
public:
    AddCommand(MFT& mft, const std::string& path)
        : mft(mft), path(path) {}
    void execute() override {
        namespace fs = std::filesystem;
        if (path.empty()) {
            std::cout << "Usage: add [absolute_path]\n";
            return;
        }
        if (!fs::exists(path)) {
            std::cout << "Directory does not exist.\n";
            return;
        }
        if (!fs::is_directory(path)) {
            std::cout << "Path is not a directory.\n";
            return;
        }
        mft.addDirectory(path);
        std::cout << "Directory added.\n";
    }
private:
    MFT& mft;
    std::string path;
};

class ShowAllCommand : public Command {
public:
    ShowAllCommand(const MFT& mft) : mft(mft) {}
    void execute() override {
        const auto& entries = mft.getEntries();
        if (entries.empty()) {
            std::cout << "No entries available.\n";
            return;
        }
        for (const auto& entry : entries) {
            std::cout << "Filename: " << entry.filename << "\n";
            // Add more fields as needed
        }
    }
private:
    const MFT& mft;
};

class HelpCommand : public Command {
public:
    void execute() override {
        std::cout << "Help:\n";
        std::cout << "  add [path] - Add directory by absolute path\n";
        std::cout << "  a         - Show all entry details\n";
        std::cout << "  h         - Help\n";
        std::cout << "  e         - Exit\n";
        std::cout << "  [number]  - View entry details\n";
        std::cout << "  s         - Search by filename\n";
    }
};

class PrintMetadataCommand : public Command {
public:
    PrintMetadataCommand(const Entry& entry) : entry(entry) {}
    void execute() override {
        std::cout << "\nFilename: " << entry.filename << "\n";
        std::cout << "Owner: " << entry.owner << "\n";
        std::cout << "System: " << (entry.isSystem ? "Yes" : "No") << "\n";
        std::cout << "Hidden: " << (entry.isHidden ? "Yes" : "No") << "\n";
        std::cout << "ReadOnly: " << (entry.isReadOnly ? "Yes" : "No") << "\n";
        std::cout << "Folder: " << (entry.isFolder ? "Yes" : "No") << "\n";
        std::cout << "Physical Size: " << entry.physicalSize << "\n";
        std::cout << "Logical Size: " << entry.logicalSize << "\n";
        char buf[26];
        time_t created = std::chrono::system_clock::to_time_t(entry.createdDate);
        if (ctime_s(buf, sizeof(buf), &created) == 0)
            std::cout << "Created Date: " << buf;
        time_t modified = std::chrono::system_clock::to_time_t(entry.modifiedDate);
        if (ctime_s(buf, sizeof(buf), &modified) == 0)
            std::cout << "Modified Date: " << buf;
        time_t accessed = std::chrono::system_clock::to_time_t(entry.accessedDate);
        if (ctime_s(buf, sizeof(buf), &accessed) == 0)
            std::cout << "Accessed Date: " << buf;
        time_t lastWritten = std::chrono::system_clock::to_time_t(entry.lastWrittenDate);
        if (ctime_s(buf, sizeof(buf), &lastWritten) == 0)
            std::cout << "Last Written Date: " << buf;
    }
private:
    Entry entry;
};

class PrintEntriesListCommand : public Command {
public:
    PrintEntriesListCommand(const MFT& mft) : mft(mft) {}
    void execute() override {
        const auto& entries = mft.getEntries();
        std::cout << "\n==== MFT List ====\n";
        for (size_t i = 0; i < entries.size(); ++i) {
            std::cout << std::setw(3) << i + 1 << ". " << entries[i].filename << std::endl;
        }
    }
private:
    const MFT& mft;
};

class SearchEntryCommand : public Command {
public:
    SearchEntryCommand(const MFT& mft) : mft(mft) {}
    void execute() override {
        std::cout << "Enter filename to search (wildcards * and ? supported): ";
        std::string pattern;
        std::getline(std::cin, pattern);
        auto results = mft.findEntriesWildcard(pattern);
        if (!results.empty()) {
            for (const Entry* entry : results) {
                PrintMetadataCommand(*entry).execute();
                std::cout << "-----------------------------\n";
            }
        } else {
            std::cout << "No entries found matching pattern.\n";
        }
        WaitForEnterCommand().execute();
    }
private:
    const MFT& mft;
};