#include <Windows.h>
#include <stdio.h>

extern int Sum(int* data, int count);

int SumArrayC(int* data, int count) {
	int i;
	int sum = 0;
	for (i = 0; i < count; i++) {
		sum += data[i];
	}
	return sum;
}

int main() {
	int numbers[] = { 1, 3, 5 };
	printf("C Sum: %d\n", SumArrayC(numbers, _countof(numbers)));
	printf("ASM Sum: %d\n", Sum(numbers, _countof(numbers)));  // Call ASM function

	return 0;
}
