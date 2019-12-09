#ifndef RMI_H
#define RMI_H

#include "net.h"
#include "weightInitializer.h"
#include "../cpp-btree/btree_map.h"
#include <unordered_set>
#include <chrono>
#include <algorithm>
#include <boost/optional.hpp>

class params
{
public:
    int batchSize;
    int maxNumEpochs;
    float learningRate;
    int neuronNumber;
};

template <typename KeyType>
std::unordered_set<KeyType> getRandomBatch(int batchSize, int datasetSize)
{
    assert(datasetSize >= batchSize && "Dataset size is smaller than requested batch size, which causes an infinite loop");

    std::unordered_set<KeyType> randomKeys;
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<KeyType> distribution(0, datasetSize);

    while (randomKeys.size() < batchSize)
    {
        auto val = distribution(rng);
        randomKeys.insert(val);
    }
    return randomKeys;
}

template <typename KeyType>
class SecondStageNode
{
public:
    // initial
    SecondStageNode(int threshold, int netBatchSize)
    {
        isUseTree = false;
        m_threshold = threshold;
        isValidSubModel = false;
        maxNegativeError = 0;
        maxPositiveError = 0;

        net.reset(new Net<float>());
        net->add(new Dense<float, 2>(netBatchSize, 1, 1, true, InitializationScheme::GlorotNormal));
    }

    // return whether the node is valid
    bool isValid()
    {
        return isValidSubModel;
    }

    // retirn max negative error
    float getMaxNegativeError()
    {
        return maxNegativeError;
    }

    // return max positive error
    float getMaxPositiveError()
    {
        return maxPositiveError;
    }

    void train(const std::vector<std::pair<KeyType, size_t>> &data, const params &trainParams, size_t totalDatasetSize); // train the submodel

    // return whether to use btree
    bool useBtree()
    {
        return isUseTree;
    }

    // return <key, idx> using btree
    boost::optional<std::pair<KeyType, int>> treeFind(KeyType key)
    {
        assert(isUseTree && "Called treeFind but the tree isn't supposed to be used");
        auto result = m_tree.find(key);
        if (result != m_tree.end())
            return std::pair<KeyType, int>(result->first, result->second); // return the result of btree.find
        else
            return std::pair<KeyType, int>(-1, -1); // if no find, return <-1,-1>
    }
    // predict the location of the key
    int predict(KeyType key, int totalDatasetSize)
    {
        Eigen::Tensor<float, 2> input(1, 1); //construct a 2d matrix, the length of each dim is 1, 1
        input(0, 0) = static_cast<float> (key);

        auto result = net->forward<2, 2>(input);
        result *= result.constant(totalDatasetSize);
        return static_cast<int>(result(0, 0));
    }

private:
    int m_threshold;
    bool isUseTree;
    bool isValidSubModel;

    std::unique_ptr<Net<float>> net;
    int maxNegativeError;
    int maxPositiveError;

    btree::btree_map<KeyType, size_t> m_tree;
};

