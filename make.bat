@echo off

cls

::Always compile the program.
::NOTE - Add additional ".cpp" files after 'main.cpp' and before '-lsfml-graphics'.
::---------------------------------------------- Starting Here \/
g++ -I "C:\SFML-2.5.1\include" -L "c:\SFML-2.5.1\lib" -std=c++17 "main.cpp" "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwutil.cpp" "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwmath.cpp" "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwgen.cpp" -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system -static-libgcc -g -o "out.exe"

::If parameter parsed in is equal to "run" then also run the file.
if "%1"=="-r" (
	goto run
) else (
	goto end
)

::Run the program.
:run
out.exe
goto end

::Terminate this script.
:end

pause