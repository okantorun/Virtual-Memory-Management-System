#include <cstdlib>
#include <iostream>

class VirtualMemorySystem {
    private:
        const int FRAME_SIZE;
        const int VIRTUAL_FRAME;
        const int VIRTUAL_MEMORY_SIZE;

        struct VirtualMemoryPageEntry {
            long long int* addressSpace;
            int virtualPageNumber;
            int pageOffset;
        };


        VirtualMemoryPageEntry* virtualMemory;

    public:
        VirtualMemorySystem(int frameSize, int virtualFrame, int virtualMemorySize);            
        void initializeVM();
        int getValueByIndex(int index);
        int getVirtualPageNumberByIndex(int index);
        void setAddressSpace(int index, int frameIndex, int value);
        int getFrameSize();
        int getVirtualMemorySize();
        int getVirtualFrameCount();
        void printVirtualMemory();

};

VirtualMemorySystem::VirtualMemorySystem(int frameSize, int virtualFrame, int virtualMemorySize)
    : FRAME_SIZE(frameSize), VIRTUAL_FRAME(virtualFrame),
    VIRTUAL_MEMORY_SIZE(virtualMemorySize) {
    
    virtualMemory = new VirtualMemoryPageEntry[VIRTUAL_FRAME];
    for (int i = 0; i < VIRTUAL_MEMORY_SIZE; ++i) {
        virtualMemory[i].addressSpace = new long long int[FRAME_SIZE];
    }
}
void VirtualMemorySystem::initializeVM() {
    int loadedValue=0;
    for (int i = 0; i < VIRTUAL_FRAME; ++i) {
        for (int j = 0; j < FRAME_SIZE; ++j) {
            virtualMemory[i].addressSpace[j] = loadedValue++ ; //(rand() % 100) + 1;
        }
        virtualMemory[i].virtualPageNumber = i;
        virtualMemory[i].pageOffset = 0;
    }
}
int VirtualMemorySystem::getValueByIndex(int index) {
    if (index >= 0 && index < VIRTUAL_MEMORY_SIZE * FRAME_SIZE) {
        int virtualPageIndex = index / FRAME_SIZE;
        int addressOffset = index % FRAME_SIZE;
        return virtualMemory[virtualPageIndex].addressSpace[addressOffset];
    } else {
        return -1;
    }
}
int VirtualMemorySystem::getVirtualPageNumberByIndex(int index) {
    if (index >= 0 && index < VIRTUAL_MEMORY_SIZE * FRAME_SIZE) {
        int virtualPageIndex = index / FRAME_SIZE;
        return virtualMemory[virtualPageIndex].virtualPageNumber;
    } else {
        return -1;
    }
}
void VirtualMemorySystem::setAddressSpace(int index, int frameIndex, int value) {
    if (index >= 0 && index < VIRTUAL_MEMORY_SIZE && frameIndex >= 0 && frameIndex < FRAME_SIZE)
        virtualMemory[index].addressSpace[frameIndex] = value;
}
int VirtualMemorySystem::getFrameSize() {
    return FRAME_SIZE;
}
int VirtualMemorySystem::getVirtualMemorySize() {
    return VIRTUAL_MEMORY_SIZE;
}
int VirtualMemorySystem::getVirtualFrameCount() {
    return VIRTUAL_FRAME;
}
void VirtualMemorySystem::printVirtualMemory() {
    for (int i = 0; i < VIRTUAL_FRAME; ++i) {
        std::cout << "Virtual Page Number: " << virtualMemory[i].virtualPageNumber << std::endl;
        for (int j = 0; j < FRAME_SIZE; ++j) {
            std::cout << virtualMemory[i].addressSpace[j] << " ";
        }
        std::cout << std::endl;
    }
}