@echo off
REM ============================================================================
REM Git Setup Script for Intel GPU Demand Paging Demo
REM This script will initialize git and prepare for GitHub upload
REM ============================================================================

echo.
echo ========================================
echo Intel GPU Demand Paging - Git Setup
echo ========================================
echo.

REM Step 1: Add all files
echo [1/5] Adding files to git...
git add .gitignore
git add CMakeLists.txt
git add README_NEW.md
git add TEST_RESULTS.md
git add GIT_SETUP.md
git add src\*.cpp
git add src\CMakeLists.txt
git add .vscode\tasks.json
git add .vscode\launch.json

echo.
echo [2/5] Checking git status...
git status

echo.
echo [3/5] Creating initial commit...
git commit -m "Initial commit: Intel GPU Demand Paging demonstration" -m "- Demand paging stress test (60GB on 16GB GPU)" -m "- GPU capability query tool" -m "- USM and Buffer examples" -m "- Comprehensive test results" -m "- VS Code build configurations"

echo.
echo ========================================
echo Git repository initialized!
echo ========================================
echo.
echo Next steps:
echo.
echo Option A - Using GitHub CLI (Recommended):
echo   gh auth login
echo   gh repo create intel-gpu-demand-paging --public --source=. --push
echo.
echo Option B - Manual setup:
echo   1. Create repo at: https://github.com/new
echo   2. Name: intel-gpu-demand-paging
echo   3. Then run:
echo      git remote add origin https://github.com/jlee52tw/intel-gpu-demand-paging.git
echo      git branch -M main
echo      git push -u origin main
echo.
echo See GIT_SETUP.md for detailed instructions.
echo.

pause
