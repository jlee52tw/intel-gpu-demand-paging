# ============================================================================
# Git Setup Script for Intel GPU Demand Paging Demo (PowerShell)
# ============================================================================

Write-Host ""
Write-Host "========================================"
Write-Host "Intel GPU Demand Paging - Git Setup"
Write-Host "========================================"
Write-Host ""

# Step 1: Add files
Write-Host "[1/5] Adding files to git..." -ForegroundColor Cyan
git add .gitignore
git add CMakeLists.txt
git add README_NEW.md
git add TEST_RESULTS.md
git add GIT_SETUP.md
git add src/*.cpp
git add src/CMakeLists.txt
git add .vscode/tasks.json
git add .vscode/launch.json

Write-Host ""
Write-Host "[2/5] Checking git status..." -ForegroundColor Cyan
git status

Write-Host ""
Write-Host "[3/5] Creating initial commit..." -ForegroundColor Cyan
git commit -m "Initial commit: Intel GPU Demand Paging demonstration" `
           -m "- Demand paging stress test (60GB on 16GB GPU)" `
           -m "- GPU capability query tool" `
           -m "- USM and Buffer examples" `
           -m "- Comprehensive test results" `
           -m "- VS Code build configurations"

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Git repository initialized!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host ""
Write-Host "Option A - Using GitHub CLI (Recommended):" -ForegroundColor White
Write-Host "  gh auth login" -ForegroundColor Gray
Write-Host "  gh repo create intel-gpu-demand-paging --public --source=. --push" -ForegroundColor Gray
Write-Host ""
Write-Host "Option B - Manual setup:" -ForegroundColor White
Write-Host "  1. Create repo at: https://github.com/new" -ForegroundColor Gray
Write-Host "  2. Name: intel-gpu-demand-paging" -ForegroundColor Gray
Write-Host "  3. Then run:" -ForegroundColor Gray
Write-Host "     git remote add origin https://github.com/jlee52tw/intel-gpu-demand-paging.git" -ForegroundColor DarkGray
Write-Host "     git branch -M main" -ForegroundColor DarkGray
Write-Host "     git push -u origin main" -ForegroundColor DarkGray
Write-Host ""
Write-Host "See GIT_SETUP.md for detailed instructions." -ForegroundColor Yellow
Write-Host ""

Read-Host -Prompt "Press Enter to continue"
