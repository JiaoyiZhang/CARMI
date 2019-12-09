#ifndef LAYER_H
#define LAYER_H

#include "weightInitializer.h"
#include "optimizer.h"

template <typename type = float, int Dims = 2>
class Layer {
public:
    virtual const std::string& getName() = 0;
    virtual Eigen::Tensor<type, Dims> forward(const Eigen::Tensor<type, Dims> &input) = 0;
    virtual Eigen::Tensor<type, Dims> backward(const Eigen::Tensor<type, Dims> &output) = 0;
    virtual void step() = 0;
    virtual void registerOptimizer(std::shared_ptr<Adam<type>> optimizer) = 0;
};

template <typename type = float, int Dims = 2>
class Relu : public Layer<type, Dims> 
{
public:
    Relu() = default;

    const std::string &getName()
    {
        const static std::string name = "Relu";
        return name;
    }
    Eigen::Tensor<type, Dims> forward(const Eigen::Tensor<type, Dims> &input)
    {
        m_output = input.cwiseMax(static_cast<type>(0));
        return m_output;
    }

    Eigen::Tensor<type, Dims> backward(const Eigen::Tensor<type, Dims> &accumulatedGrad)
    {
        auto inputPositive = m_output > static_cast<type>(0);
        return inputPositive.select(accumulatedGrad, accumulatedGrad.constant(0.0));
    }

    void step() {}

    void registerOptimizer(std::shared_ptr<Adam<type>> optimizer) {}

private:
    Eigen::Tensor<type, Dims> m_output;
};

template <typename type = float, int Dims = 2>
class Dense : public Layer<type, Dims> 
{
public:
    explicit Dense(int batchSize, int inputDimension, int outputDimension, bool useBias,
          InitializationScheme weightInitializer = InitializationScheme::GlorotUniform)
        : m_outputShape({batchSize, outputDimension}), m_useBias(useBias)
    {
        m_weights = getRandomWeights<type>(inputDimension, outputDimension, weightInitializer);

        m_weightsGrad = Eigen::Tensor<type, Dims>(inputDimension, outputDimension);
        m_weightsGrad.setZero();

        if (useBias)
        {
            m_bias = getRandomWeights<type>(1, outputDimension, weightInitializer);

            m_biasGrad = Eigen::Tensor<type, Dims>(1, outputDimension);
            m_biasGrad.setZero();
        }
    }

    const std::string &getName()
    {
        const static std::string name = "Dense";
        return name;
    }

    Eigen::Tensor<type, Dims> forward(const Eigen::Tensor<type, Dims> &input)
    {
        assert(input.dimensions()[1] == m_weights.dimensions()[0] &&
               "Dense::forward dimensions of input and weights do not match");
        m_inputCache = input;

        Eigen::array<Eigen::IndexPair<int>, 1> productDims = {Eigen::IndexPair<int>(1, 0)};
        auto result = input.contract(m_weights, productDims);

        if (m_useBias)
        {
            // Copy the bias from (1, outputSize) to (inputBatchSize, outputDimension)
            return result + m_bias.broadcast(Eigen::array<Eigen::Index, 2>{input.dimensions()[0], 1});
        }
        else
        {
            return result;
        }
    }

    Eigen::Tensor<type, Dims> backward(const Eigen::Tensor<type, Dims> &accumulatedGrad)
    {
        assert(accumulatedGrad.dimensions()[0] == m_inputCache.dimensions()[0] &&
               "Dense::backward dimensions of accumulatedGrad and inputCache do not match");
        static const Eigen::array<Eigen::IndexPair<int>, 1> transposeInput = {Eigen::IndexPair<int>(0, 0)};

        m_weightsGrad = m_inputCache.contract(accumulatedGrad, transposeInput);
        if (m_useBias)
        {
            m_biasGrad = accumulatedGrad.sum(Eigen::array<int, 1>{0}).eval().reshape(Eigen::array<Eigen::Index, 2>{1, m_outputShape[1]});
        }
        
        static const Eigen::array<Eigen::IndexPair<int>, 1> transposeWeights = {Eigen::IndexPair<int>(1, 1)};
        return accumulatedGrad.contract(m_weights, transposeWeights);
    }

    Eigen::array<Eigen::Index, Dims> getOutputShape()
    {
        return m_outputShape;
    }

    void step()
    {
        m_weights -= m_weightOptimizer->weightUpdate(m_weightsGrad);

        if (m_useBias)
        {
            m_bias -= m_biasOptimizer->weightUpdate(m_biasGrad);
        }
    }

    void registerOptimizer(std::shared_ptr<Adam<type>> optimizer)
    {
        m_weightOptimizer = std::move(optimizer->template createOptimizer<Dims>());

        if (m_useBias)
        {
            m_biasOptimizer = std::move(optimizer->template createOptimizer<Dims>());
        }
    }

private:
    Eigen::array<Eigen::Index, Dims> m_outputShape;
    Eigen::Tensor<type, Dims> m_inputCache;
    Eigen::Tensor<type, Dims> m_weights;
    Eigen::Tensor<type, Dims> m_bias;

    // Gradients
    Eigen::Tensor<type, Dims> m_weightsGrad;
    Eigen::Tensor<type, Dims> m_biasGrad;
    std::unique_ptr<AdamImpl<type, Dims>> m_weightOptimizer;
    std::unique_ptr<AdamImpl<type, Dims>> m_biasOptimizer;

    bool m_useBias;
};

#endif