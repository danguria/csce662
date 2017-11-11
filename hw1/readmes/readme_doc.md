Documentation {#document}
====================
API Documentation is provided inline in the source header files, which use the __doxygen__ standard mark-up.
Run `doxygen` on the `./doxygen-cfg.txt` file located in the library main directory.

    doxygen ./doxygen-cfg.txt

This will produce the documentation in the `./doc/html` and `./doc/latex` directory. If you want to see the html, you can open `./doc/html/index.html`. If you want to convert latex to pdf, run the command as follow:

    cd ./doc/latex
    make

then, *refman.pdf* will be generated.
