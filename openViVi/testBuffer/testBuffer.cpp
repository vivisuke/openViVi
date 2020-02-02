#include <assert.h>
#include <iostream>
#include "../bufffer/gap_buffer.h"

void test_gap_buffer();

int main()
{
	test_gap_buffer();
	//
    std::cout << "OK\n";
}
void test_gap_buffer()
{
	gap_buffer<char> gb;
	assert( gb.isEmpty() );
	assert( gb.empty() );
	assert( gb.size() == 0 );
	//
	gb.insert(0, 'a');
	assert( !gb.isEmpty() );
	assert( gb.size() == 1 );
	assert( gb[0] == 'a' );
	gb.insert(0, 'x');
	assert( gb.size() == 2 );
	assert( gb[0] == 'x' );
	assert( gb[1] == 'a' );
}
