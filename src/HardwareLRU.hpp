#include "PageTable.hpp"

class HardwareLRU {
    public:
        int getReplacementPage(PageTable* pageTable, int physicalFrameCount);
        
};

int HardwareLRU::getReplacementPage(PageTable* pageTable, int frameCount) {
    int min = counter;
    int minIndex = 0;
    for (int i = 0; i < frameCount; ++i) {
        if (pageTable->getReferencedTime(i) < min && pageTable->getPresent(i) == 1) {
            min = pageTable->getReferencedTime(i);
            minIndex = i;
            pageTable->setReferencedBit(i, 0);
            pageTable->setDirty(i, 0);
        }
    }
    return pageTable->getPhysicalPageNumber(minIndex);
}