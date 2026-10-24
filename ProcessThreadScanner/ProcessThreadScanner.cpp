#include "ProcessScanner.hpp"
#include "StringBuilder.hpp";

int main()
{

	//Scanner s{};
	//s.basic_info();
	wchar_t buffer[] = L"Lets go bake in time";
	string_builder sx{};
	for (int i{} ; i < 100'000; ++i)
	{
		sx.append_line(buffer,sizeof(buffer) / sizeof(buffer[0]));
	}


	return 0;
}