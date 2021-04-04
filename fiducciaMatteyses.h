#ifndef HYPERGRAPHS_FIDUCCIAMATTEYSES_H
#define HYPERGRAPHS_FIDUCCIAMATTEYSES_H

#include "gainContainer.h"

void applyMove (GainContainer& gc, Partitions& partitionment, const pair <size_t, int>& best_move);
int FMpass (GainContainer& gc, Partitions& partitionment);
size_t FM (Partitions& partitions);


#endif //HYPERGRAPHS_FIDUCCIAMATTEYSES_H
