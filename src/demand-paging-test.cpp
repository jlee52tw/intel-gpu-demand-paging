//==============================================================
// Demand Paging Test - USM Memory Oversubscription
// This example tests Intel GPU demand paging by allocating more
// memory than physically available on the GPU, forcing the driver
// to page memory between system RAM and GPU memory.
//==============================================================
// Copyright © 2025 Intel Corporation
// SPDX-License-Identifier: MIT
//==============================================================
#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <chrono>

using namespace sycl;
using namespace std;

// Create an exception handler for asynchronous SYCL exceptions
static auto exception_handler = [](sycl::exception_list e_list) {
  for (std::exception_ptr const &e : e_list) {
    try {
      std::rethrow_exception(e);
    }
    catch (std::exception const &e) {
      std::cout << "SYCL Exception: " << e.what() << std::endl;
      std::terminate();
    }
  }
};

// Simple kernel to update data across the allocated memory (GPU operation)
void UpdatePattern(queue &q, int *data, size_t size, int increment) {
  // Split into chunks to avoid int overflow with large ranges
  const size_t chunk_size = 100000000; // 100M elements per kernel launch
  
  for (size_t offset = 0; offset < size; offset += chunk_size) {
    size_t current_chunk = min(chunk_size, size - offset);
    range num_items{current_chunk};
    
    auto e = q.parallel_for(num_items, [=](auto i) { 
      size_t global_idx = offset + i;
      data[global_idx] = data[global_idx] + increment; 
    });
    e.wait();
  }
}

// Verify the data (CPU operation)
bool VerifyPattern(int *data, size_t size, int expected_value, size_t sample_count = 10000) {
  size_t stride = size / sample_count;
  if (stride == 0) stride = 1;
  
  for (size_t i = 0; i < size; i += stride) {
    int expected = expected_value + static_cast<int>(i);
    if (data[i] != expected) {
      cout << "  [ERROR] Mismatch at index " << i 
           << ": expected " << expected 
           << ", got " << data[i] << "\n";
      return false;
    }
  }
  return true;
}

