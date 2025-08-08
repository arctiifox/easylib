setlocal
cd /d %~dp0
g++ -static -O3 -std=c++17 -o proof.exe proof.cpp -lstdc++fs