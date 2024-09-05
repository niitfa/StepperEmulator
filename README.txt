Для сборки требуется: 
Windows: CMake, Visual Studio
Linux: CMake

Сборка и запуск на Windows:
1) Из папки проекта выполнить
mkdir build
cmake -B build
2) Открыть сгенерированный проект VS /build/emulator.sln
3) В VS собрать проект emulator (выбираем решение emulator - ПКМ - Build/Собрать)
4) Запустить файл build/Debug/emulator.exe
5) Передаваемыми значениями управлять через файл message.txt

Сборка и запуск на Linux:
1) Из папки проекта выполнить
cmake .
make
2) Запустить собранный проект
./emulator
3) Передаваемыми значениями управлять через файл message.txt
