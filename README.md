# Rest++

My goal with this project is to provide an easy library for restfull services in C++. 
I am not found of the current solutions hiding complexity away through several macros. 
The code itself should be simple and the complexity whould be in the hidden object and classes 
managed by the library.


### Compiling

You will need conan and cmake to build this project

    mkdir build
    cd build
    conan instal .. 
    cmake ..
    make 
    ctest