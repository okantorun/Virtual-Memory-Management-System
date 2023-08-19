#include <iostream>
#include "PageTable.hpp"
#include <chrono>

uint64_t currentVirtualTime = 0;
const int threshold = 200;

class WSClock {
    private:
        PageTableEntry* head;
        PageTableEntry* current;

    public:
        void addPage(int pageNumber);
        int getReplacementPage(PageTable* pageTable);
        void setReferencedBit(int pageNumber, int referencedBit);
        void initializePageList(int size);
        void printPageList();
        void associativePageTable(PageTable* pageTable);
};

void WSClock::addPage(int physicalPageNumber) {
    PageTableEntry* newPage = new PageTableEntry;
    newPage->physicalPageNumber = physicalPageNumber;
    newPage->referencedBit = 0;
    
    if (head == nullptr) {
        head = newPage;
        newPage->next = newPage;
    } else {
        newPage->next = head->next;
        head->next = newPage;
    }
}

int WSClock::getReplacementPage(PageTable* pageTable) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    currentVirtualTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    while (true) {
        if (current->referencedBit == 1) {
            current->referencedBit = 0;
            current->timeOfLastUsed = currentVirtualTime;
        } else if (current->referencedBit == 0 && currentVirtualTime - current->timeOfLastUsed >= threshold) {
            current->referencedBit = 1;
            current->timeOfLastUsed = currentVirtualTime;
            int replacementPage = current->physicalPageNumber;
            current = current->next;
            return replacementPage;
        }
        else{
            current->timeOfLastUsed = currentVirtualTime - current->timeOfLastUsed;
        }

        for (int i = 0; i < pageTable->getEntrySize(); ++i) {
            if (pageTable->getPhysicalPageNumber(i) == current->physicalPageNumber) {
                // Eşleşen sayfa bulundu, referencedBit ve timeOfLastUsed değerlerini güncelle
                pageTable->setReferencedBit(i, current->referencedBit);
                pageTable->setTimeOfLastUsed(i, current->timeOfLastUsed);
                break;
            }
        }
        current = current->next;
    }
}


void WSClock::setReferencedBit(int physicalPageNumber, int referencedBit) {
    PageTableEntry* temp = head;
    
    do {
        if (temp->physicalPageNumber == physicalPageNumber) {
            temp->referencedBit = referencedBit;
            return;
        }
        
        temp = temp->next;
    } while (temp != head);
}

void WSClock::initializePageList(int size) {
    head = nullptr;
    current = nullptr;
    
    for (int i = 0; i < size; ++i) {
        PageTableEntry* newPage = new PageTableEntry;
        newPage->physicalPageNumber = i;
        newPage->referencedBit = 0;
        newPage->timeOfLastUsed = 0;
        
        if (head == nullptr) {
            head = newPage;
            newPage->next = newPage;
            current = newPage;
        } else {
            newPage->next = head->next;
            head->next = newPage;
        }
        
        head = newPage; // head her zaman yeni eklenen sayfayı göstersin
    }
}
void WSClock::printPageList() {
    PageTableEntry* temp = head;
    
    do {
        std::cout << "physicalPageNumber:" << temp->physicalPageNumber << " referencedBit:"<< temp->referencedBit << " timeOfLastUsed:" << temp->timeOfLastUsed << std::endl;
        temp = temp->next;
    } while (temp != head);
    
    std::cout << std::endl;
}

void WSClock::associativePageTable(PageTable* pageTable) {
    PageTableEntry* temp = head;

    do {
        int count = 0;
        int physicalPageNumber = temp->physicalPageNumber;
        for (int i = 0; i < pageTable->getEntrySize(); ++i) {
            pageTable->getPhysicalPageNumber(i);
            if (pageTable->getPhysicalPageNumber(i) == physicalPageNumber) {
                pageTable->setReferencedBit(i, temp->referencedBit);
                pageTable->setReferencedTime(i, temp->timeOfLastUsed);
                count++;
                break;
            }
            if (count == 0)
            {
                pageTable->setReferencedBit(i, 0);
                pageTable->setReferencedTime(i, 0);
            }
            
        }

        temp = temp->next;
    } while (temp != head);
}

