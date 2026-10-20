@echo off
title MYCELIUM installer
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-mycelium.ps1"
pause
