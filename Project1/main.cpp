#include "MFT.h"
#include "Menu.h"
#include <chrono>

//minimal cause its just an entry point
int main() {
    MFT mft;
    // Start with an empty MFT
    Menu menu(mft);
    menu.run(); //our core loop
    return 0;
}
