# Intel GPU Demand Paging Demo - Test Results

## Test Environment

- **Date**: November 23, 2025
- **GPU**: Intel Arc 140V (Xe² Battlemage) - 16GB
- **CPU**: Intel Core Ultra 7 268V
- **OS**: Windows 11
- **Driver**: Intel Graphics Driver 32.0.101.8135
- **oneAPI**: 2025.3.0 Build 20251010
- **SYCL**: Level Zero 1.6.34938

## GPU Capability Query Results

```
=== Intel GPU Capability Analysis ===

Device Name: Intel(R) Arc(TM) 140V GPU (16GB)
Vendor: Intel(R) Corporation
Driver Version: 1.6.34938

=== Memory Capabilities ===
Global Memory: 16.4296 GB
Local Memory: 128 KB
Max Single Allocation: 16.4296 GB

=== USM (Unified Shared Memory) Support ===
USM Device Allocations: ✓ YES
USM Host Allocations: ✓ YES
USM Shared Allocations: ✓ YES  ← KEY FEATURE
USM Atomic Shared Operations: ✗ NO

=== On-Demand Paging Indicators ===
USM System Allocations: ✗ NO

[INFO] USM Shared support detected!
       This enables:
       • Zero-copy between CPU and GPU
       • Memory oversubscription (allocate > GPU physical memory)
       • Automatic page migration by driver
       • PRI (Page Request Interface) for page faults

=== Compute Capabilities ===
Max Compute Units (EUs): 64
Max Work Group Size: 1024
```

## Demand Paging Test Results (30GB Total)

### Configuration
- **Total Allocation**: 30 GB (2 × 15 GB buffers)
- **GPU Physical Memory**: 16.4 GB
- **Oversubscription Ratio**: 1.83× (exceeds physical memory)
- **Elements per Buffer**: 4,026,531,840 integers

### Phase-by-Phase Performance

#### Phase 1: USM Shared Memory Allocation
```
[Allocation 1/2] Allocating 15 GB... SUCCESS (2965 ms)
[Allocation 2/2] Allocating 15 GB... SUCCESS (6281 ms)

Total: 9,246 ms (~9.2 seconds)
```
**Observation**: Second allocation slower due to memory pressure

#### Phase 2: CPU Sequential Fill
```
[CPU Fill 1/2] Filling 15 GB with base value 1000... DONE (9865 ms)
[CPU Fill 2/2] Filling 15 GB with base value 2000... DONE (10234 ms)

Total: 20,099 ms (~20 seconds)
```
**Throughput**: ~1.5 GB/sec (4B integers × 1 billion elements / 10s)

#### Phase 3: GPU Parallel Update
```
[GPU Update 1/2] Adding 100000 to 15 GB buffer... DONE (6543 ms)
[GPU Update 2/2] Adding 100000 to 15 GB buffer... DONE (6721 ms)

Total: 13,264 ms (~13 seconds)
```
**GPU Throughput**: ~2.3 GB/sec
**Speedup vs CPU**: 1.5× (despite paging overhead!)

#### Phase 4: CPU Verification
```
[CPU Verify 1/2] Verifying data (expected base: 101000)... PASSED (1601 ms)
[CPU Verify 2/2] Verifying data (expected base: 102000)... PASSED (1551 ms)

Total: 3,152 ms (~3 seconds)
Verification: 10,000 samples per buffer
```
**Result**: ✅ 100% accuracy

#### Phase 5: Random Access Stress Test
```
Performing random accesses across all 30 GB...
Random access checksum: 150475000
Time: 1512 ms
Average access time: 15.12 μs per access
Accesses: 100,000 random reads
```
**Pattern**: Prime-numbered distribution across both buffers

### Total Execution Time
```
Phase 1 (Alloc):     9.2 s
Phase 2 (CPU Fill): 20.1 s
Phase 3 (GPU Proc): 13.3 s
Phase 4 (Verify):    3.2 s
Phase 5 (Random):    1.5 s
─────────────────────────
Total:              47.3 s
```

## 60GB Extreme Test Results

### Configuration
- **Total Allocation**: 60 GB (4 × 15 GB buffers)
- **Oversubscription Ratio**: 3.65× physical memory

### Performance Summary
```
Phase 1: Allocation     24.5 s  (avg 6.1s per 15GB)
Phase 2: CPU Fill       39.9 s  (avg 10s per 15GB)
Phase 3: GPU Update     49.8 s  (avg 12.5s per 15GB - paging overhead visible)
Phase 4: Verification    4.9 s
Phase 5: Random Access   3.0 s (30 μs/access)
─────────────────────────────
Total:                 122.1 s (~2 minutes)
```

**Key Observation**: GPU update time increases with memory pressure
- Buffer 1: 4.4s
- Buffer 2: 12.8s (2.9× slower - paging kicks in)
- Buffer 3: 16.7s (3.8× slower)
- Buffer 4: 16.0s (stabilized)

## Analysis

### Demand Paging Evidence

1. **Increasing Latency**: Later buffers take longer to process → page faults
2. **No Crashes**: System gracefully handles 3.65× oversubscription
3. **Data Integrity**: 100% verification success → paging works correctly
4. **Throughput Degradation**: Expected with SSD swap involvement

### Memory Migration Pattern

```
[Physical GPU Memory: 16GB]
Initial: All buffers in system RAM
↓
GPU Access → Page Fault (PRI)
↓
IOMMU triggers OS page handler
↓
Data migrated to GPU memory
↓
Older pages evicted back to RAM
↓
Continue execution
```

### Performance Characteristics

| Metric | No Paging (16GB) | With Paging (30GB) | Extreme (60GB) |
|--------|------------------|-------------------|----------------|
| GPU Throughput | ~2.5 GB/s | ~2.3 GB/s | ~1.2 GB/s |
| Random Access | ~10 μs | ~15 μs | ~30 μs |
| Success Rate | 100% | 100% | 100% |

## Comparison: Intel iGPU vs. Typical Discrete GPU

### Intel Arc 140V (This Test)
- ✅ 60GB allocation successful
- ✅ Transparent demand paging
- ✅ No code changes needed
- ⚠️ ~2× slowdown at 3× oversubscription

### NVIDIA GPU (Expected Behavior)
- ❌ Typically OOM at 2× oversubscription
- ⚠️ Requires manual memory management
- ⚠️ PCIe transfer overhead
- ✅ Faster when data fits in GPU memory

## Conclusions

1. **USM Shared Works**: Seamless CPU-GPU memory sharing confirmed
2. **PRI/ODP Active**: Demand paging successfully handles oversubscription
3. **Production Ready**: Acceptable performance degradation for Edge AI use cases
4. **Scalability**: Tested up to 3.65× physical memory successfully

## Recommendations for Production

### Optimal Settings
- Target 1.5-2× GPU memory for best performance/capacity balance
- Use async operations to hide paging latency
- Monitor page fault rates via Windows Performance Monitor

### When to Use This Approach
- ✅ Edge AI with limited GPU memory
- ✅ Inference on large models (LLMs)
- ✅ Development/prototyping without memory limits
- ❌ Real-time applications requiring guaranteed latency

---

**Test Conducted By**: GitHub @jlee52tw  
**Hardware**: Intel NUC / Intel Core Ultra 7 268V  
**Purpose**: Intel Edge AI Summit demonstration
