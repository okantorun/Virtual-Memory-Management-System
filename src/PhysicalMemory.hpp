class PhysicalMemorySystem {
    private:
        const int FRAME_SIZE;
        const int PHYSICAL_FRAME;
        const int PHYSICAL_MEMORY_SIZE;

        struct PhysicalMemoryPageEntry {
            long long int* addressSpace;
            int physicalPageNumber;
            int pageOffset;
        };

        PhysicalMemoryPageEntry* physicalMemory;


    public:
        PhysicalMemorySystem(int frameSize, int physicalFrame, int physicalMemorySize);
        void initializePM();
        void setAddressSpace(int frameIndex, int index, int value);
        int getAddressSpace(int frameIndex, int index);
      
};

PhysicalMemorySystem::PhysicalMemorySystem(int frameSize, int physicalFrame, int physicalMemorySize)
    : FRAME_SIZE(frameSize), PHYSICAL_FRAME(physicalFrame),
    PHYSICAL_MEMORY_SIZE(physicalMemorySize) {
    
    physicalMemory = new PhysicalMemoryPageEntry[PHYSICAL_FRAME];
    for (int i = 0; i < PHYSICAL_MEMORY_SIZE; ++i) {
        physicalMemory[i].addressSpace = new long long int[FRAME_SIZE];
    }
}
void PhysicalMemorySystem::initializePM() {
    int loadedValue=0;
    for (int i = 0; i < PHYSICAL_FRAME; ++i) {
        physicalMemory[i].physicalPageNumber = i;
        physicalMemory[i].pageOffset = 0;
    }
}
void PhysicalMemorySystem::setAddressSpace(int frameIndex, int index, int value) {
    if (index >= 0 && index < PHYSICAL_MEMORY_SIZE && frameIndex >= 0 && frameIndex < FRAME_SIZE)
        physicalMemory[frameIndex].addressSpace[index] = value;
}
int PhysicalMemorySystem::getAddressSpace(int frameIndex, int index) {
    if (index >= 0 && index < PHYSICAL_MEMORY_SIZE && frameIndex >= 0 && frameIndex < FRAME_SIZE)
        return physicalMemory[frameIndex].addressSpace[index];
    return -1;
}