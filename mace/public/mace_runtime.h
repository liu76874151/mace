// Copyright 2018 Xiaomi, Inc.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file defines runtime tuning APIs.
// These APIs are not stable.

#ifndef MACE_PUBLIC_MACE_RUNTIME_H_
#define MACE_PUBLIC_MACE_RUNTIME_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "mace/public/mace.h"

namespace mace {

enum GPUPerfHint {
  PERF_DEFAULT = 0,
  PERF_LOW = 1,
  PERF_NORMAL = 2,
  PERF_HIGH = 3
};

enum GPUPriorityHint {
  PRIORITY_DEFAULT = 0,
  PRIORITY_LOW = 1,
  PRIORITY_NORMAL = 2,
  PRIORITY_HIGH = 3
};

enum CPUAffinityPolicy {
  AFFINITY_NONE = 0,
  AFFINITY_BIG_ONLY = 1,
  AFFINITY_LITTLE_ONLY = 2,
};

class KVStorage {
 public:
  // return: 0 for success, -1 for error
  virtual int Load() = 0;
  virtual bool Insert(const std::string &key,
                      const std::vector<unsigned char> &value) = 0;
  virtual const std::vector<unsigned char> *Find(const std::string &key) = 0;
  // return: 0 for success, -1 for error
  virtual int Flush() = 0;
};

class KVStorageFactory {
 public:
  virtual std::unique_ptr<KVStorage> CreateStorage(const std::string &name) = 0;
};

class __attribute__((visibility("default"))) FileStorageFactory
    : public KVStorageFactory {
 public:
  explicit FileStorageFactory(const std::string &path);

  ~FileStorageFactory();

  std::unique_ptr<KVStorage> CreateStorage(const std::string &name) override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// Set KV store factory used as OpenCL cache. (Call Once)
__attribute__((visibility("default")))
void SetKVStorageFactory(std::shared_ptr<KVStorageFactory> storage_factory);

// Just call once. (Not thread-safe)
// Set paths of Generated OpenCL Compiled Kernel Binary file (not libOpenCL.so)
// if you use gpu of specific soc.
// Using OpenCL binary will speed up the initialization.
// OpenCL binary is corresponding to the OpenCL Driver version,
// you should update the binary when OpenCL Driver changed.
__attribute__((visibility("default")))
void SetOpenCLBinaryPaths(const std::vector<std::string> &paths);

// Just call once. (Not thread-safe)
// Set the path of Generated OpenCL parameter file if you use gpu of specific soc.
// The parameters is the local work group size tuned for specific SOC, which
// may be faster than the general parameters.
void SetOpenCLParameterPath(const std::string &path);

// Set GPU hints, currently only supports Adreno GPU.
//
// Caution: this function may hurt performance if improper parameters provided.
__attribute__((visibility("default")))
void SetGPUHints(GPUPerfHint perf_hint, GPUPriorityHint priority_hint);

// Set OpenMP threads number and affinity policy.
//
// Caution: this function may hurt performance if improper parameters provided.
//
// num_threads_hint is only a hint. When num_threads_hint is zero or negative,
// the function will set the threads number equaling to the number of
// big (AFFINITY_BIG_ONLY), little (AFFINITY_LITTLE_ONLY) or all
// (AFFINITY_NONE) cores according to the policy. The threads number will
// also be truncated to the corresponding cores number when num_threads_hint
// is larger than it.
//
// The OpenMP threads will be bind to (via sched_setaffinity) big cores
// (AFFINITY_BIG_ONLY) and little cores (AFFINITY_LITTLE_ONLY).
//
// If successful, it returns MACE_SUCCESS and error if it can't reliabley
// detect big-LITTLE cores (see GetBigLittleCoreIDs). In such cases, it's
// suggested to use AFFINITY_NONE to use all cores.
__attribute__((visibility("default")))
MaceStatus SetOpenMPThreadPolicy(int num_threads_hint,
                                 CPUAffinityPolicy policy);

// Set OpenMP threads number and processor affinity.
//
// Caution: this function may hurt performance if improper parameters provided.
//
// This function may not work well on some chips (e.g. MTK). Setting thread
// affinity to offline cores may run very slow or unexpectedly. In such cases,
// please use SetOpenMPThreadPolicy with default policy instead.
__attribute__((visibility("default")))
void SetOpenMPThreadAffinity(int num_threads, const std::vector<int> &cpu_ids);

// Get ARM big.LITTLE configuration.
//
// This function will detect the max frequencies of all CPU cores, and assume
// the cores with largest max frequencies as big cores, and all the remaining
// cores as little. If all cpu core's max frequencies equals, big_core_ids and
// little_core_ids will both be filled with all cpu core ids.
//
// If successful, it returns MACE_SUCCESS and error if it can't reliabley
// detect the frequency of big-LITTLE cores (e.g. MTK).
__attribute__((visibility("default")))
MaceStatus GetBigLittleCoreIDs(std::vector<int> *big_core_ids,
                               std::vector<int> *little_core_ids);

}  // namespace mace

#endif  // MACE_PUBLIC_MACE_RUNTIME_H_
