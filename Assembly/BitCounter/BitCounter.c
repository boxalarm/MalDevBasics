#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

extern int BitCount(uint64_t num);

int CountBits(uint64_t num) {
	int count = 0;

	while (num) {      // loop iterates as long as num != 0
		if (num & 1) { // check least significant bit and add 1 to count if LSB is 1
			count++;
		}
		num >>= 1;	   // shift all bits right by 1
	}
	return count;
}

int main() {
	printf("Count with ASM: %d\n", BitCount(35));
	printf("Count with C: %d\n", CountBits(35));

	return 0;
}
