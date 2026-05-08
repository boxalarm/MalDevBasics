#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>

extern size_t AsmStrLen(char* s);
extern char* AsmStrCpy(char* dest, char* src);
extern char* AsmStrCat(char* dest, char* src);

int main() {
	char text[] = "my string";
	char text2[] = "another string";
	char copy[64];
	char asmCopy[64];

	size_t len = strlen(text);
	size_t asmLen = AsmStrLen(text);

	printf("Length (C): %zu\nLength (ASM): %zu\n\n", len, asmLen);

	strcpy(copy, text);
	AsmStrCpy(asmCopy, text);

	printf("Copy (C): %s\nCopy (ASM): %s\n\n", copy, asmCopy);

	printf("strcat (ASM): %s", AsmStrCat(text2, text));

	return 0;
}
