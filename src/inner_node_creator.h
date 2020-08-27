#include "./innerNode/lr_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/histogram_node.h"
#include "innerNode/inner_node.h"

BasicInnerNode *InnerNodeCreator(int innerNodeType, int childNum)
{
    BasicInnerNode *newNode;
    switch (innerNodeType)
    {
    case 0:
        newNode = new AdaptiveLR(childNum);
        break;
    case 1:
        newNode = new AdaptiveNN(childNum);
        break;
    case 2:
        newNode = new AdaptiveHis(childNum);
        break;
    case 3:
        newNode = new AdaptiveBin(childNum);
        break;
    }
    return newNode;
}
