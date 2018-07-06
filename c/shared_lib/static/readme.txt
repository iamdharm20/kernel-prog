ihttp://www.tenouk.com/ModuleW.html
http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
compiler stages:

	preprocessing	: include headers, Expand Macros => input : .c, .h, .cpp and output : .i and .ii
	compilation	: Genearte Assembly code	=> input : .i , output : .s
	assemble	: Genrate Machine code => input : .S , output : .o and .obj command : as
	linking		: static library =>  input : .lib, .a,  output : .out, .exe and command: ld
	

gcc -E <file>.c				Preprocessor, but don't compile
gcc -S					Compile but don't assemble
gcc -c  <file>.c			assemble but don't link
gcc -o <executable>.out <file>.c	link your object files and generate the executable

Eg.
gcc -E -o linking_error.I linking_error.c
gcc -S -o linking_error.s linking_error.c
gcc -c linking_error.c
gcc -o linking_error.out -c linking_error.c


Note:
gcc -Wall -I/sourcepath/ -L/opt/lib sourcefile -llibrary -o outputfile
-I sourcespath.
-l heymath which tells the linker to link the object files contained in lib<library>.a
-L option tells the linker to search path for libraries

static library creation:
	Example:
	gcc -Wall -c check_endianess.c little_to_bigindian.c 
	ar -cvq libindian.a check_endianess.o little_to_bigindian.o
	gcc -o indian.out little_big_indian.c libindian.a
	./indian.out

ar: list object files in archive library
	ar tf libindian.a 

dynamic library creation:
	Example:
	gcc -Wall -fpic -c little_to_bigindian.c check_endianess.c 
	gcc -shared -Wl,-soname,libindian.so.1 -o libindian.so.1.0 *.o
	gcc -Wall -L . little_big_indian.c -lindian -o indian.out
	./indian.out 

compiler options:

 -Wall: include warnings. See man page for warnings specified.
 -fPIC: Compiler directive to output position independent code, a characteristic required by shared libraries.
 -shared: Produce a shared object which can then be linked with other objects to form an executable.
 -Wl,options: Pass options to linker.

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/lib

List Dependencies of shared library:

	ldd libindian.so.1.0

nm: list symbols: object files, archive library and shared library:

	nm little_to_bigindian.o

A 	The symbol's value is absolute, and will not be changed by further linking.
B 	Un-initialized data section
D 	Initialized data section
T 	Normal code section
U 	Undefined symbol used but not defined. Dependency on another library.
W 	Doubly defined symbol. If found, allow definition in another library to resolve dependency.

readelf: list symbols in shared library: 
	readelf -s libindian.so.1.0

ld - Linker
ldconfig - configure dynamic linker run-time bindings
	/etc/ld.so.conf

little_big_indian_sharedlib.c:
	gcc -o  indian.out little_big_indian_sharedlib.c -ldl
	dlopen() - gain access to an executable object file
	dclose() - close a dlopen object
	dlsym() - obtain the address of a symbol from a dlopen object
	dlvsym() - Programming interface to dynamic linking loader.
	dlerror() - get diagnostic information

objdump : objdump is a program for displaying various information about object files.
	

addr2linei : convert addresses into file names and line numbers.
	addr2line -f -e a.out 0x4005BDC
