#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <unsupported/Eigen/CXX11/Tensor>
#include <iostream>
#include <memory>

template <typename type, int Dims>
class AdamImpl
{
public:
    AdamImpl(type learningRate, type beta1, type beta2, type epsilon) : m_learningRate(learningRate), m_beta1(beta1), m_beta2(beta2), m_epsilon(epsilon), m_isInitialized(false), m_currentTimestep(1) {}

    Eigen::Tensor<type, Dims> weightUpdate(const Eigen::Tensor<type, Dims> &gradWeights)
    {
        if (!m_isInitialized)
        {
            m_firstMoment = Eigen::Tensor<type, Dims>(gradWeights.dimensions());
            m_firstMoment.setZero();

            m_secondMoment = Eigen::Tensor<type, Dims>(gradWeights.dimensions());
            m_secondMoment.setZero();
            m_isInitialized = true;
        }

        // m_t = B_1 * m_(t-1) + (1 - B_1) * g_t
        m_firstMoment = m_firstMoment.constant(m_beta1) * m_firstMoment +
                        gradWeights.constant(1 - m_beta1) * gradWeights;

        // v_t = B_2 * v_(t-1) + (1 - B_2) * g_t^2
        m_secondMoment = m_secondMoment.constant(m_beta2) * m_secondMoment +
                         gradWeights.constant(1 - m_beta2) * gradWeights.square();

        auto biasCorrectedFirstMoment = m_firstMoment / m_firstMoment.constant(1 - pow(m_beta1, m_currentTimestep));
        auto biasCorrectedSecondMoment = m_secondMoment / m_secondMoment.constant(1 - pow(m_beta2, m_currentTimestep));

        m_currentTimestep++;

        // Return firstMoment  * (learning_rate) / (sqrt(secondMoment) + epsilon)
        return biasCorrectedFirstMoment * ((gradWeights.constant(m_learningRate) /
                                            (biasCorrectedSecondMoment.sqrt() + gradWeights.constant(m_epsilon))));
    };

private:
    type m_learningRate;
    type m_beta1;
    type m_beta2;
    type m_epsilon;

    bool m_isInitialized;
    size_t m_currentTimestep;
    Eigen::Tensor<type, Dims> m_firstMoment;
    Eigen::Tensor<type, Dims> m_secondMoment;
};


template <typename type>
class Adam
{
public:
    Adam(type learningRate, type beta1 = 0.9, type beta2 = 0.999, type epsilon = 1e-8) : m_learningRate(learningRate), m_beta1(beta1), m_beta2(beta2), m_epsilon(epsilon) {}

    template <int Dims>
    std::unique_ptr<AdamImpl<type, Dims>> createOptimizer() const
    {
        return std::unique_ptr<AdamImpl<type, Dims>>(new AdamImpl<type, Dims>(m_learningRate, m_beta1, m_beta2, m_epsilon));
    };

private:
    type m_learningRate;
    type m_beta1;
    type m_beta2;
    type m_epsilon;
};

template <typename type, int Dims>
class OptimizerImpl
{
public:
    virtual Eigen::Tensor<type, Dims> weightUpdate(const Eigen::Tensor<type, Dims> &weights) = 0;
};

#endif