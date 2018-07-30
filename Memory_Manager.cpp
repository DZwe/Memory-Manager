// --------------------------------------------
//                MemoryManager.cpp
// 
// Author: Zwe Phone Shein   Date: 02/15/2016
//
// --------------------------------------------

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include "MemoryManager.h"

using namespace std;

ostream & operator<<(ostream & out,const MemoryManager &M)
{
   blocknode *tmp = M.firstBlock;
   assert(tmp);
   while(tmp)
   {
      out << "[" << tmp->bsize << ",";
      if (tmp->free)
	 out << "free] ";
      else
	 out << "allocated] ";
      if (tmp->next)
	 out << " -> "; 
      tmp = tmp->next;
   }
   return out;
}

MemoryManager::MemoryManager(unsigned int memtotal): memsize(memtotal)
{
   baseptr = new unsigned char[memsize];
   firstBlock = new blocknode(memsize,baseptr);
}

blocknode *MemoryManager::getFirstPtr()
{
   return firstBlock;
}

unsigned char * MemoryManager::malloc(unsigned int request)
// Finds the first block in the list whose size is >= request
// If the block's size is strictly greater than request
// the block is split, with the newly create block being free. 
// It then changes the original block's free status to false
{
   // If only one node exists, allocate the first node
   if (firstBlock->next == 0 && firstBlock->bsize >= request)
   {
      if ((firstBlock->bsize) > request)
      {
         splitBlock(firstBlock, request);
         firstBlock->free = false;
         return firstBlock->bptr;
      }

      firstBlock->free = false;
      return firstBlock->bptr;
   }

   // Declaration of temporary blocknode address holders
   blocknode *tmp1 = 0;
   blocknode *tmp2 = 0;
   tmp1 = firstBlock;

   // Find a free block along the blocknode list and create a node
   do
   {
      // Check whether the free node's bsize is sufficient
      while (tmp1->bsize < request)
      {
         tmp1 = tmp1->next; // Move on to another node
      }

      if (tmp1->bsize >= request && tmp1->free == true)
      {
         // If block size is greater than the request then split
         if (tmp1->bsize > request)
         {
            splitBlock(tmp1, request);
            tmp1->prev->free = false;
            return tmp1->prev->bptr;
         }

         // If block size is equal to request, just allocate the entire block
         tmp1->free = false;
         return tmp1->bptr;
      }

      // Move on to next node
      tmp1 = tmp1->next;

      if (tmp1->bsize >= request && tmp1->free == true)
      {
         // If block size is greater than the request then split
         if (tmp1->bsize > request)
         {
            tmp2 = tmp1;
            splitBlock(tmp1, request);
            tmp1->prev->free = false;

            return tmp1->prev->bptr;
         }

         // If block size is equal to request, just allocate the entire block
         tmp1->free = false;
         return tmp1->bptr;
      }

   } while (tmp1->next != 0);

   // If all conditions fail, just return NULL.
   return NULL;
}

void MemoryManager::splitBlock(blocknode *p, unsigned int chunksize)
// Utility function. Inserts a block after that represented by p
// changing p's blocksize to chunksize; the new successor node 
// will have blocksize the original blocksize of p minus chunksize and 
// will represent a free block.  
// Preconditions: p represents a free block with block size > chunksize
// and the modified target of p will still be free.
{
   // Check whether p is the firstnode
   if (p->prev == 0 && p->next == 0)
   {

      // Allocate memory from the free first node
      firstBlock = new blocknode(p->bsize - chunksize, p->bptr + chunksize);
      blocknode *tmp = firstBlock;

      // Make firstBlock point to the first allocated node
      firstBlock = p;
      firstBlock->next = tmp;
      firstBlock->prev = p->prev;
      tmp->prev = firstBlock;

      // Change free status of allocated node to false
      firstBlock->free = true;

      firstBlock->bsize = chunksize;
      tmp->free = true;
      return;
   }

   // If p is not the first node, store the first node temporarily
   blocknode *tmp = firstBlock;

   // New node is created
   firstBlock = new blocknode(chunksize, p->bptr);
   blocknode *tmp2 = firstBlock;

   // Assign proper data members for the nodes
   tmp2->free = true;
   tmp2->prev = p->prev;
   tmp2->next = p;
   p->prev = tmp2;
   p->bsize = p->bsize - chunksize;
   p->bptr = p->bptr + chunksize;
   p->free = true;

   // Assign block pointer for the newly created node
   tmp2->bptr = p->bptr;

   // Make the firstBlock the first node in the blocknode list
   while (tmp->prev != 0){
      tmp = tmp->prev;
   }
   firstBlock = tmp;
      
   // Check whether if it's the first node
   if (tmp2->prev != 0)
   {
      tmp2->prev->next = tmp2;
   }

   // Assign proper pointers
   blocknode *tmp3 = p;
   p = tmp2;
   tmp2 = tmp3;

   return;
}

void MemoryManager::free(unsigned char *blockptr)
// makes the block represented by the blocknode free
// and merges with successor, if it is free; also 
// merges with the predecessor, it it is free
{
   // Temporary node pointer
   blocknode *tmp = 0;
   tmp = firstBlock;

   // Find the node of the given block pointer
   while (tmp->bptr != blockptr)
   {
      tmp = tmp->next;
   }

   // Check if successor node is also free
   if (tmp->next != 0 && tmp->next->free == true)
   {
      mergeForward(tmp); // Merge with the free sucessor node
   }

   // Check if predecessor node is also free
   if (tmp->prev != 0 && tmp->prev->free == true)
   {
      mergeForward(tmp->prev); // Merge with the free predecessor node
   }

   // If predecessor nor successor is free, then free the chosen block
   tmp->free = true;

   return;

}

void MemoryManager::mergeForward(blocknode *p)
// merges two consecutive free blocks
// using a pointer to the first blocknode;
// following blocknode is deleted
{
   blocknode *tmp = 0;

   // Assign tmp the successor node of p
   tmp = p->next;

   // Combine the bsizes
   p->bsize = p->bsize + tmp->bsize;

   // Properly assign the prev and next pointers
   p->next = tmp->next;
   if (tmp->next != 0)
   {
      tmp->next->prev = p;
   }

   // Delete the sucessor node
   delete tmp;

   return;
}

