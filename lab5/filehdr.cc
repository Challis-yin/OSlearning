// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
	int lastindex = NumDirect-1;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
		return FALSE;		// not enough space
	if(freeMap->NumClear()<numSectors)
		return false;
	else if(NumDirect+NumDirect2<=numSectors)
		return false;
	if(numSectors<lastindex)
	{
		for (int i = 0; i < numSectors; i++)
		dataSectors[i] = freeMap->Find();
		dataSectors[lastindex]=-1;
	}
    else{
	for (int i = 0;i<lastindex;i++)
        dataSectors[i] = freeMap->Find();
	dataSectors[lastindex]=freeMap->Find();
	int dataSector2[NumDirect2];
	for(int i = 0;i<numSectors-NumDirect;i++)
	    dataSector2[i] = freeMap->Find();
	synchDisk->WriteSector(dataSectors[lastindex],(char*)dataSector2);
    }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    int lastIndex = NumDirect-1;
	if(dataSectors[lastIndex]==-1){
		for (int i = 0; i < numSectors; i++) {
			ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
			freeMap->Clear((int) dataSectors[i]);
		}
	}
    else
	{
		int i = 0;
		for(;i<lastIndex;i++)
		{
			ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
			freeMap->Clear((int) dataSectors[i]);
		}
		int dataSector2[NumDirect2];
		synchDisk->ReadSector(dataSectors[lastIndex],(char*)dataSector2);
		freeMap->Clear((int)dataSectors[lastIndex]);
		for(;i<numSectors;i++)
			freeMap->Clear((int)dataSectors[i-lastIndex]);
	}
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
        int lastIndex = NumDirect-1;
	if(offset/SectorSize<lastIndex)
		return(dataSectors[offset / SectorSize]);
	else{
	int dataSectors2[NumDirect2];
	synchDisk->ReadSector(dataSectors[lastIndex],(char*)dataSectors2);
	return (dataSectors2[offset/SectorSize-lastIndex]);
	}
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
	int lastIndex = NumDirect-1;
    char *data = new char[SectorSize];
	if(dataSectors[lastIndex]==-1)
	{
		printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
		for (i = 0; i < numSectors; i++)
		printf("%d ", dataSectors[i]);
		printf("\nFile contents:\n");
		for (i = k = 0; i < numSectors; i++) {
		synchDisk->ReadSector(dataSectors[i], data);
			for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
			if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
			printf("%c", data[j]);
				else
			printf("\\%x", (unsigned char)data[j]);
		}
			printf("\n"); 
		}
	}
    else
	{
		int dataSectors2[NumDirect2];
		synchDisk->ReadSector(dataSectors[lastIndex],(char*)dataSectors2);
		printf("FileHeaderdddddd contents.  File size: %d.  File blocks:\n", numBytes);
		for (i = 0; i < lastIndex; i++)
			printf("%d ", dataSectors[i]);
		for(;i<numSectors;i++)
			printf("%d ", dataSectors2[i-lastIndex]);
		printf("\nFile contents:\n");
		for (i = k = 0; i < lastIndex; i++) {
		synchDisk->ReadSector(dataSectors[i], data);
			for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
			if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
			printf("%c", data[j]);
				else
			printf("\\%x", (unsigned char)data[j]);
		}
			printf("\n"); 
		}
		for (; i < numSectors; i++) {
		synchDisk->ReadSector(dataSectors2[i], data);
			for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
			if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
			printf("%c", data[j]);
				else
			printf("\\%x", (unsigned char)data[j]);
		}
			printf("\n"); 
		}
	}
    delete [] data;
}

/*********

edit by ycl
extend file size
*********/


bool

FileHeader::ExtendFileSize(BitMap *freeMap,int filesize)

{

    int restFileSize = SectorSize * numSectors - numBytes;
if(restFileSize>=filesize)
{
numBytes += filesize;
return true;
}
else
{
int moreFileSize = filesize - restFileSize;
	if(freeMap->NumClear()<divRoundUp(moreFileSize,SectorSize))
		return false;
	else if(NumDirect+NumDirect2<=numSectors+divRoundUp(moreFileSize,SectorSize))
		return false;
	int i = numSectors;
	numBytes += filesize;
	numSectors += divRoundUp(moreFileSize, SectorSize);
	int lastIndex = NumDirect-1;
	if(dataSectors[lastIndex]==-1)
	{
		if(numSectors < lastIndex)
			for(;i<numSectors;i++)
				dataSectors[i] = freeMap->Find();
		else
		{
			for(;i<lastIndex;i++)
				dataSectors[i] = freeMap->Find();
			dataSectors[lastIndex] = freeMap->Find();
			int dataSectors2[NumDirect2];
			for(;i<numSectors;i++)
				dataSectors2[i-lastIndex] = freeMap->Find();
			synchDisk->WriteSector(dataSectors[lastIndex],(char*)dataSectors2);
		}
	}
	else
	{
		int dataSectors2[NumDirect2];
		synchDisk->ReadSector(dataSectors[lastIndex],(char*)dataSectors2);
		for(;i<numSectors;i++){//printf(freeMap->Find());
			dataSectors2[i-lastIndex] = freeMap->Find();}
		synchDisk->WriteSector(dataSectors[lastIndex],(char*)dataSectors2);
	}
}
return true;
}



