#include "cactusGlobalsPrivate.h"

static CactusDisk * cactusDisk;
static Net * net;
static MetaEvent *rootMetaEvent;
static MetaEvent *leafMetaEvent;
static EventTree * eventTree;
static Event *rootEvent;
static Event *leafEvent;
static End *end1, *end2;
static Cap * topCap1, * topCap2, *topCap3, *topCap4;
static Cap * bottomCap1, * bottomCap2, *bottomCap3, *bottomCap4;
static Face * face;

static void cactusFacesTestSharedTeardown() {
	if(cactusDisk != NULL) {
		cactusDisk_destruct(cactusDisk);
		testCommon_deleteTemporaryNetDisk();
		cactusDisk = NULL;
	}
}

static void cactusFacesTestSharedSetup() {
	cactusFacesTestSharedTeardown();
	cactusDisk = cactusDisk_construct(testCommon_getTemporaryNetDisk());
	net = net_construct(cactusDisk);

	rootMetaEvent = metaEvent_construct("ROOT", cactusDisk);
	leafMetaEvent = metaEvent_construct("LEAF1", cactusDisk);

	eventTree = eventTree_construct(rootMetaEvent, net);

	rootEvent = eventTree_getRootEvent(eventTree);
	leafEvent = event_construct(leafMetaEvent, 0.2, rootEvent, eventTree);
	
	end1 = end_construct(1, net);
	end2 = end_construct(1, net);

	topCap1 = cap_construct(end_getReverse(end1), rootEvent);
	bottomCap1 = cap_construct(end_getReverse(end1), leafEvent);
	cap_makeParentAndChild(topCap1, bottomCap1);

	topCap2 = cap_construct(end1, rootEvent);
	bottomCap2 = cap_construct(end1, leafEvent);
	cap_makeParentAndChild(topCap2, bottomCap2);

	topCap3 = cap_construct(end_getReverse(end2), rootEvent);
	bottomCap3 = cap_construct(end_getReverse(end2), leafEvent);
	cap_makeParentAndChild(topCap3, bottomCap3);

	topCap4 = cap_construct(end2, rootEvent);
	bottomCap4 = cap_construct(end2, leafEvent);
	cap_makeParentAndChild(topCap4, bottomCap4);

	cap_makeAdjacent(topCap1, topCap3);
	cap_makeAdjacent(topCap2, topCap4);
	cap_makeAdjacent(bottomCap1, bottomCap4);
	cap_makeAdjacent(bottomCap2, bottomCap3);
	
	face = face_construct(net); 
	face_allocateSpace(face, 4);
	face_setTopNode(face, 0, topCap1);
	face_setTopNode(face, 1, topCap2);
	face_setTopNode(face, 2, topCap3);
	face_setTopNode(face, 3, topCap4);

	face_setBottomNodeNumber(face, 0, 1);
	face_addBottomNode(face, 0, bottomCap1);
	face_setDerivedDestination(face, 0, 0, topCap4);

	face_setBottomNodeNumber(face, 1, 1);
	face_addBottomNode(face, 1, bottomCap2);
	face_setDerivedDestination(face, 1, 0, topCap3);

	face_setBottomNodeNumber(face, 2, 1);
	face_addBottomNode(face, 2, bottomCap3);
	face_setDerivedDestination(face, 2, 0, topCap2);

	face_setBottomNodeNumber(face, 3, 1);
	face_addBottomNode(face, 3, bottomCap4);
	face_setDerivedDestination(face, 3, 0, topCap1);
}
