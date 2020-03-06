#include <iostream>
#include <assert.h>
#include "../buffer/gap_buffer.h"
#include "../buffer/HierBuffer.h"

void test_gap_buffer();
void test_HierBuffer();

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
void test_HierBuffer()
{
	HierBuffer<char> gb;
	assert( gb.isEmpty() );
	assert( gb.empty() );
	assert( gb.size() == 0 );
}
