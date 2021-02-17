#ifndef CARMI_H
#define CARMI_H
#include "nodes/rootNode/bin_type.h"
#include "nodes/rootNode/his_type.h"
#include "nodes/rootNode/lr_type.h"
#include "nodes/rootNode/plr_type.h"

class CARMI
{
private:
    union CARMIRoot
    {
        LRType lrRoot;
        PLRType plrRoot;
        HisType hisRoot;
        BSType bsRoot;
        CARMIRoot(){};
        ~CARMIRoot(){};
    };

public:
    // static structure
    CARMI(const vector<pair<double, double>> &dataset, int childNum)
    {
        switch (kInnerNodeID)
        {
        case 0:
            root.lrRoot = LRType(childNum);
            root.lrRoot.Initialize(dataset);
            break;
        case 1:
            root.plrRoot = PLRType(childNum);
            root.plrRoot.Initialize(dataset);
            break;
        case 2:
            root.hisRoot = HisType(childNum);
            root.hisRoot.Initialize(dataset);
            break;
        case 3:
            root.bsRoot = BSType(childNum);
            root.bsRoot.Initialize(dataset);
            break;
        }
    };
    CARMI(const vector<pair<double, double>> &initData, const vector<pair<double, double>> &insertData)
    {
        rootType = Construction(initData, insertData);
    }
    CARMI(const vector<pair<double, double>> &findData)
    {
        vector<pair<double, double>> tmp;
        rootType = Construction(findData, tmp);
    }

    int Construction(const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData);

    pair<double, double> Find(int rootType, double key);
    bool Insert(int rootType, pair<double, double> data);
    bool Update(int rootType, pair<double, double> data);
    bool Delete(int rootType, double key);

    CARMIRoot root;
    int rootType;
};

#endif // !CARMI_H
