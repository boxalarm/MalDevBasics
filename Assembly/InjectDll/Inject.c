#include <stdio.h>
#include <stdint.h> 
#include <stdbool.h>

extern bool Inject(uint32_t pid, const char dllPath[]); // returns bool to indicate success or failure

int main() {
	// Pass the required arguments to our ASM function Inject
	bool injected = Inject(27956, "c:\\code\\SimpleDll.dll");
	
	if (injected)
		printf("[+] Injection was successful!\n");
	else
		printf("[-] Injection failed...");

	return 0;
}
