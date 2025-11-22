//==============================================================
// Intel GPU Capability Query - PRI/ODP Detection
// This tool queries Intel GPU capabilities including:
// - On-Demand Paging support
// - Unified Shared Memory features
// - Memory management capabilities
//==============================================================
#include <sycl/sycl.hpp>
#include <iostream>
#include <iomanip>

using namespace sycl;
using namespace std;

void print_device_info(const device& dev) {
    cout << "=== Intel GPU Capability Analysis ===\n\n";
    
    cout << "Device Name: " << dev.get_info<info::device::name>() << "\n";
    cout << "Vendor: " << dev.get_info<info::device::vendor>() << "\n";
    cout << "Driver Version: " << dev.get_info<info::device::driver_version>() << "\n\n";
    
    // Memory Information
    cout << "=== Memory Capabilities ===\n";
    auto global_mem = dev.get_info<info::device::global_mem_size>();
    auto local_mem = dev.get_info<info::device::local_mem_size>();
    auto max_alloc = dev.get_info<info::device::max_mem_alloc_size>();
    
    cout << "Global Memory: " << (global_mem / (1024.0*1024*1024)) << " GB\n";
    cout << "Local Memory: " << (local_mem / 1024.0) << " KB\n";
    cout << "Max Single Allocation: " << (max_alloc / (1024.0*1024*1024)) << " GB\n\n";
    
    // USM Capabilities
    cout << "=== USM (Unified Shared Memory) Support ===\n";
    
    // Check USM device allocations
    bool usm_device = dev.has(aspect::usm_device_allocations);
    cout << "USM Device Allocations: " << (usm_device ? "✓ YES" : "✗ NO") << "\n";
    
    // Check USM host allocations
    bool usm_host = dev.has(aspect::usm_host_allocations);
    cout << "USM Host Allocations: " << (usm_host ? "✓ YES" : "✗ NO") << "\n";
    
    // Check USM shared allocations (KEY for demand paging!)
    bool usm_shared = dev.has(aspect::usm_shared_allocations);
    cout << "USM Shared Allocations: " << (usm_shared ? "✓ YES" : "✗ NO") << "\n";
    
    // Check atomic operations on shared memory
    bool usm_atomic_shared = dev.has(aspect::usm_atomic_shared_allocations);
    cout << "USM Atomic Shared Operations: " << (usm_atomic_shared ? "✓ YES" : "✗ NO") << "\n\n";
    
    // Page-based Memory Management
    cout << "=== On-Demand Paging Indicators ===\n";
    
    // System USM means memory can be allocated by system and accessed by device
    // This is a strong indicator of demand paging support
    bool usm_system = dev.has(aspect::usm_system_allocations);
    cout << "USM System Allocations: " << (usm_system ? "✓ YES" : "✗ NO") << "\n";
    
    if (usm_shared) {
        cout << "\n[INFO] USM Shared support detected!\n";
        cout << "       This enables:\n";
        cout << "       • Zero-copy between CPU and GPU\n";
        cout << "       • Memory oversubscription (allocate > GPU physical memory)\n";
        cout << "       • Automatic page migration by driver\n";
        cout << "       • PRI (Page Request Interface) for page faults\n";
    }
    
    cout << "\n=== Compute Capabilities ===\n";
    auto max_compute_units = dev.get_info<info::device::max_compute_units>();
    auto max_work_group_size = dev.get_info<info::device::max_work_group_size>();
    
    cout << "Max Compute Units (EUs): " << max_compute_units << "\n";
    cout << "Max Work Group Size: " << max_work_group_size << "\n\n";
    
    // Check if this is an integrated GPU
    cout << "=== Device Type ===\n";
    auto dev_type = dev.get_info<info::device::device_type>();
    cout << "Type: ";
    switch(dev_type) {
        case info::device_type::cpu: cout << "CPU\n"; break;
        case info::device_type::gpu: cout << "GPU\n"; break;
        case info::device_type::accelerator: cout << "Accelerator\n"; break;
        default: cout << "Other\n";
    }
    
    // Check if host unified memory (typical for integrated GPUs)
    auto has_unified_mem = dev.has(aspect::usm_shared_allocations);
    cout << "Likely Integrated (via USM Shared): " << (has_unified_mem ? "✓ YES (iGPU)" : "? Unknown") << "\n\n";
    
    // Additional Intel-specific capabilities
    cout << "=== Advanced Features ===\n";
    cout << "64-bit Atomics: " << (dev.has(aspect::atomic64) ? "✓ YES" : "✗ NO") << "\n";
    cout << "Queue Profiling: " << (dev.has(aspect::queue_profiling) ? "✓ YES" : "✗ NO") << "\n";
}

int main() {
    try {
        // Query all available devices
        auto platforms = platform::get_platforms();
        
        cout << "Found " << platforms.size() << " platform(s)\n\n";
        
        for (auto& platform : platforms) {
            auto devices = platform.get_devices();
            
            cout << string(60, '=') << "\n";
            cout << "Platform: " << platform.get_info<info::platform::name>() << "\n";
            cout << "Vendor: " << platform.get_info<info::platform::vendor>() << "\n";
            cout << "Version: " << platform.get_info<info::platform::version>() << "\n";
            cout << string(60, '=') << "\n\n";
            
            for (auto& dev : devices) {
                print_device_info(dev);
                cout << "\n" << string(60, '-') << "\n\n";
            }
        }
        
        // Test with default device
        cout << "\n" << string(60, '=') << "\n";
        cout << "DEFAULT DEVICE SELECTION\n";
        cout << string(60, '=') << "\n\n";
        
        device default_dev{default_selector_v};
        print_device_info(default_dev);
        
    } catch (std::exception const& e) {
        cerr << "Exception: " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}
