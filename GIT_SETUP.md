# Git Setup Instructions for Intel GPU Demand Paging Demo

Follow these steps to create and push this project to your GitHub account.

## Prerequisites

- Git installed on your system
- GitHub account: jlee52tw
- GitHub CLI (gh) or web browser for creating repository

## Step 1: Initialize Local Repository

```bash
cd c:\working\oneapi\simple-add\simple-add

# Initialize git
git init

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: Intel GPU Demand Paging demonstration

- Demand paging stress test (60GB on 16GB GPU)
- GPU capability query tool
- USM and Buffer examples
- Comprehensive test results and documentation
- VS Code build/debug configurations"
```

## Step 2: Create GitHub Repository

### Option A: Using GitHub CLI (Recommended)

```bash
# Login to GitHub (if not already)
gh auth login

# Create repository
gh repo create intel-gpu-demand-paging \
  --public \
  --description "Demonstrates Intel GPU On-Demand Paging with USM - Allocate 60GB on 16GB Arc 140V iGPU" \
  --source=. \
  --remote=origin \
  --push
```

### Option B: Using Web Browser

1. Go to https://github.com/new
2. Fill in:
   - **Repository name**: `intel-gpu-demand-paging`
   - **Description**: `Demonstrates Intel GPU On-Demand Paging with USM - Allocate 60GB on 16GB Arc 140V iGPU`
   - **Visibility**: Public
   - **Don't** initialize with README (we already have one)
3. Click "Create repository"

4. Then link and push:
```bash
git remote add origin https://github.com/jlee52tw/intel-gpu-demand-paging.git
git branch -M main
git push -u origin main
```

## Step 3: Add Topics/Tags

On GitHub repository page:
1. Click "⚙️ Settings"
2. Scroll to "Topics"
3. Add: `intel-gpu`, `sycl`, `oneapi`, `unified-shared-memory`, `demand-paging`, `edge-ai`, `opencl`, `level-zero`

## Step 4: Create Release

```bash
# Tag the release
git tag -a v1.0.0 -m "Release 1.0.0 - Initial demonstration

Tested on Intel Arc 140V (Xe² Battlemage)
- 60GB allocation on 16GB GPU successful
- Demand paging verified
- Complete documentation included"

# Push the tag
git push origin v1.0.0
```

## Step 5: Update Repository Settings (Optional)

On GitHub:
1. **About** section:
   - Add website: Link to Intel oneAPI documentation
   - Add topics as mentioned above

2. **GitHub Pages** (for documentation):
   - Settings → Pages
   - Source: Deploy from main branch
   - Select `/docs` folder if you create one

## Suggested Repository Structure

```
intel-gpu-demand-paging/
├── README.md                 (rename from GITHUB_README.md)
├── TEST_RESULTS.md          ✓ Already created
├── LICENSE                   (add if desired)
├── .gitignore               ✓ Already created
├── CMakeLists.txt           ✓ Exists
├── src/
│   ├── demand-paging-test.cpp
│   ├── gpu-capability-query.cpp
│   ├── simple-add-usm.cpp
│   ├── simple-add-buffers.cpp
│   └── CMakeLists.txt
└── .vscode/
    ├── tasks.json
    └── launch.json
```

## Quick Commands Summary

```bash
# Initialize
cd c:\working\oneapi\simple-add\simple-add
git init
git add .
git commit -m "Initial commit: Intel GPU Demand Paging demonstration"

# Create on GitHub and push
gh repo create intel-gpu-demand-paging --public --source=. --push

# Or manually
git remote add origin https://github.com/jlee52tw/intel-gpu-demand-paging.git
git branch -M main
git push -u origin main

# Tag release
git tag -a v1.0.0 -m "Release 1.0.0"
git push origin v1.0.0
```

## Files to Review Before Pushing

1. ✅ GITHUB_README.md → Rename to README.md
2. ✅ TEST_RESULTS.md → Review test data accuracy
3. ✅ .gitignore → Ensure build artifacts excluded
4. ⚠️ .vscode/ → May contain local paths (check tasks.json)
5. ⚠️ CMakeLists.txt → Remove any local paths

## Recommended README Badge

Add to top of README.md:

```markdown
[![Intel oneAPI](https://img.shields.io/badge/Intel-oneAPI-blue.svg)](https://www.intel.com/content/www/us/en/developer/tools/oneapi/overview.html)
[![SYCL](https://img.shields.io/badge/SYCL-2020-green.svg)](https://www.khronos.org/sycl/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
```

## Next Steps After Push

1. **Create Issues**: Document known limitations or future enhancements
2. **Wiki**: Add detailed architecture diagrams
3. **Discussions**: Enable for Q&A about demand paging
4. **Actions**: Set up CI/CD for automated builds (optional)

---

**Author**: @jlee52tw  
**Project**: Intel GPU Demand Paging Demonstration  
**Date**: November 23, 2025
