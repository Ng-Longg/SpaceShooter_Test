@echo off
echo === Building SpaceShooter ===

REM Compile all cpp in src and link SFML (MinGW / g++)
g++ src\*.cpp -Iinclude -Lbuild -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o GAME\SpaceShooter.exe -mwindows

if %errorlevel% neq 0 (
    echo === Build failed! ===
    pause
    exit /b
)

echo === Build complete! ===
pause
