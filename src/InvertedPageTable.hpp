#include <unordered_map>

const int HASH_TABLE_SIZE = 4;

struct PageTableNode {
    int virtual_address;
    int physical_address;
    PageTableNode* next;

    PageTableNode(int vaddr, int paddr) : virtual_address(vaddr), physical_address(paddr), next(nullptr) {}
};

class InvertedPageTable {
private:
    std::unordered_map<int, PageTableNode*> hash_table;

public:
    void InsertPage(int virtual_address, int physical_address) {
        int index = virtual_address % HASH_TABLE_SIZE;  // Sanal adresin bitini hash tablosunun boyutuna böl

        PageTableNode* new_node = new PageTableNode(virtual_address, physical_address);

        if (hash_table.find(index) == hash_table.end()) {
            // İndekste bir düğüm yoksa, yeni bir düğüm oluştur
            hash_table[index] = new_node;
        } else {
            // İndekste bir düğüm varsa, bağlı listeye yeni düğümü ekle
            PageTableNode* current = hash_table[index];
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = new_node;
        }
    }

    int LookupPage(int virtual_address) {
        int index = virtual_address % HASH_TABLE_SIZE;  // Sanal adresin bitini hash tablosunun boyutuna böl

        if (hash_table.find(index) != hash_table.end()) {
            // İndekste bir düğüm varsa, bağlı listeyi dolaşarak sanal adresin fiziksel adresini bul
            PageTableNode* current = hash_table[index];
            while (current != nullptr) {
                if (current->virtual_address == virtual_address) {
                    return current->physical_address;
                }
                current = current->next;
            }
        }

        // Eşleşme bulunamadıysa -1 döndür
        return -1;
    }
};