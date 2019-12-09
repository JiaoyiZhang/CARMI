#ifndef LOSS_H
#define LOSS_H

#include <unsupported/Eigen/CXX11/Tensor>

template <typename type, int Dims>
class HuberLoss
{
public:
    HuberLoss(type threshold = 1.0) : m_threshold(threshold) {}

    Eigen::Tensor<type, Dims> backward(const Eigen::Tensor<type, Dims> &predictions, const Eigen::Tensor<type, Dims> &labels);

    type loss(const Eigen::Tensor<type, Dims> &predictions, const Eigen::Tensor<type, Dims> &labels);

private:
    type m_threshold;
    Eigen::Tensor<bool, Dims> m_cachedSwitchResults;
};

template <typename type, int Dims>
type HuberLoss<type, Dims>::loss(const Eigen::Tensor<type, Dims> &predictions, const Eigen::Tensor<type, Dims> &labels)
{
    assert(predictions.dimensions()[0] == labels.dimensions()[0] && "HuberLoss::loss dimensions don't match");
    assert(predictions.dimensions()[1] == labels.dimensions()[1] && "HuberLoss::loss dimensions don't match");
    int batchSize = predictions.dimensions()[0];
    // Definition taken from: https://en.wikipedia.org/wiki/Huber_loss

    // Precalculate y_hat - y
    auto error = predictions - labels;
    auto absoluteError = error.abs();

    // Set up our switch statement and cache it
    m_cachedSwitchResults = absoluteError <= m_threshold;

    // Calculate both terms for the huber loss
    auto lessThanThreshold = error.constant(0.5) * error.square();
    auto moreThanThreshold = error.constant(m_threshold) * absoluteError - error.constant(0.5 * pow(m_threshold, 2));

    // If abs(y_hat - y) <= threshold
    auto perItemLoss = m_cachedSwitchResults.select(
        lessThanThreshold,  // Then use 0.5 * (y_hat - y)^2
        moreThanThreshold); // Else use thresh * |y_hat - y| - (0.5 * threshold^2)

    Eigen::Tensor<type, 0> sum = perItemLoss.sum();

    // Sum and divide by N
    return sum(0) / batchSize;
}

template <typename type, int Dims>
Eigen::Tensor<type, Dims> HuberLoss<type, Dims>::backward(const Eigen::Tensor<type, Dims> &predictions,
                                                            const Eigen::Tensor<type, Dims> &labels)
{

    auto error = predictions - labels;

    auto errorPositiveOrZero = error >= static_cast<type>(0);
    auto absoluteErrorGrad = errorPositiveOrZero.select(error.constant(m_threshold), error.constant(-m_threshold));
    
    return m_cachedSwitchResults.select(error, absoluteErrorGrad);
}

#endif