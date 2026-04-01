# Wheel Simulator


## Зависимости
В данном проекте используются следующие библиотеки:
 - OpenGL (минимум core 3.3)
 - GLFW
 - glad
 - Imgui

<br> </br>
Часть из них находится в директории `3rdparty`:
### Imgui    
Исходный код взят из https://github.com/ocornut/imgui .
Коммит - fe5c529 (ветка *docking*)
    
    cd 3rdparty/imgui

### Implot 
Исходный код взят из https://github.com/epezent/implot .
Коммит - bd99f8d

    cd 3rdparty/implot
    
### glad
Исходный код был сгенерирован с помощью glad

    glad --api='gl:core=3.3' --out-path=3rdparty/glad c


Также для библиотек из 3rdparty были убраны некоторые ненужные файлы.

## Сборка
Чтобы собрать проект необходимо установить нужные зависимости, описанные ранее.

### Linux
Для пользователей Linux необходимо выполнить

    git clone https://github.com/dangerUser45/WheelSimulator.git
    cd WheelSimulator
    mkdir -p build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
Для запуска

    build/WheelSimulator
