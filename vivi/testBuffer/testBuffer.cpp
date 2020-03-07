﻿#include <iostream>
#include <assert.h>
#include "../buffer/gap_buffer.h"
#include "../buffer/HierBuffer.h"

using namespace std;

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
	gap_buffer<char> buf;
	assert( buf.isEmpty() );
	assert( buf.empty() );
	assert( buf.size() == 0 );
	//
	buf.insert(0, 'a');
	assert( !buf.isEmpty() );
	assert( buf.size() == 1 );
	assert( buf[0] == 'a' );
	buf.insert(0, 'x');
	assert( buf.size() == 2 );
	assert( buf[0] == 'x' );
	assert( buf[1] == 'a' );
	buf.clear();
	assert( buf.isEmpty() );
	assert( buf.empty() );
	assert( buf.size() == 0 );
	//
	try {
		buf[100];
	} catch(cchar* ptr) {
		assert(0);
	}
	try {
		buf.at(100);
		assert(0);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
	}
}
void test_HierBuffer()
{
	HierBuffer<char> buf;
	assert( buf.isEmpty() );
	assert( buf.empty() );
	assert( buf.size() == 0 );
	//
	buf.push_back('a');
	assert( !buf.isEmpty() );
	assert( !buf.empty() );
	assert( buf.size() == 1 );
	assert( buf.front() == 'a' );
	assert( buf.back() == 'a' );
	buf.push_back('z');
	assert( !buf.isEmpty() );
	assert( !buf.empty() );
	assert( buf.size() == 2 );
	assert( buf.front() == 'a' );
	assert( buf.back() == 'z' );
	assert( buf.at(0) == 'a' );
	assert( buf.at(1) == 'z' );
	assert( buf[0] == 'a' );
	assert( buf[1] == 'z' );
	buf.clear();
	assert( buf.isEmpty() );
	assert( buf.empty() );
	assert( buf.size() == 0 );
	//
}
