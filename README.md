Basic doubly linked list implementation I did for cpython & some other associated files - it's not 100% implemented, and I also have some module functions that were just for testing (reversing a string, setting a callable). 

File Details: 

    llist.h --> the header file
    llist.c --> the C file that implements the extension module llist
    setup.py --> the python script that builds the module
    llist.so --> the shared object lib (result of compiling llist.c on linux-x86_64)
    test2.py --> my test python script that uses the module

If you want to compile this, just put all the files in the same folder and run `python setup.py build` then you need the built .so to be in the folder.. so just do `ln -s build/lib.{some string}/llist.so`. (this assumes you have gcc)

If you run the test2 script, you'll see two cases where a linked list performs significantly better than an array backed list (python's standard implementation). 
