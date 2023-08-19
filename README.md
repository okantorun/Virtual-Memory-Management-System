# Virtual-Memory-Management-System
* It is homework for the Operating Systems course.

## General Info
* Virtual memory was designed by taking the frame size and virtual memory size information from the user. Address space, virtual page number and page off set values were determined for each page in the virtual memory.
* Physical memory was designed by taking the frame size and physical memory size information from the user. Address space, physical page number and page off set values were determined for each page in the physical memory.
* The information required for the page replacement algorithms was stored in the page table. For example, modified bit, present bit, referenced bit etc. Page table was designed in accordance with these and virtual memory.
* Unlike the standard page table, the Inverted Page Table, which can be more efficient in some cases, has been designed.
* Algorithms used in cases where page replacement is required were designed. These are Hardware Lru, Second Chance Fifo and WS Clock algorithms.
* A txt file was created as a disk and the information that did not fit in the memory was saved there.
* After designing the system end-to-end, matrix vector multiplication, array summation and search algorithms are run interactively and the results are shown in the report.
## Setup 
```
$ cd ../Virtual-Memory-Management-System/src
$ make
$ ./operateArrays 12 5 10 LRU 10000 diskFileName.dat
```
