#include <vector>
#include <unordered_map>
#include <bitset>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>
#include "VirtualMemory.hpp"
#include "PhysicalMemory.hpp"
#include "InvertedPageTable.hpp"
#include "SecondChanceFifo.hpp"
#include "HardwareLRU.hpp"
#include "WSClock.hpp"
using namespace std;

int numberOfReads = 0;
int numberOfWrites = 0;
int numberOfPageMisses = 0;
int numberOfPageReplacements = 0;
int numberOfDiskPageWrites = 0;
int numberOfDiskPageReads = 0;

long long int memoryAccessCount = 0;
int memoryAccessFlag;

int isPresent(int virtualPageNumber,PageTable &PT){
    return PT.getPresent(virtualPageNumber);
}

void matrixVectorMultiply(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU,
                                        WSClock &wsclock,string diskName,int algorithmFlag) 
{
    int totalMatrixElements = size * 3 / 4;
    int vectorSize = size - totalMatrixElements;
    int matrixElement,vectorElement;
    int matrixVPNumber,matrixPPNumber,vectorVPNumber,vectorPPNumber,matrixPageOffset,vectorPageOffset;
    int prevPhysicalPageNumber = -1;
    int matrixValue,vectorValue;
    ofstream outputFile(diskName, ios::app);
    int temp=0;
    int count =0;

    for(int i=0;i<totalMatrixElements;i++){
        matrixVPNumber = VM.getVirtualPageNumberByIndex(i);
        matrixPPNumber = PT.getPhysicalPageNumber(matrixVPNumber);
        matrixPageOffset = i % VM.getFrameSize();
        if(isPresent(matrixVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            memoryAccessCount++;
            numberOfReads++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(matrixVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(matrixPPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(matrixPPNumber,1);
                
            }
            PT.setReferencedBit(matrixVPNumber,1);
            matrixValue = PM.getAddressSpace(matrixPPNumber,matrixPageOffset);
        
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            numberOfPageMisses++;
            numberOfPageReplacements++;

            int oldestPPNumber;
            if(algorithmFlag == 2){ //LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
            }
            else if(algorithmFlag == 1){ //SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = matrixPPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(matrixVPNumber,oldestPPNumber);  

            PT.setPresent(matrixVPNumber,1);
            PT.setDirty(vectorVPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {              
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == matrixPPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            matrixValue = PM.getAddressSpace(oldestPPNumber,matrixPageOffset);
            numberOfReads++;
            matrixPPNumber = oldestPPNumber;

        }
        if(totalMatrixElements+temp >= size){
            temp=0;
        }
        vectorVPNumber = VM.getVirtualPageNumberByIndex(totalMatrixElements+temp);
        vectorPPNumber = PT.getPhysicalPageNumber(vectorVPNumber);
        vectorPageOffset = (totalMatrixElements+temp) % VM.getFrameSize();
        //cout<<vectorVPNumber<<" "<<vectorPPNumber<<" "<<vectorPageOffset<<endl;
        
        if(isPresent(vectorVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(vectorVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(vectorPPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(vectorPPNumber,1);
                
            }
            vectorValue = PM.getAddressSpace(vectorPPNumber,vectorPageOffset);
            numberOfReads++;
            PM.setAddressSpace(matrixPPNumber,matrixPageOffset,(matrixValue*vectorValue)); 
            numberOfWrites++;   
            PT.setDirty(vectorVPNumber,1);   
            PT.setReferencedBit(matrixVPNumber,1);  
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            int oldestPPNumber;
            if(algorithmFlag == 2){
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
            }
            else if(algorithmFlag == 1){
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
                /*cout<<"oldestPPNumber: "<<oldestPPNumber<<endl;
                break;*/
            }
            
            int assignNewPPNumber = vectorPPNumber + VM.getVirtualFrameCount();
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            PT.setPhysicalPageNumber(vectorVPNumber,oldestPPNumber);  
            PT.setPresent(vectorVPNumber,1);
            PT.setDirty(vectorVPNumber,0);
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }          
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == vectorPPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            vectorValue = PM.getAddressSpace(oldestPPNumber,vectorPageOffset);
            numberOfReads++;
            vectorPPNumber = oldestPPNumber;
            PM.setAddressSpace(matrixPPNumber,matrixPageOffset,(matrixValue*vectorValue));
            numberOfWrites++; 
            PT.setDirty(matrixVPNumber,1);
        }
        temp++;
        if((memoryAccessCount % memoryAccessFlag) == 0)
        {
            PT.printPageTable();
        }
    }
    PT.printPageTable();
    outputFile.close();

   
}

// Array Summation
void arraySummation(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                    SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU, 
                                            WSClock &wsclock,string diskName,int algorithmFlag) {
    int totalMultiplicationResults = size * 3 / 4;
    int eachRowSize = size / 4;
    int matrixVPNumber,matrixPPNumber,pageOffset;
    long long int result =0;
    ofstream outputFile(diskName, ios::app);

    for(int i=0;i<totalMultiplicationResults;i++)
    {
        matrixVPNumber = VM.getVirtualPageNumberByIndex(i);
        matrixPPNumber = PT.getPhysicalPageNumber(matrixVPNumber);
        pageOffset = i % VM.getFrameSize();
        if(isPresent(matrixVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(matrixVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(matrixVPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(matrixPPNumber,1);
                
            }
            result += PM.getAddressSpace(matrixPPNumber,pageOffset);
            PT.setReferencedBit(matrixVPNumber,1);
            numberOfReads++;
           
        }
        else
        {
             //cout<<"Page Fault Occured"<<endl;
            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            int oldestPPNumber;
            if(algorithmFlag == 2){//LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
            }
            else if(algorithmFlag == 1){//SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
                /*cout<<"oldestPPNumber: "<<oldestPPNumber<<endl;
                break;*/
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = matrixPPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(matrixVPNumber,oldestPPNumber);  

            PT.setPresent(matrixVPNumber,1);
            PT.setDirty(matrixVPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == matrixPPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            result += PM.getAddressSpace(oldestPPNumber,pageOffset);
            numberOfReads++;
            
        }
        if((memoryAccessCount % memoryAccessFlag) == 0)
        {
            PT.printPageTable();
        }
    }
    outputFile.close();
    PT.printPageTable();
    cout<<"Result: "<<result<<endl;
}

void transposeOfVector(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                    SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU, 
                                            WSClock &wsclock,string diskName,int algorithmFlag) 
{
    int vectorSize = size * 2 / 4;
    int matrixElement,vectorElement;
    int vectorVPNumber,vectorPPNumber,vectorPageOffset,transposeVPNumber,transposePPNumber,transposePageOffset;
    int prevPhysicalPageNumber = -1;
    int vectorValue,transposeVectorValue;
    ofstream outputFile(diskName, ios::app);
    int temp=0;
    int count =0;
    int result =0;
    for(int i=0;i<vectorSize;i++)
    {
        vectorVPNumber = VM.getVirtualPageNumberByIndex(i);
        vectorPPNumber = PT.getPhysicalPageNumber(vectorVPNumber);
        vectorPageOffset = i % VM.getFrameSize();
        //cout<<vectorVPNumber<<" "<<vectorPPNumber<<" "<<vectorPageOffset<<endl;
        if(isPresent(vectorVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(vectorVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(vectorPPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(vectorPPNumber,1);
                
            }
            vectorValue = PM.getAddressSpace(vectorPPNumber,vectorPageOffset);
            PT.setReferencedBit(vectorVPNumber,1);
            numberOfReads++;
        
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            int oldestPPNumber;
            if(algorithmFlag == 2){ //LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
                //out<<"oldestPPNumber: "<<oldestPPNumber<<endl;
            }
            else if(algorithmFlag == 1){ //SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = vectorPPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(vectorVPNumber,oldestPPNumber);  

            PT.setPresent(vectorVPNumber,1);
            PT.setDirty(vectorVPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == vectorPPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            vectorValue = PM.getAddressSpace(oldestPPNumber,vectorPageOffset);
            numberOfReads++;
            vectorPPNumber = oldestPPNumber;

        }
       
        transposeVPNumber = VM.getVirtualPageNumberByIndex(vectorSize+i);
        transposePPNumber = PT.getPhysicalPageNumber(transposeVPNumber);
        transposePageOffset = (vectorSize+i) % VM.getFrameSize();
        //cout<<"okan"<<endl;
        //cout<<transposeVPNumber<<" "<<transposePPNumber<<" "<<transposePageOffset<<endl;
        
        if(isPresent(transposeVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(transposeVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(transposePPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(transposePPNumber,1);
                
            }
            vectorValue = PM.getAddressSpace(vectorPPNumber,vectorPageOffset);
            numberOfReads++;
            PM.setAddressSpace(transposePPNumber,transposePageOffset,vectorValue);   
            PT.setDirty(vectorVPNumber,1); 
            PT.setReferencedBit(transposeVPNumber,1);
            numberOfWrites++;      
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            int oldestPPNumber;
            if(algorithmFlag == 2){//LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
            }
            else if(algorithmFlag == 1){//SC
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            
            int assignNewPPNumber = transposePPNumber + VM.getVirtualFrameCount();
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            PT.setPhysicalPageNumber(transposeVPNumber,oldestPPNumber);  
            PT.setPresent(transposeVPNumber,1);
            PT.setDirty(transposeVPNumber,0);
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }          
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == transposePPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            transposePPNumber = oldestPPNumber;
            PM.setAddressSpace(transposePPNumber,transposePageOffset,vectorValue);
            PT.setDirty(transposeVPNumber,1);
            numberOfWrites++; 
        }
        temp++;
        if((memoryAccessCount % memoryAccessFlag) == 0)
        {
            PT.printPageTable();
        }
    }
    outputFile.close();
    PT.printPageTable();
}
void transposeVectorMultiply(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                    SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU, 
                                            WSClock &wsclock,string diskName,int algorithmFlag) 
{
    int vectorSize = size * 2 / 4;
    int matrixElement,vectorElement;
    int vectorVPNumber,vectorPPNumber,vectorPageOffset,transposeVPNumber,transposePPNumber,transposePageOffset;
    int prevPhysicalPageNumber = -1;
    int vectorValue,transposeVectorValue;
    ofstream outputFile(diskName, ios::app);
    int temp=0;
    int count =0;
    int result =0;
    for(int i=0;i<vectorSize;i++)
    {
        vectorVPNumber = VM.getVirtualPageNumberByIndex(i);
        vectorPPNumber = PT.getPhysicalPageNumber(vectorVPNumber);
        vectorPageOffset = i % VM.getFrameSize();
        //cout<<vectorVPNumber<<" "<<vectorPPNumber<<" "<<vectorPageOffset<<endl;
        if(isPresent(vectorVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(vectorVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(vectorPPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(vectorPPNumber,1);
                
            }
            vectorValue = PM.getAddressSpace(vectorPPNumber,vectorPageOffset);
            PT.setReferencedBit(vectorVPNumber,1);
            numberOfReads++;
        
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            int oldestPPNumber;
            if(algorithmFlag == 2){ //LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
                
            }
            else if(algorithmFlag == 1){ //SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = vectorPPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(vectorVPNumber,oldestPPNumber);  

            PT.setPresent(vectorVPNumber,1);
            PT.setDirty(vectorVPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == vectorPPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            vectorValue = PM.getAddressSpace(oldestPPNumber,vectorPageOffset);
            numberOfReads++;
            vectorPPNumber = oldestPPNumber;

        }
       
        transposeVPNumber = VM.getVirtualPageNumberByIndex(vectorSize+i);
        transposePPNumber = PT.getPhysicalPageNumber(transposeVPNumber);
        transposePageOffset = (vectorSize+i) % VM.getFrameSize();
      
        
        if(isPresent(transposeVPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(transposeVPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(transposePPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(transposePPNumber,1);
                
            }
            vectorValue = PM.getAddressSpace(vectorPPNumber,vectorPageOffset);
            numberOfReads++;
            transposeVectorValue = PM.getAddressSpace(transposePPNumber,transposePageOffset);
            numberOfReads++;
            PM.setAddressSpace(vectorPPNumber,vectorPageOffset,(vectorValue*transposeVectorValue)); 
            PT.setDirty(vectorVPNumber,1);
            PT.setReferencedBit(vectorVPNumber,1);
            numberOfWrites++;         
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            int oldestPPNumber;
            if(algorithmFlag == 2){
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
            }
            else if(algorithmFlag == 1){
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            
            int assignNewPPNumber = transposePPNumber + VM.getVirtualFrameCount();
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            PT.setPhysicalPageNumber(transposeVPNumber,oldestPPNumber);  
            PT.setPresent(transposeVPNumber,1);
            PT.setDirty(transposeVPNumber,0);
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }          
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == transposePPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            transposeVectorValue = PM.getAddressSpace(oldestPPNumber,transposePageOffset);
            numberOfReads++;
            transposePPNumber = oldestPPNumber;
            PM.setAddressSpace(vectorPPNumber,vectorPageOffset,(transposeVectorValue*vectorValue));
            numberOfWrites++; 
            PT.setDirty(vectorVPNumber,1);
        }
        temp++;
        if((i % memoryAccessFlag) == 0)
        {
            PT.printPageTable();
        }
    }
    outputFile.close();
    PT.printPageTable();
}

int linerSearch(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                    SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU, 
                                            WSClock &wsclock,string diskName,int algorithmFlag) 
{   
     int VPNumber,PPNumber,PageOffset;
     int value;
    int target =900000000;
    ofstream outputFile(diskName, ios::app);

     for(int i=0;i<size;i++)
     {
        VPNumber = VM.getVirtualPageNumberByIndex(i);
        PPNumber = PT.getPhysicalPageNumber(VPNumber);
        PageOffset = i % VM.getFrameSize();
       
        if(isPresent(VPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(VPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(PPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(PPNumber,1);
                
            }
            value = PM.getAddressSpace(PPNumber,PageOffset);
            PT.setReferencedBit(VPNumber,1);
            if(target == value){
                return i;
            }
        
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            int oldestPPNumber;
            if(algorithmFlag == 2){ //LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
                
            }
            else if(algorithmFlag == 1){ //SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = PPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(VPNumber,oldestPPNumber);  

            PT.setPresent(VPNumber,1);
            PT.setDirty(VPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == PPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            value = PM.getAddressSpace(oldestPPNumber,PageOffset);
            PPNumber = oldestPPNumber;
            if(value == target){
                return i;
            }

        }
        if((memoryAccessCount % memoryAccessFlag) == 0)
        {
            PT.printPageTable();
        }

     }
     outputFile.close();
     PT.printPageTable();
     return -1;
}
int binarySearch(int size,VirtualMemorySystem &VM,PhysicalMemorySystem &PM,PageTable &PT,
                                    SecondChanceFifo &secondChanceFifo,HardwareLRU &hardwareLRU, 
                                            WSClock &wsclock,string diskName,int algorithmFlag) 
{
    int left = 0;
    int right = size/2 - 1;
    int VPNumber,PPNumber,PageOffset;
     int value;
    int target =400000000;
    ofstream outputFile(diskName, ios::app);

    while (left <= right) {
        int mid = left + (right - left) / 2;
        //cout<<right<<" "<<left<<" "<<mid<<endl;

        VPNumber = VM.getVirtualPageNumberByIndex(mid);
        PPNumber = PT.getPhysicalPageNumber(VPNumber);
        PageOffset = mid % VM.getFrameSize();

        if(isPresent(VPNumber,PT))
        {
            //cout<<"Page Hit Occured"<<endl;
            numberOfReads++;
            memoryAccessCount++;
            if(algorithmFlag == 2){
                counter++;
                PT.setReferencedTime(VPNumber,counter);
            }
            else if (algorithmFlag == 1)
            {
                secondChanceFifo.setReferencedBit(PPNumber,1);
            }
            else if(algorithmFlag == 3){
                currentVirtualTime++;
                wsclock.setReferencedBit(PPNumber,1);
                
            }
            value = PM.getAddressSpace(PPNumber,PageOffset);
            PT.setReferencedBit(VPNumber,1);
        
        }
        else
        {
            //cout<<"Page Fault Occured"<<endl;
            numberOfPageMisses++;
            numberOfPageReplacements++;

            //Burada SCF'ye göre hangi page kaldırılacaksa onun physical page numberı alınır
            int oldestPPNumber;
            if(algorithmFlag == 2){ //LRU
                oldestPPNumber = hardwareLRU.getReplacementPage(&PT,VM.getVirtualFrameCount());
                
            }
            else if(algorithmFlag == 1){ //SCF
                oldestPPNumber = secondChanceFifo.getReplacementPage(&PT);
            }
            else if(algorithmFlag == 3){ //WSClock
                oldestPPNumber = wsclock.getReplacementPage(&PT);
            }
            //Diskte aynı ppnumber a sahip başka page olmasın
            int assignNewPPNumber = PPNumber + VM.getVirtualFrameCount();
            //Burada değiştirilecek olan page i önceden işaret eden virtual addressin oraya olan işareti değiştirilir
            PT.updatePageTable(oldestPPNumber,assignNewPPNumber);
            //Burada ise page fault alan virtual addresse ,istenilen page'in memorydeki numberi atanır
            PT.setPhysicalPageNumber(VPNumber,oldestPPNumber);  

            PT.setPresent(VPNumber,1);
            PT.setDirty(VPNumber,0);
            //Fifoya diskten getirilen page eklenir,yeni page number'ı ile
            secondChanceFifo.addPage(oldestPPNumber);
            secondChanceFifo.setReferencedBit(oldestPPNumber,1);
            
            
            //Burada physical memorydeki page'in içindeki değerler disk'e yazılır
            long long int value;
            for (int j = 0; j < VM.getFrameSize(); j++) {
                numberOfDiskPageWrites++;
                numberOfReads++;
                value = PM.getAddressSpace(oldestPPNumber, j);
                outputFile << assignNewPPNumber << " " << value << endl;
            }
            
            //Burada ise diskteki değerler okunur ve physical memorydeki page'e yazılır
            string line;
            ifstream inputFile(diskName);
            for (int j = 0; j < VM.getFrameSize(); j++) {
                while (getline(inputFile, line)) {
                    stringstream ss(line);
                    int pageNumber;
                    ss >> pageNumber;
                    if (pageNumber == PPNumber) {
                        ss >> value;
                        PM.setAddressSpace(oldestPPNumber, j, value);
                        numberOfDiskPageReads++;
                        numberOfWrites++;
                        break;
                    }
                }
                
            }
            inputFile.close();
            value = PM.getAddressSpace(oldestPPNumber,PageOffset);
            PPNumber = oldestPPNumber;

        }

        if (value == target) {
            return mid;  // Return the index if the target is found
        } else if (value < target) {
            left = mid + 1;  // Search the right half
        } else {
            right = mid - 1;  // Search the left half
        }

        if((memoryAccessCount % memoryAccessFlag) == 0)
        {
           PT.printPageTable();
        }
    }

    outputFile.close();
    PT.printPageTable();
    return -1;  // Return -1 if the target is not found
}

void fillPhysicalMemory(PhysicalMemorySystem &PM,VirtualMemorySystem &VM,PageTable &PT,string diskName){
    int virtualPageNumber,physicalPageNumber,pageOffset;
    ofstream outputFile(diskName, ios::app);

    for(int i=0;i<VM.getVirtualMemorySize();i++){
        virtualPageNumber = VM.getVirtualPageNumberByIndex(i);
        physicalPageNumber = PT.getPhysicalPageNumber(virtualPageNumber);
        pageOffset = i % VM.getFrameSize();
        if(isPresent(virtualPageNumber,PT))
        {
            numberOfWrites++;
            PM.setAddressSpace(physicalPageNumber,pageOffset,VM.getValueByIndex(i));
        }
        else
        {
            numberOfDiskPageWrites++;
            outputFile << physicalPageNumber << " " << VM.getValueByIndex(i) << endl;
        }
        
    }
    outputFile.close();
}



int main(int argc, char* argv[]) {
    if (argc != 8) {
        cout << "Incorrect Entry !!!" << endl;
        return 1;
    }

    int algorithmFlag = 0;

    int frameSizePower = stoi(argv[1]);
    int frameSize = pow(2, frameSizePower);
    
    int physicalFramePow = stoi(argv[2]);
    int physicalFrame = pow(2, physicalFramePow);

    int virtualFramePow = stoi(argv[3]);
    int virtualFrame = pow(2, virtualFramePow);

    string algorithmName = argv[4];
    if (algorithmName == "SC") {
        algorithmFlag = 1;
    }
    if(algorithmName == "LRU"){
        algorithmFlag = 2;
    }
    if(algorithmName == "WSClock"){
        algorithmFlag = 3;
    }
    memoryAccessFlag = stoi(argv[6]);
    string diskName = argv[7];

    int virtualMemorySize = virtualFrame*frameSize;
    int physicalMemorySize = physicalFrame*frameSize;
    

    VirtualMemorySystem virtualMemorySystem(frameSize, virtualFrame, virtualMemorySize);
    virtualMemorySystem.initializeVM();

    PhysicalMemorySystem physicalMemorySystem(frameSize, physicalFrame, physicalMemorySize);
    physicalMemorySystem.initializePM();

    PageTable pageTable(virtualFrame);
    pageTable.initializePTable (physicalFrame);

    fillPhysicalMemory(physicalMemorySystem,virtualMemorySystem,pageTable,diskName);

    SecondChanceFifo secondChanceFifo;
    secondChanceFifo.initializePageQueue(physicalFrame);

    HardwareLRU hardwareLRU;

    WSClock wsclock;
    wsclock.initializePageList(physicalFrame);

    
    
    matrixVectorMultiply(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);
    arraySummation(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);
    /*transposeOfVector(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);

    transposeVectorMultiply(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);
    int a =linerSearch(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);

    cout<<"Target Value is "<<a/frameSize<<". frame in Virtual Memory"<<"and "<<a<<". index"<<endl;*/

     /*int a = binarySearch(virtualMemorySize,virtualMemorySystem,physicalMemorySystem,
                                                pageTable,secondChanceFifo,hardwareLRU,wsclock,diskName,algorithmFlag);*/

    //cout<<"a: "<<a<<endl;
    cout << "Virtual Memory Size: " << virtualMemorySize << endl;
    cout << "Physical Memory Size: " << physicalFrame*frameSize << endl;
    cout << "Frame Size: " << frameSize << endl;
    cout << "Virtual Frame: " << virtualFrame << endl;
    cout << "Physical Frame: " << physicalFrame << endl;
    cout << "Algorithm Name: " << algorithmName << endl;

    cout<<"numberOfReads: "<<numberOfReads<<endl;
    cout<<"numberOfWrites: "<<numberOfWrites<<endl;
    cout<<"numberOfPageMisses: "<<numberOfPageMisses<<endl;
    cout<<"numberOfPageReplacements: "<<numberOfPageReplacements<<endl;
    cout<<"numberOfDiskPageReads: "<<numberOfDiskPageReads<<endl;
    cout<<"numberOfDiskPageWrites: "<<numberOfDiskPageWrites<<endl;

    
}

