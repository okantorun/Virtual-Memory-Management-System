#include <iostream>
#include <queue>
#include "PageTable.hpp"

struct Page {
    int pageNumber;
    int referencedBit;
};

class SecondChanceFifo {
private:
    std::queue<Page> pageQueue;

public:
    void addPage(int pageNumber);
    int getReplacementPage(PageTable* pageTable);
    void setReferencedBit(int pageNumber, int referencedBit);
    void initializePageQueue(int size);
    void assoociativePageTables(PageTable* pageTable);
};


void SecondChanceFifo::addPage(int pageNumber) {
    Page newPage;
    newPage.pageNumber = pageNumber;
    newPage.referencedBit = 0;
    pageQueue.push(newPage);
}

int SecondChanceFifo::getReplacementPage(PageTable* pageTable) {
    while (true) {
        Page frontPage = pageQueue.front();
        
        if (frontPage.referencedBit) {
            // Eğer referencedBit true ise, referencedBit false yapar ve sayfayı kuyruğun sonuna ekler
            frontPage.referencedBit = 0;
            pageQueue.push(frontPage);
            for (int i = 0; i < pageTable->getEntrySize(); ++i) {
                if (pageTable->getPhysicalPageNumber(i) == frontPage.pageNumber) {
                    pageTable->setReferencedBit(i, frontPage.referencedBit);
                    break;
                }
            }
        } else {
            // Eğer referencedBit false ise, sayfayı kuyruktan çıkarıp döndürür
            pageQueue.pop();
            return frontPage.pageNumber;
        }
        
        // Kuyruğun en başındaki sayfayı referencedBit bakıp güncelledikten sonra kuyruktan çıkarır
        pageQueue.pop();
    }
}
void SecondChanceFifo::setReferencedBit(int pageNumber, int referencedBit) {
    std::queue<Page> tempQueue;
    while (!pageQueue.empty()) {
        Page frontPage = pageQueue.front();
        pageQueue.pop();
        
        if (frontPage.pageNumber == pageNumber) {
            frontPage.referencedBit = referencedBit;
        }
        
        tempQueue.push(frontPage);
    }
    
    pageQueue = tempQueue;
}
void SecondChanceFifo::initializePageQueue(int size) {
    for (int i = 0; i < size; ++i) {
        Page newPage;
        newPage.pageNumber = i;
        newPage.referencedBit = 0;
        pageQueue.push(newPage);
    }
}

void SecondChanceFifo::assoociativePageTables(PageTable* pageTable) {
    
}




