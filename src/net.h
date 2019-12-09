#ifndef NET_H
#define NET_H


#include "optimizer.h"
#include "loss.h"
#include "layer.h"
#include <vector>

template <typename type = float>
class Net
{
public:
    Net() = default;

    template <int inDim, int outDim>
    Eigen::Tensor<type, outDim> forward(Eigen::Tensor<type, inDim> input)
    {
        if (m_layers.empty())
        {
            std::cerr << "No layers" << std::endl;
            return{};
        }

        auto currentInput = input;
        for (const auto &layer : m_layers)
        {
            currentInput = layer->forward(currentInput);
        }
        return currentInput;
    }

    template <int labelDim>
    void backward(Eigen::Tensor<type, labelDim> input)
    {
        if (!m_hasOptimizer)
        {
            std::cerr << "No registered optimizer" << std::endl;
            return;
        }

        if (m_layers.empty())
        {
            std::cerr << "No layers" << std::endl;
            return;
        }

        auto accumulatedGrad = input;
        for (auto i = m_layers.rbegin(); i < m_layers.rend(); i++)
        {
            accumulatedGrad = (*i)->backward(accumulatedGrad);
        }
    }

    void registerOptimizer(Adam<type> *optimizer)
    {
        m_hasOptimizer = true;

        std::shared_ptr<Adam<type>> optimizerPtr(optimizer);

        for (auto &layer : m_layers)
        {
            layer->registerOptimizer(optimizerPtr);
        }
    }

    void step()
    {
        for (auto &layer : m_layers)
        {
            layer->step();
        }
    }

    template <int Dims>
    Net<type> &add(std::unique_ptr<Layer<type, Dims>> layer)
    {
        m_layers.push_back(layer);
        return *this;
    }

    template <int Dims>
    Net<type> &add(Dense<type, Dims> *denseLayer)
    {
        m_layers.push_back(std::unique_ptr<Layer<type, Dims>>(denseLayer));
        return *this;
    }

    template <int Dims>
    Net<type> &add(Relu<type, Dims> *reluLayer)
    {
        m_layers.push_back(std::unique_ptr<Layer<type, Dims>>(reluLayer));
        return *this;
    }

private: 
    std::vector<std::unique_ptr<Layer<type>>> m_layers;
    bool m_hasOptimizer;
};

#endif