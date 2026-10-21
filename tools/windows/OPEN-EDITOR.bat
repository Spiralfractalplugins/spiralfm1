@echo off
title SPIRALFM1 editor
cd /d "%~dp0..\.."
if not exist build\spiralfm1-site\webapp\editor\index.html (
  echo The editor is not built yet: run tools\windows\INSTALL-SPIRALFM1.bat first.
  pause
  exit /b 1
)
start "" http://localhost:8766/webapp/editor/
echo SPIRALFM1 editor: http://localhost:8766/webapp/editor/  (keep this window open while you use it)
python -m http.server 8766 --bind 127.0.0.1 --directory build/spiralfm1-site
pause
