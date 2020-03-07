#include <iostream>
#include <assert.h>
#include "../buffer/gap_buffer.h"
#include "../buffer/HierBuffer.h"

using namespace std;

void test_gap_buffer();
void test_HierBuffer();

int main()
{
	test_gap_buffer();
	test_HierBuffer();
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
		buf[100];		//	operator[] は範囲チェックを行わない
	} catch(cchar* ptr) {
		assert(0);
	}
	try {
		buf.at(100);		//	at() は範囲外の場合は例外をスロー
		assert(0);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
	}
//#if sizeof(char*) == 8
#ifdef		_WIN64
	try {
		buf.reserve(0xffffffff);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
		assert(0);
	}
#else
	try {
		buf.reserve(0x7fffffff);
		assert(0);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
	}
#endif
	try {
		buf.reserve(0x40000000);
	} catch(cchar* ptr) {
		assert(0);
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
	for (int i = 0; i < HierBuffer<char>::PAGE_MAX_SZ; ++i) {
		buf.push_back('a'+i%26);
	}
	assert( buf.pageSize() == 1 );
	buf.push_back('Z');
	assert( buf.pageSize() == 2 );
}
