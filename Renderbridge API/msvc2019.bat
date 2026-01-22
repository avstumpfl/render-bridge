
cd /d %~dp0
cmake -B _build -G "Visual Studio 16 2019"
start _build/RXExt.sln
