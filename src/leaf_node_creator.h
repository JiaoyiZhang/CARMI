
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"
#include "leafNode/leaf_node.h"

BasicLeafNode *LeafNodeCreator(int leafNodeType)
{
    BasicLeafNode *newNode;
    switch (leafNodeType)
    {
    case 0:
        newNode = new ArrayNode(kThreshold);
        break;
    case 1:
        newNode = new GappedArray(kThreshold);
        break;
    }
    return newNode;
}