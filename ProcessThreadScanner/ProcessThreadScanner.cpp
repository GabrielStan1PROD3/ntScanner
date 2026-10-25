#include "ProcessScanner.hpp"
#include "StringBuilder.hpp";

int main()
{


	HANDLE h   =  ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (h == INVALID_HANDLE_VALUE) return 1;

	Scanner s{};
	s.basic_info();
	const size_t size = s.dumpSize();
	
	DWORD noCharsWritten = 0;
	
	WriteConsoleW(h, s.dump(), size,&noCharsWritten, nullptr);
	if (noCharsWritten != size) return 1;
	return 0;
}