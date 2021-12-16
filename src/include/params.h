/**
 * @file params.h
 * @author Jiaoyi
 * @brief parameters in carmi_params space
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef PARAMS_H_
#define PARAMS_H_

#define DEBUG

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX == 1
#define CATCH_PLATFORM_MAC
#elif TARGET_OS_IPHONE == 1
#define CATCH_PLATFORM_IPHONE
#endif

#elif defined(linux) || defined(__linux) || defined(__linux__)
#define CATCH_PLATFORM_LINUX

#elif defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || \
    defined(_MSC_VER) || defined(__MINGW32__)
#define CATCH_PLATFORM_WINDOWS
#endif

/**
 * @brief These are parameters in the CARMI framework. The first three
 * parameters are the maximum capacity of the cf array leaf node and external
 * array leaf node we have provided in the source code, and the boundary value
 * for switching between the dynamic programming algorithm and the greedy node
 * selection algorithm. Users can change their values according to their actual
 * needs. As for the parameters of the time costs of different nodes, users can
 * use our profiler to obtain them on their machine.
 */
namespace carmi_params {
/**
 * @brief bytes, the size of a data block in cf array leaf nodes.
 * The value must be an integer multiple of the size of the cache line, the
 * reference values are: 64, 128, 256, 512, etc.
 */
static constexpr int kMaxLeafNodeSize = 256;

/**
 * @brief The maximum number of data points in an external leaf node.
 * This value is generally an integer multiple of 2. Since the external dataset
 * is not stored in our index structure, the value can be larger to reduce the
 * space cost. Reference values are 512, 1024, 2048, and so on.
 */
static constexpr int kMaxLeafNodeSizeExternal = 1024;

/**
 * @brief The maximum number of data points which can use the DP algorithm to
 * construct an inner node. If the size of the sub-dataset exceeds this
 * parameter, greedy node selection algorithm is used to construct the inner
 * node.
 * This value needs to be no less than the first two parameters.
 */
static constexpr int kAlgorithmThreshold = 1024;

/**
 * @brief The latency of a memory access
 */
static constexpr double kMemoryAccessTime = 80.09;

/**
 * @brief The time cost of the lr root node including the latency of
 * accessing the cache (8.29 ns) and the CPU time (3.25 ns)
 */
static constexpr double kLRRootTime = 11.54;

/**
 * @brief The time cost of the plr root node including the latency of
 * accessing the cache (11.24 ns) and the CPU time (18.38 ns)
 */
static constexpr double kPLRRootTime = 29.62;

/**
 * @brief The time cost of the lr inner node including the latency of
 * memory access and the CPU time
 */
static constexpr double kLRInnerTime = kMemoryAccessTime + 5.23;

/**
 * @brief The time cost of the plr inner node including the latency of
 * memory access and the CPU time
 */
static constexpr double kPLRInnerTime = kMemoryAccessTime + 24.21;

/**
 * @brief The time cost of the his inner node including the latency of
 * memory access and the CPU time
 */
static constexpr double kHisInnerTime = kMemoryAccessTime + 18.44;

/**
 * @brief The time cost of the bs inner node including the latency of
 * memory access and the CPU time
 */
static constexpr double kBSInnerTime = kMemoryAccessTime + 36.38;

/**
 * @brief The time cost of moving a data point
 */
static constexpr double kCostMoveTime = 6.25;

/**
 * @brief The basic time cost of a leaf node including the latency of accessing
 * the leaf node in the memory and the time cost of searching in the leaf node
 * (25.4 ns) and the data block (53.8 ns)
 */
static constexpr double kLeafBaseTime = kMemoryAccessTime + 25.4 + 53.8;

/**
 * @brief The average time cost of a binary search
 */
static constexpr double kCostBSTime = 10.9438;
}  // namespace carmi_params

#endif  // PARAMS_H_
