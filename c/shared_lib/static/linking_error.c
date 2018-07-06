#include "a.h"
#include "b.h"

/*How Linkers Resolve Global Symbols Defined at Multiple Places?*/
/*
 At compile time, the compiler exports each global symbol to the assembler as either strong or weak, and the assembler encodes 
 this information implicitly in the symbol table of the relocatable object file. Functions and initialized global variables get strong symbols. 
 Uninitialized global variables get weak symbols.
 During linking, a strong symbol can override a weak symbol of the same name. In contrast, two strong symbols that share a name 
 yield a link error during link-time. 
 linkers use the following rules for dealing with multiply defined symbols:
 Rule 1: Multiple strong symbols are not allowed.
 Rule 2: Given a strong symbol and multiple weak symbols, choose the strong symbol.
 Rule 3: Given multiple weak symbols, choose any of the weak symbols.
*/

int main() {
	a = 15;
	b = 25;
	printf("a :%d b:%d", a, b);
	return 0;
}
