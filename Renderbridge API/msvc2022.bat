
cd /d %~dp0
cmake -B _build -G "Visual Studio 17 2022"
start _build/RXExt.sln
