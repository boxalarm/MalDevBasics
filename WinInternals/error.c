// Enter a system error code that you received from calling GetLastError 
// and call FormatMessage to return useful info about that error code

#include <Windows.h>
#include <stdio.h>

int main() {
    int errorCode;
    printf("Enter the error code from GetLastError: ");

    while (scanf_s("%d", &errorCode) == 1) {
        TCHAR buffer[512] = { 0 };

        DWORD msg = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buffer,
            _countof(buffer),
            NULL
        );

        if (msg > 0) {
            printf("Error %d: %ws\n", errorCode, buffer);
        }
        else {
            printf("No message found for error %u.\n\n", errorCode);
        }
        printf("Enter next error code: ");
    }

    return 0;
}