template <typename KeyType>
void SecondStageNode<KeyType>::train(const std::vector<std::pair<KeyType, size_t>> &data, const params &trainParams, size_t totalDatasetSize)
{
    size_t size = data.size();
    if (size == 0)
    {
        std::cerr << "Dataset of this submodel is empty!" << std::endl;
        isValidSubModel = false;
        return;
    }

    isValidSubModel = true;
    int batchSize = trainParams.batchSize;
    if (size < static_cast<int>(trainParams.batchSize))
    {
        batchSize = size;
        net.reset(new Net<float>());
        net->add(new Dense<float, 2>(batchSize, 1, 1, true, InitializationScheme::GlorotNormal));
    }

    net->registerOptimizer(new Adam<float>(trainParams.learningRate));

    Eigen::Tensor<float, 2> input(batchSize, 1);
    Eigen::Tensor<float, 2> positions(batchSize, 1);
    HuberLoss<float, 2> lossFunction;

    for (int epoch = 0; epoch < trainParams.maxNumEpochs; epoch++)
    {
        auto newBatch = getRandomBatch<KeyType>(batchSize, size);
        int i = 0;
        // put random batch data into tensor
        for (auto idx : newBatch)
        {
            input(i, 0) = static_cast<float>(data[idx].first);
            positions(i, 0) = static_cast<float>(data[idx].second);
            i++;
        }

        // get result of the nn
        auto res = net->forward<2, 2>(input);
        res *= res.constant(totalDatasetSize);

        // calculate loss
        auto loss = lossFunction.loss(res, positions);
        auto lossBackward = lossFunction.backward(res, positions);

        lossBackward /= lossBackward.constant(totalDatasetSize);

        net->backward<2>(lossBackward);
        net->step();
    }
    

    Eigen::Tensor<float, 2> testInput(1, 1);
    long maxError = 0;
    maxNegativeError = 0;
    maxPositiveError = 0;
    for (int i = 0; i < size; i++)
    {
        const KeyType &key = data[i].first;
        const size_t &idx = data[i].second;
        testInput(0, 0) = static_cast<float>(key);

        auto res = net->forward<2, 2>(testInput);
        res *= res.constant(totalDatasetSize);

        int predictIdx = static_cast<int>(res(0, 0));
        auto error = static_cast<int>(idx) - predictIdx;

        if (error < maxNegativeError)
            maxNegativeError = error;
        if (error > maxPositiveError)
            maxPositiveError = error;
        long absError = abs(error);
        if (absError > maxError)
            maxError = absError;
    }

    if (maxError > m_threshold)
    {
        isUseTree = true;
        for (int i = 0; i < data.size(); i++)
        {
            m_tree.insert(data[i]);
        }
    }
    else
    {
        isUseTree = false;
    }

    std::cout << std::endl << "Absolute max error:" << maxError << std::endl;
    std::cout << "Max negative error:" << maxNegativeError << std::endl;
    std::cout << "Max positive error:" << maxPositiveError << std::endl;
    std::cout << "This sub model use tree is : ";
    if(isUseTree)
        std::cout << "True" << std::endl;
    else
        std::cout << "False" << std::endl;
}



template <typename KeyType, typename ValueType, int secondStageSize>
class RMI
{
public:
    RMI(const params &firstStageParams, const params &secondStageParams, int maxSecondStageError)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
        m_maxSecondStageError = maxSecondStageError;

        // init first stage
        m_firstStageNetwork.reset(new Net<float>());
        m_firstStageNetwork->add(new Dense<float, 2>(firstStageParams.batchSize, 1, firstStageParams.neuronNumber, true, InitializationScheme::GlorotNormal));
        m_firstStageNetwork->add(new Relu<float, 2>());
        m_firstStageNetwork->add(new Dense<float, 2>(firstStageParams.batchSize, firstStageParams.neuronNumber, 1, true, InitializationScheme::GlorotNormal));

        // init second stage
        for (int i = 0; i < secondStageSize; i++)
        {
            m_secondStage.emplace_back(SecondStageNode<KeyType>(m_maxSecondStageError, secondStageParams.batchSize));
        }
    }

    // insert
    void insert(KeyType key, ValueType value)
    {
        m_data.push_back({key, value});
    }

    void sortData()
    {
        std::sort(m_data.begin(), m_data.end(), [](std::pair<KeyType, ValueType> p1, std::pair<KeyType, ValueType> p2) {
            return p1.first < p2.first;
        });
    }

    boost::optional<std::pair<KeyType, ValueType>> find(KeyType key);

    void train();

private:
    std::vector<std::pair<KeyType, ValueType>> m_data;
    params m_firstStageParams;
    params m_secondStageParams;
    std::unique_ptr<Net<float>> m_firstStageNetwork;     
    std::vector<SecondStageNode<KeyType>> m_secondStage; 
    int m_maxSecondStageError;
};

