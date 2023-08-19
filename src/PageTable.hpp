#ifndef PAGETABLE_HPP
#define PAGETABLE_HPP

#include <iostream>

uint64_t counter = 0;

struct PageTableEntry {
    int dirty; // modified bit
    int present; //For general use
    int referencedBit; //For SecondChanceFifo
    uint64_t referencedTime;//For HardwareLRU like counter
    uint64_t timeOfLastUsed; //For WSClock
    int physicalPageNumber; //Accessing to physical memory
    PageTableEntry* next;
};

class PageTable {
    private:
        PageTableEntry* entries; // Page tablosu
        const int ENTRY_SIZE; // Page tablosunun boyutu

    public:
        
        PageTable(int size);
        void initializePTable(int physicalFrame);
        int getDirty(int index);
        void setDirty(int index, bool dirty);
        int getPresent(int index);
        void setPresent(int index, bool present);
        int getPhysicalPageNumber(int index);
        void setPhysicalPageNumber(int index, int pageNumber);
        int updatePageTable(int oldPhysicalPageNumber,int newPhysicalPageNumber);
        void printPageTable();
        void setReferencedTime(int index, int referencedTime);
        int getReferencedTime(int index) { return entries[index].referencedTime; }
        void setReferencedBit(int index, int referencedBit) { entries[index].referencedBit = referencedBit; }
        int getEntrySize() { return ENTRY_SIZE; }
        void setTimeOfLastUsed(int index, int timeOfLastUsed) { entries[index].timeOfLastUsed = timeOfLastUsed; }
};


PageTable::PageTable(int size) : ENTRY_SIZE(size){
    entries = new PageTableEntry[size];
}
void PageTable::initializePTable(int physicalFrame) {
    for (int i = 0; i < ENTRY_SIZE; i++) {
        if(i < physicalFrame)
            entries[i].present = 1;
        else
            entries[i].present = 0;

        entries[i].dirty = 0;
        entries[i].physicalPageNumber = i;
    }
}
int PageTable::getDirty(int index)  {
    return entries[index].dirty;
}
void PageTable::setDirty(int index, bool dirty) {
    entries[index].dirty = dirty;
}
int PageTable::getPresent(int index)  {
    return entries[index].present;
}
void PageTable::setPresent(int index, bool present) {
    entries[index].present = present;
}
int PageTable::getPhysicalPageNumber(int index)  {
    return entries[index].physicalPageNumber;
}
void PageTable::setPhysicalPageNumber(int index, int pageNumber) {
    entries[index].physicalPageNumber = pageNumber;
}

int PageTable::updatePageTable(int oldPhysicalPageNumber,int newPhysicalPageNumber){
    for(int i=0;i<ENTRY_SIZE;i++){
        if(entries[i].physicalPageNumber == oldPhysicalPageNumber){
            entries[i].physicalPageNumber = newPhysicalPageNumber;
            entries[i].present = 0;
            entries[i].dirty = 0;
            return i;
        }
    }
    return -1;
}
void PageTable::setReferencedTime(int index, int referencedTime){
    entries[index].referencedTime = referencedTime;
}
void PageTable::printPageTable(){
    std::cout<<"****************************************************************************"<<std::endl;
    for(int i=0;i<ENTRY_SIZE;i++){
        std::cout << "Virtual Page Number: " << i << " Physical Page Number: " << entries[i].physicalPageNumber 
                    << " Present: " << entries[i].present << " Dirty: " << entries[i].dirty 
                    << " Referenced Bit: " << entries[i].referencedBit << " Referenced Time: "<< entries[i].referencedTime 
                    << " Time Of Last Used: " << entries[i].timeOfLastUsed << std::endl;

      
    }
    std::cout<<"****************************************************************************"<<std::endl;
}

#endif // PAGETABLE_HPP