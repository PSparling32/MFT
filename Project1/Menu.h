#pragma once
#include "MFT.h"
#include "Command.h"
#include <iostream>
#include <limits>
#include <string>
#include <iomanip>
#include <filesystem>

class Menu {
public:
    Menu(const MFT& mft) : mft(mft) {}

    //core loop
    void run() {
		while (true) { //run untill delibrately exited
            //print the text stuff
            printEntriesList();

            //prompt
            std::cout << "Select option(Type h for command): ";

            //get input
            std::string input;
            std::getline(std::cin, input);

			// our selected command
            Command* cmd = nullptr;

			//should we exit?
            if (input == "e" || input == "E") break;

			// Check for commands

			//help command
            if (input == "h" || input == "H") {
                cmd = new HelpCommand();
            }
            //search
            else if (input == "s" || input == "S") {
                searchEntry();
                continue;
            } 
			//show all entries
            else if (input == "a" || input == "A") {
                cmd = new ShowAllCommand(mft);
            }
			//prit sorted entries
            else if (input == "o" || input == "O") {
                // Print all entries sorted by filename
                mft.printAllSorted();
                waitForEnter();
                continue;
            } 
			// add directory command
            else if (input.substr(0, 3) == "add") {
                // Parse for path only
				std::string rest = input.substr(3); //first 3 chars are "add"
                // Remove leading spaces
				size_t pos = rest.find_first_not_of(' '); //remove the leading spaces
				if (pos != std::string::npos) rest = rest.substr(pos); // remove leading spaces
				if (rest.empty()) { //if it's empty
					std::cout << "Usage: add [absolute_path]\n"; // print usage
                    continue;
                }
				cmd = new AddCommand(const_cast<MFT&>(mft), rest); // create command to add directory and pass the path
            } 
            // other
            else {
				// Try to parse input as an entry index
                try {
					//size_t is unsigned int, we wont have negative indices
					size_t index = std::stoul(input); //stoul converts string to unsigned long (string to unsigned long)
					// Check if index is within valid range
                    const auto& entries = mft.getEntries();

					// If the index is valid, print the metadata for that entry
                    if (index >= 1 && index <= entries.size()) {
                        printMetadata(entries[index - 1]);
                        waitForEnter();
                    } else {
                        std::cout << "Invalid selection. Please enter a valid entry number.\n";
                    }
                } catch (...) {
                    std::cout << "Invalid input. Type 'h' for help.\n";
                }
                continue;
            }
            if (cmd) {
                cmd->execute();
                delete cmd;
                waitForEnter();
            }
        }
    }

private:
    const MFT& mft;

    void printEntriesList() const {
        const auto& entries = mft.getEntries();
        std::cout << "\n==== MFT List ====" << std::endl;
        for (size_t i = 0; i < entries.size(); ++i) {
            std::cout << std::setw(3) << i + 1 << ". " << entries[i].filename << std::endl;
        }
    }

    void printOptions() const {
        std::cout << "\nOptions:" << std::endl;
        std::cout << "  [number] - View entry details" << std::endl;
        std::cout << "  s        - Search by filename" << std::endl;
        std::cout << "  a        - Show all entry details" << std::endl;
        std::cout << "  o        - Show all entry details sorted by filename" << std::endl;
        std::cout << "  h        - Help" << std::endl;
        std::cout << "  e        - Exit" << std::endl;
        std::cout << "  add      - Add directory by absolute path" << std::endl;
    }

    void searchEntry() const {
        std::cout << "Enter filename to search (wildcards * and ? supported): ";
        std::string pattern;
        std::getline(std::cin, pattern);
        auto results = mft.findEntriesWildcard(pattern);
        if (!results.empty()) {
            for (const Entry* entry : results) {
                printMetadata(*entry);
                std::cout << "-----------------------------\n";
            }
        } else {
            std::cout << "No entries found matching pattern.\n";
        }
        waitForEnter();
    }

    void printMetadata(const Entry& entry) const {
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

    void waitForEnter() const {
        std::cout << "\nPress Enter to return to menu...";
        std::cin.ignore();
    }
};
