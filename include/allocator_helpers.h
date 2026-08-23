#pragma once
#include "allocator.h"

int translate(int num);
void addSentinel(header *newBlock, int size);
header *bigRequest(int amount);
header *smallRequest();
header *splitBlock(header *newBlock, int amount, header **lists);
header *firstFit(int amount, header **lists, header **prev, int *listNum);
header *bestFit(int amount, header **lists, header **prev, int *listNum);
header *worstFit(int amount, header **lists, header **prev, int *listNum);
void forwardCoalesce(header **block, header **lists);
