#include "ProcessScanner.hpp"
#include "StringBuilder.hpp";

int main()
{

	Scanner s{};
	s.basic_info();
	std::printf("%ws", s.dump());


	return 0;
}