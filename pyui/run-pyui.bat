@echo off
setlocal
cd /d D:\Open\OpenclawGuard\pyui

if not exist .venv (
  echo [OpenclawGuard-PyUI] creating virtual environment...
  python -m venv .venv
)

call .venv\Scripts\activate.bat
if errorlevel 1 (
  echo [OpenclawGuard-PyUI] failed to activate virtual environment
  exit /b 1
)

echo [OpenclawGuard-PyUI] installing dependencies...
python -m pip install -r requirements.txt
if errorlevel 1 (
  echo [OpenclawGuard-PyUI] dependency install failed
  exit /b 1
)

echo [OpenclawGuard-PyUI] launching app...
python main.py
