#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <array>
#include <random>
#include <algorithm>
#include <iostream>

template <int Length>
std::array<int, Length> getData(float maxValue)
{
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);

    std::array<double, Length> dataset;
    for (int i = 0; i < Length; i++)
    {
        dataset[i] = distribution(generator);
    }
    std::sort(dataset.begin(), dataset.end());

    double maxV = dataset[dataset.size() - 1];
    double factor = maxValue / maxV;
    std::array<int, Length> returnValues;
    for (int i = 0; i < dataset.size(); i++)
    {
        returnValues[i] = static_cast<int>(dataset[i] * factor);
    }

    return returnValues;
}

#endif