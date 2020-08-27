// #include "./trainModel/lr.h"
// #include "./trainModel/nn.h"
// // #include "./innerNode/adaptiveRMI.h"
// // #include "./innerNode/staticRMI.h"
// // #include "./innerNode/scaleModel.h"
// // #include "./leafNode/gappedNode.h"
// // #include "./leafNode/normalNode.h"

// #include <algorithm>
// #include <random>
// #include <iostream>

// int datasetSize = 10000;
// vector<pair<double, double>> dataset;
// vector<pair<double, double>> insertDataset;

// btree::btree_map<double, double> btreemap;

// staticRMI<normalNode<linearRegression>, linearRegression> SRMI_normal;
// staticRMI<gappedNode<linearRegression>, linearRegression> SRMI_gapped;
// adaptiveRMI<gappedNode<linearRegression>, linearRegression> ARMI_gapped;
// scaleModel<gappedNode<linearRegression>> SCALE_gapped;

// void generateDataset()
// {
//     float maxValue = 10000.00;

//     // create dataset randomly
//     std::default_random_engine generator;
//     std::lognormal_distribution<double> distribution(0.0, 2.0);
//     vector<double> ds;

//     for (int i = 0; i < datasetSize; i++)
//     {
//         ds.push_back(distribution(generator));
//     }
//     std::sort(ds.begin(), ds.end());

//     double maxV = ds[ds.size() - 1];
//     double factor = maxValue / maxV;
//     for (int i = 0; i < ds.size(); i++)
//     {
//         if (i % 10 != 0)
//         {
//             dataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
//             btreemap.insert({double(ds[i] * factor), double(ds[i] * factor) * 10});
//         }
//         else
//             insertDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
//     }
//     datasetSize = dataset.size();
// }

// void createModel()
// {
//     params firstStageParams(0.001, 100000, 8);
//     params secondStageParams(0.001, 100000, 8);

//     SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 1024, 128, 200);
//     SRMI_normal.train();
//     cout << "SRMI_normal init over!" << endl;

//     SRMI_gapped = staticRMI<gappedNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 5000, 128, 800);
//     SRMI_gapped.train();
//     cout << "SRMI_gapped init over!" << endl;

//     ARMI_gapped = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 12, 800);
//     ARMI_gapped.initialize(dataset);
//     cout << "ARMI_gapped init over!" << endl;

//     SCALE_gapped = scaleModel<gappedNode<linearRegression>>(secondStageParams, 1000, 100, 800);
//     SCALE_gapped.initialize(dataset);
//     cout << "SCALE_gapped init over!" << endl;
// }

// template <typename type>
// void find(type obj)
// {
//     for (int i = 0; i < datasetSize; i++)
//     {
//         cout << "Find " << i << ":    " << dataset[i].first;
//         auto res = obj.find(dataset[i].first);

//         cout << "  " << res.first << "  " << res.second << endl;
//     }
//     cout << "Find over!" << endl;
// }
// template <typename type>
// void insert(type obj)
// {
//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         cout << "Insert " << i << ":    " << insertDataset[i].first;

//         obj.insert(insertDataset[i]);

//         auto res = obj.find(insertDataset[i].first);
//         cout << "    After insert: " << res.first << "  " << res.second << endl;
//     }
// }
// template <typename type>
// void update(type obj)
// {
//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         cout << "Update " << i << ":    " << insertDataset[i].first;
//         obj.update({insertDataset[i].first, 1.11});

//         auto res = obj.find(insertDataset[i].first);
//         cout << "    After update: " << res.first << "  " << res.second << endl;
//     }
// }
// template <typename type>
// void del(type obj)
// {
//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         cout << "Delete " << i << ":    " << insertDataset[i].first;
//         obj.del(insertDataset[i].first);

//         auto res = obj.find(insertDataset[i].first);
//         cout << "    After delete: " << res.first << "  " << res.second << endl;
//     }
// }
// template <typename type>
// void test(type obj)
// {
//     find(obj);
//     insert(obj);
//     update(obj);
//     del(obj);
// }
// void btreetest()
// {
//     for (int i = 0; i < datasetSize; i++)
//     {
//         btreemap.find(dataset[i].first);
//     }

//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         btreemap.insert(insertDataset[i]);
//     }
//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         btreemap.find(insertDataset[i].first);
//     }

//     for (int i = 0; i < insertDataset.size(); i++)
//     {
//         btreemap.erase(insertDataset[i].first);
//     }
// }
// int main()
// {
//     generateDataset();
//     createModel();
//     test(SRMI_normal);
//     test(SRMI_gapped);
//     test(ARMI_gapped);
//     test(SCALE_gapped);
//     btreetest();
// }