template <typename KeyType, typename ValueType, int secondStageSize>
boost::optional<std::pair<KeyType, ValueType>> RMI<KeyType, ValueType, secondStageSize>::find(KeyType key)
{
    Eigen::Tensor<float, 2> input(1, 1);
    input(0, 0) = static_cast<float>(key);

    auto res = m_firstStageNetwork->forward<2, 2>(input);
    auto resIdx = res * res.constant(m_data.size());

    int subModel = static_cast<int>(res(0, 0) * secondStageSize);
    subModel = std::max(0, subModel);
    subModel = std::min(secondStageSize - 1, subModel);

    if (m_secondStage[subModel].isValid())
    {
        if (m_secondStage[subModel].useBtree())
        {
            auto res = m_secondStage[subModel].treeFind(key);
            return {key, m_data[res.get().second]};
        }
        else
        {
            int preIdx = m_secondStage[subModel].predict(key, m_data.size());
            int start = std::max(0, preIdx + static_cast<int>(m_secondStage[subModel].getMaxNegativeError()));
            int end = std::min(static_cast<int>(m_data.size()) - 1, static_cast<int>(preIdx + m_secondStage[subModel].getMaxPositiveError()));

            int res = -1;
            while(start < end)
            {
                int mid = (start + end) / 2;
                if(m_data[mid].first == key)
                {
                    res = mid;
                    break;
                }
                else if(m_data[mid].first > key)
                    end = mid;
                else
                    start = mid + 1;
            }

            if(res != -1)
                return std::pair<KeyType, ValueType>{key, static_cast<int>(m_data[res].second)};
            else
                return {};
        }
    }
    else
    {
        std::cerr << key << " requested an invalid sub model" << std::endl;
        return {};
    }
}

template <typename KeyType, typename ValueType, int secondStageSize>
void RMI<KeyType, ValueType, secondStageSize>::train()
{
    // train stage one
    std::cout << "train stage one" << std::endl;
    HuberLoss<float, 2> lossFunction;
    m_firstStageNetwork->registerOptimizer(new Adam<float>(m_firstStageParams.learningRate));
    Eigen::Tensor<float, 2> input(m_firstStageParams.batchSize, 1);
    Eigen::Tensor<float, 2> positions(m_firstStageParams.batchSize, 1);

    for (int epoch = 0; epoch < m_firstStageParams.maxNumEpochs; epoch++)
    {
        auto newBatch = getRandomBatch<KeyType>(m_firstStageParams.batchSize, m_data.size());
        int i = 0;
        for (auto idx : newBatch)
        {
            input(i, 0) = static_cast<float>(m_data[idx].first);
            positions(i, 0) = static_cast<float>(idx);
            i++;
        }

        auto res = m_firstStageNetwork->forward<2, 2>(input);
        res *= res.constant(m_data.size());

        auto loss = lossFunction.loss(res, positions);
        std::cout << "Epoch: " << epoch << "\t Loss: " << loss << std::endl;

        auto lossBack = lossFunction.backward(res, positions);
        lossBack /= lossBack.constant(m_data.size());

        m_firstStageNetwork->backward<2>(lossBack);
        m_firstStageNetwork->step();
    }


    // train stage two
    std::array<std::vector<std::pair<KeyType, size_t>>, secondStageSize> perSubModelDataset;
    Eigen::Tensor<float, 2> predictInput(1, 1);
    for(int i=0;i<m_data.size();i++)
    {
        predictInput(0, 0) = static_cast<float>(m_data[i].first);

        auto res = m_firstStageNetwork->forward<2, 2>(predictInput);
        auto resIdx = res * res.constant(m_data.size());

        int subModel = static_cast<int>(res(0, 0) * secondStageSize);
        subModel = std::max(0, subModel);
        subModel = std::min(secondStageSize -1, subModel);

        perSubModelDataset[subModel].push_back({m_data[i].first, i});
    }

    std::cout<<"train stage two"<<std::endl;
    for(int i=0;i<secondStageSize;i++)
    {
        m_secondStage[i].train(perSubModelDataset[i], m_secondStageParams, m_data.size());
    }
}
#endif