int main() {
  auto selector = gpu_selector_v; //default_selector_v;

  try {
    queue q(selector, exception_handler);

    // Print device information
    auto device = q.get_device();
    cout << "=== Intel GPU Demand Paging Test ===\n";
    cout << "Device: " << device.get_info<info::device::name>() << "\n";
    
    // Query device memory
    auto global_mem_size = device.get_info<info::device::global_mem_size>();
    auto local_mem_size = device.get_info<info::device::local_mem_size>();
    
    cout << "Device Global Memory: " << (global_mem_size / (1024*1024*1024.0)) << " GB\n";
    cout << "Device Local Memory: " << (local_mem_size / 1024.0) << " KB\n\n";

    // Configuration: Allocate 4 buffers of 15GB each = 60GB total
    const size_t GB = 1024ULL * 1024ULL * 1024ULL;
    const size_t allocation_size_gb = 15; //15;
    const size_t num_allocations = 2 ; //4;
    const size_t elements_per_allocation = (allocation_size_gb * GB) / sizeof(int);
    const size_t total_gb = allocation_size_gb * num_allocations;

    cout << "Test Configuration:\n";
    cout << "  Number of allocations: " << num_allocations << "\n";
    cout << "  Size per allocation: " << allocation_size_gb << " GB\n";
    cout << "  Total allocation: " << total_gb << " GB\n";
    cout << "  Elements per allocation: " << elements_per_allocation << " ints\n\n";

    if (total_gb <= (global_mem_size / GB)) {
      cout << "WARNING: Total allocation (" << total_gb << " GB) is less than device memory.\n";
      cout << "         Demand paging may not be triggered.\n\n";
    } else {
      cout << "INFO: Total allocation (" << total_gb << " GB) exceeds device memory.\n";
      cout << "      This will trigger demand paging!\n\n";
    }

    // Vector to hold pointers to our allocations
    vector<int*> allocations;
    allocations.reserve(num_allocations);

    // Phase 1: Allocate all buffers
    cout << "=== Phase 1: Allocating USM Shared Memory ===\n";
    for (size_t i = 0; i < num_allocations; i++) {
      cout << "[Allocation " << (i+1) << "/" << num_allocations << "] ";
      cout << "Allocating " << allocation_size_gb << " GB..." << std::flush;
      
      auto start = chrono::high_resolution_clock::now();
      int *ptr = malloc_shared<int>(elements_per_allocation, q);
      auto end = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
      
      if (ptr == nullptr) {
        cout << " FAILED!\n";
        cout << "ERROR: Could not allocate " << allocation_size_gb << " GB for allocation " << (i+1) << "\n";
        
        // Free previously allocated memory
        for (auto p : allocations) {
          free(p, q);
        }
        return -1;
      }
      
      allocations.push_back(ptr);
      cout << " SUCCESS (" << duration << " ms)\n";
    }
    cout << "\nTotal allocated: " << total_gb << " GB of USM shared memory\n\n";

    // Phase 2: CPU fills data sequentially
    cout << "=== Phase 2: CPU Filling Data (Sequential) ===\n";
    for (size_t i = 0; i < num_allocations; i++) {
      int base_value = 1000 * (i + 1);
      cout << "[CPU Fill " << (i+1) << "/" << num_allocations << "] ";
      cout << "Filling " << allocation_size_gb << " GB with base value " << base_value << "..." << std::flush;
      
      auto start = chrono::high_resolution_clock::now();
      for (size_t j = 0; j < elements_per_allocation; j++) {
        allocations[i][j] = base_value + static_cast<int>(j);
      }
      auto end = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
      
      cout << " DONE (" << duration << " ms)\n";
    }
    cout << "\n";

    // Phase 3: GPU updates the data in parallel
    cout << "=== Phase 3: GPU Updating Data (Parallel on Device) ===\n";
    const int gpu_increment = 100000;
    for (size_t i = 0; i < num_allocations; i++) {
      cout << "[GPU Update " << (i+1) << "/" << num_allocations << "] ";
      cout << "Adding " << gpu_increment << " to " << allocation_size_gb << " GB buffer on GPU..." << std::flush;
      
      auto start = chrono::high_resolution_clock::now();
      UpdatePattern(q, allocations[i], elements_per_allocation, gpu_increment);
      auto end = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
      
      cout << " DONE (" << duration << " ms)\n";
    }
    cout << "\n";

    // Phase 4: CPU verifies the updated data
    cout << "=== Phase 4: CPU Verifying GPU Results ===\n";
    bool all_verified = true;
    
    for (size_t i = 0; i < num_allocations; i++) {
      int expected_value = 1000 * (i + 1) + gpu_increment;
      cout << "[CPU Verify " << (i+1) << "/" << num_allocations << "] ";
      cout << "Verifying data (expected base: " << expected_value << ")..." << std::flush;
      
      auto start = chrono::high_resolution_clock::now();
      bool verified = VerifyPattern(allocations[i], elements_per_allocation, expected_value);
      auto end = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
      
      if (verified) {
        cout << " PASSED (" << duration << " ms)\n";
      } else {
        cout << " FAILED (" << duration << " ms)\n";
        all_verified = false;
      }
    }
    cout << "\n";

    // Phase 5: Random access pattern (stress test for demand paging)
    cout << "=== Phase 5: Random Access Test ===\n";
    cout << "Performing random accesses across all " << total_gb << " GB...\n";
    
    auto start = chrono::high_resolution_clock::now();
    
    // Access random elements from different allocations
    const size_t num_random_accesses = 100000;
    long long sum = 0;
    
    for (size_t i = 0; i < num_random_accesses; i++) {
      size_t alloc_idx = i % num_allocations;
      size_t elem_idx = (i * 997) % elements_per_allocation; // Prime number for better distribution
      sum += allocations[alloc_idx][elem_idx];
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "Random access checksum: " << sum << "\n";
    cout << "Time: " << duration << " ms\n";
    cout << "Average access time: " << (duration * 1000.0 / num_random_accesses) << " μs per access\n\n";

    // Phase 6: Cleanup
    cout << "=== Phase 6: Freeing Memory ===\n";
    for (size_t i = 0; i < num_allocations; i++) {
      cout << "[Free " << (i+1) << "/" << num_allocations << "] ";
      cout << "Freeing " << allocation_size_gb << " GB..." << std::flush;
      
      free(allocations[i], q);
      cout << " DONE\n";
    }
    cout << "\n";

    if (all_verified) {
      cout << "=== TEST PASSED ===\n";
      cout << "Successfully allocated and accessed " << total_gb << " GB across GPU demand paging!\n";
    } else {
      cout << "=== TEST FAILED ===\n";
      cout << "Data verification errors detected.\n";
      return -1;
    }

  } catch (std::exception const &e) {
    cout << "\nException caught: " << e.what() << "\n";
    return -1;
  }

  return 0;
}
