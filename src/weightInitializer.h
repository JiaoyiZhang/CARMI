#ifndef WEIGHT_INITIALIZER_H
#define WEIGHT_INITIALIZER_H

#include <unsupported/Eigen/CXX11/Tensor>
enum class InitializationScheme
{
    GlorotUniform,
    GlorotNormal
};

template <typename type>
class WeightDistribution
{
public:
    explicit WeightDistribution(InitializationScheme scheme, int fanIn, int fanOut) : m_scheme(scheme),
                                                                                      m_randomNumberGenerator(std::random_device()())
    {
        if (m_scheme == InitializationScheme::GlorotUniform)
        {
            type limit = std::sqrt(6.0 / (fanIn + fanOut));
            m_uniformDist.reset(new std::uniform_real_distribution<type>(-limit, limit));
        }
        else if (m_scheme == InitializationScheme::GlorotNormal)
        {
            type std = std::sqrt(2.0 / (fanIn + fanOut));
            m_normalDist.reset(new std::normal_distribution<type>(0, std));
        }
    }

    type get()
    {
        if (m_scheme == InitializationScheme::GlorotUniform)
        {
            return (*m_uniformDist)(m_randomNumberGenerator);
        }
        else if (m_scheme == InitializationScheme::GlorotNormal)
        {
            return (*m_normalDist)(m_randomNumberGenerator);
        }
        else
        {
            std::cerr << "Tried to draw from distribution that is uninitialized" << std::endl;
            exit(-1);
        }
    }

private:
    InitializationScheme m_scheme;
    std::mt19937 m_randomNumberGenerator;
    std::unique_ptr<std::uniform_real_distribution<type>> m_uniformDist;
    std::unique_ptr<std::normal_distribution<type>> m_normalDist;
};

template <typename type>
Eigen::Tensor<type, 2> getRandomWeights(int inputDimensions, int outputDimensions,
                                        InitializationScheme scheme = InitializationScheme::GlorotUniform)
{
    Eigen::Tensor<type, 2> weights(inputDimensions, outputDimensions);
    weights.setZero();

    auto distribution = WeightDistribution<type>(scheme, inputDimensions, outputDimensions);
    for (unsigned int ii = 0; ii < inputDimensions; ++ii)
    {
        for (unsigned int jj = 0; jj < outputDimensions; ++jj)
        {
            weights(ii, jj) = distribution.get();
        }
    }
    return weights;
};

#endif