#include "main.h"

int main (int argc, char ** argv)
{
	struct fit_file test;
	parse_file("./sample.fit", &test);
	print_fit(stdout, &test);
	return 0;
}
