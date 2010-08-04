#include "referenceViewer.h"

void addPseudoAdjacencyEnds(PseudoAdjacency *pseudoAdjacency, void *extraArgument) {
	/*
	 * Adds the two ends in a pseudo adjacency to the plot.
	 */
	End *_5End = pseudoAdjacency_get5End(pseudoAdjacency);
	End *_3End = pseudoAdjacency_get3End(pseudoAdjacency);
	addEnd(_5End, extraArgument);
	addEnd(_3End, extraArgument);
	//Now add the all important pseudo-adjacency edge..
	addPseudoAdjacencyEdge(_5End, _3End, extraArgument);
}

void addPseudoChromosomeEnds(PseudoChromosome *pseudoChromosome, void *extraArgument) {
	/*
	 * Adds the pseudo adjacencies' ends to the the plot.
	 */
	PseudoChromsome_PseudoAdjacencyIterator *pseudoAdjacencyIterator = pseudoChromosome_getPseudoAdjacencyIterator(pseudoChromosome);
	PseudoAdjacency *pseudoAdjacency;
	while((pseudoAdjacency = pseudoChromosome_getNextPseudoAdjacency(pseudoAdjacencyIterator)) != NULL) {
		addPseudoAdjacencyEnds(pseudoAdjacency, extraArgument);
	}
	pseudoChromosome_destructPseudoAdjacencyIterator(pseudoAdjacencyIterator);
}

void addReferenceEnds(Reference *reference, void *extraArgument) {
	/*
	 * Adds the ends in the reference structure to the plot.
	 */
	Reference_PseudoChromosomeIterator *pseudoChromosomeIterator = reference_getPseudoChromosomeIterator(reference);
	PseudoChromosome *pseudoChromosome;
	while((pseudoChromosome = reference_getNextPseudoChromosome(pseudoChromosomeIterator)) != NULL) {
		addPseudoChromosomeEnds(pseudoChromosome, extraArgument);
	}
	reference_destructPseudoChromosomeIterator(pseudoChromosomeIterator);
}

void addTelomereEdges(Reference *reference, void *extraArgument) {
	/*
	 * Adds edges between the stub ends of contiguous pseudo-chromosomes in the reference ordering to the plot.
	 */
	Reference_PseudoChromosomeIterator *pseudoChromosomeIterator = reference_getPseudoChromosomeIterator(reference);
	PseudoChromosome *pseudoChromosome, *pPseudoChromosome = NULL;
	while((pseudoChromosome = reference_getNextPseudoChromosome(pseudoChromosomeIterator)) != NULL) {
		if(pPseudoChromosome != NULL) {
			End *end1 = pseudoChromosome_get3End(pPseudoChromosome);
			End *end2 = pseudoChromosome_get5End(pseudoChromosome);
			addTelomereEdge(end1, end2, extraArgument);
		}
		pPseudoChromosome = pseudoChromosome;
	}
	//Now add the edge between the 5 end of the first pseudo-chromsome and the 3 end of the last pseudoChromosome.
	pseudoChromosome = reference_getFirst(reference);
	End *end1 = pseudoChromosome_get5End(pseudoChromosome);
	End *end2 = pseudoChromosome_get3End(pPseudoChromosome);
	addTelomereEdge(end1, end2, extraArgument);

	reference_destructPseudoChromosomeIterator(pseudoChromosomeIterator);
}

void addBlocks(Net *net, void *extraArgument) {
	/*
	 * Adds block edges to the plot.
	 */
	Net_EndIterator *endIterator = net_getEndIterator(net);
	End *end;
	while((end = net_getNextEnd(endIterator)) != NULL) {
		if(end_isBlockEnd(end)) {
			End *otherEnd = end_getOtherBlockEnd(end);
			assert(otherEnd != NULL);
			assert(end_getOrientation(end));
			assert(end_getOrientation(otherEnd));
			assert(end_getSide(end) != end_getSide(otherEnd));
			if(end_getSide(end)) { //only add in one direction
				addBlockEdge(end, end_getOtherBlockEnd(end), extraArgument);
			}
		}
	}
	net_destructEndIterator(endIterator);
}

void addTangle(Group *group, stHash *endToPseudoAdjacencyHash, void *extraArgument) {
	/*
	 * Adds a adjacency edge between pairs of ends connected in a tangle. Does not
	 * add any adjacency edges between ends already connected by a pseudo-adjacency edge,
	 * avoid making duplicate edges.
	 */
	assert(group_isTangle(group));
	Group_EndIterator *endIterator = group_getEndIterator(group);
	End *end;
	while((end = group_getNextEnd(endIterator)) != NULL) {
		if(end_isBlockEnd(end) || end_isAttached(end)) {
			stHash *edgesHash = stHash_construct();
			PseudoAdjacency *pseudoAdjacency = stHash_search(endToPseudoAdjacencyHash, end);
			assert(pseudoAdjacency != NULL);
			End *otherEnd = pseudoAdjacency_get5End(pseudoAdjacency) == end ? pseudoAdjacency_get3End(pseudoAdjacency) : pseudoAdjacency_get5End(pseudoAdjacency);
			assert(otherEnd != end);
			assert(end_getOrientation(otherEnd));
			Cap *cap;
			End_InstanceIterator *capIterator = end_getInstanceIterator(end);
			while((cap = end_getNext(capIterator)) != NULL) {
				Cap *adjacentCap = cap_getAdjacency(cap);
				if(adjacentCap != NULL) {
					End *adjacentEnd = end_getPositiveOrientation(cap_getEnd(adjacentCap));
					assert(adjacentEnd != NULL);
					if((end_isBlockEnd(adjacentEnd) || end_isAttached(adjacentEnd)) &&
					   adjacentEnd != otherEnd) {
						if(stHash_search(edgesHash, adjacentEnd) == NULL) {
							if(cactusMisc_nameCompare(end_getName(end), end_getName(adjacentEnd)) == 1) { //only add in one direction
								addAdjacencyEdge(end, adjacentEnd, extraArgument);
							}
							stHash_insert(edgesHash, adjacentEnd, adjacentEnd);
						}
					}
				}
			}
			end_destructInstanceIterator(capIterator);
			stHash_destruct(edgesHash);
		}
	}
	group_destructEndIterator(endIterator);
}

void addTangles(Net *net, Reference *reference, void *extraArgument) {
	/*
	 * Adds adjacency edges representing tangles to the structure.
	 */
	stHash *endToPseudoAdjacencyHash = reference_getEndToPseudoAdjacencyHash(reference);
	Net_GroupIterator *groupIterator = net_getGroupIterator(net);
	Group *group;
	while((group = net_getNextGroup(groupIterator)) != NULL) {
		if(group_isTangle(group)) { //the group is a tangle (not a link in a chain).
			addTangle(group, endToPseudoAdjacencyHash, extraArgument);
		}
	}
	net_destructGroupIterator(groupIterator);
	stHash_destruct(endToPseudoAdjacencyHash);
}

void makeReferenceGraph(Reference *reference, void *extraArgument) {
	/*
	 * Constructs a reference plot.
	 *
	 * First it add the ends to the plot, in order.
	 * Then it adds edges representing blocks, adjacencies and
	 * edges between telomeres that are adjacent in the ordering.
	 */
	Net *net = reference_getNet(reference);
	addReferenceEnds(reference, extraArgument);
	addBlocks(net, extraArgument);
	addTangles(net, reference, extraArgument);
	addTelomereEdges(reference, extraArgument);
}

