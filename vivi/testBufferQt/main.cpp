//#include <QtCore/QCoreApplication>
#include <iostream>
#include <chrono>
#include <random>
#include <assert.h>
#include <exception>
#include "../buffer/gap_buffer.h"
#include "../buffer/gap_buffer_MT.h"
#include "../buffer/HierBuffer.h"

using namespace std;

#define		SZ			(1024*1024*1024)
#define		GAP		256

random_device g_rd;
#if	1
mt19937 g_mt(g_rd());
#else
mt19937 g_mt(0);
#endif

void test_gap_buffer();
void test_gap_buffer_MT();
void test_HierBuffer();
void time_gap_buffer();

int main(int argc, char *argv[])
{
	test_gap_buffer();
	test_gap_buffer_MT();
	test_HierBuffer();
	//
	//time_gap_buffer();
	//
	//QCoreApplication a(argc, argv);
	//return a.exec();
	//
	cout << "OK\n";
	return 0;
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
	buf.clear();
	buf.push_back('a');
	buf.push_back('b');
	buf.pop_back();
	assert( !buf.isEmpty() );
	assert( buf.size() == 1 );
	assert( buf[0] == 'a' );
	//
	buf.clear();
	try {
		buf[100];		//	operator[] は範囲チェックを行わない
	} catch(cchar* ptr) {
		assert(0);
	} catch( out_of_range e ) {
		cout << e.what() << "\n";
		assert(0);
	}
	try {
		buf.at(100);		//	at() は範囲外の場合は例外をスロー
		assert(0);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
	} catch( out_of_range e ) {
		cout << e.what() << "\n";
	}
//#if sizeof(char*) == 8
#if	0	//def		_WIN64
	try {
		buf.reserve(0xffffffff);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
		assert(0);
	} catch( out_of_range e ) {
		cout << e.what() << "\n";
	}
#else
	try {
		buf.reserve(0x7fffffff);
		assert(0);
	} catch(cchar* ptr) {
		cout << ptr << "\n";
	} catch( out_of_range e ) {
		cout << e.what() << "\n";
	}
#endif
	try {
		buf.reserve(0x40000000);
	} catch(cchar* ptr) {
		assert(0);
	} catch( out_of_range e ) {
		cout << e.what() << "\n";
		assert(0);
	}
	//
	gap_buffer<char> buf2;
	buf2.reserve(123);
	assert( buf2.capacity() >= 123 );
	assert( buf2.isEmpty() );
	//
	gap_buffer<char> buf3;
	buf3.push_back('a');
	vector<char> vec3;
	vec3.push_back('a');
	const int SIZE = 1024;
	for (int i = 0; i < SIZE; ++i) {
		int ix = g_mt() % vec3.size();
		char ch = g_mt() % 26 + 'a';
		buf3.insert(ix, ch);
		vec3.insert(vec3.begin() + ix, ch);
	}
	assert( vec3.size() == buf3.size() );
	for (int i = 0; i < SIZE; ++i) {
		assert( vec3[i] == buf3[i] );
	}
	//
	buf.clear();
	buf.resize(10, 'a');
	assert( buf.size() == 10 );
	for (int i = 0; i != buf.size(); ++i) {
		assert( buf[i] == 'a' );
	}
	buf.insert(0, 'x');		//	ギャップを先頭に移動
	buf.resize(20, 'b');
	//
	gap_buffer<short> wbuf;
	wbuf.resize(SZ);
}
void test_gap_buffer_MT()
{
	gap_buffer_MT<char> buf;
	assert( buf.isEmpty() );
	assert( buf.empty() );
	assert( buf.size() == 0 );
	//
	buf.push_back('a');
	assert(!buf.isEmpty());
	assert(!buf.empty());
	assert(buf.size() == 1);
	assert(buf.front() == 'a');
	assert(buf.back() == 'a');
	buf.push_back('z');
	assert(!buf.isEmpty());
	assert(!buf.empty());
	assert(buf.size() == 2);
	assert(buf.front() == 'a');
	assert(buf.back() == 'z');
	assert(buf.at(0) == 'a');
	assert(buf.at(1) == 'z');
	assert(buf[0] == 'a');
	assert(buf[1] == 'z');
	buf.clear();
	assert(buf.isEmpty());
	assert(buf.empty());
	assert(buf.size() == 0);
	//
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
	buf.clear();
	buf.push_back('a');
	buf.push_back('b');
	buf.pop_back();
	assert( !buf.isEmpty() );
	assert( buf.size() == 1 );
	assert( buf[0] == 'a' );
	//
	buf.clear();
	buf.insert(0, 'a');
	buf.insert(0, 'b');
	assert( buf.size() == 2 );
	assert( buf[0] == 'b' );
	assert( buf[1] == 'a' );
	buf.erase(0);
	assert( buf.size() == 1 );
	assert( buf[0] == 'a' );
	//
	buf.clear();
	const int kPageMaxSize = HierBuffer<char>::PAGE_MAX_SZ;
	for (int i = 0; i < kPageMaxSize; ++i) {
		buf.push_back('a'+i%26);
	}
	assert( buf.pageSize() == 1 );
	buf.push_back('Z');
	assert( buf.pageSize() == 2 );
	assert( buf.m_curPage == 1 );
	assert( buf.m_curFront == kPageMaxSize );
	assert( buf.m_curFront == buf.m_buffer[0]->size() );
	//
	HierBuffer<char> buf2;
	buf2.reserve(123);
	assert( buf2.capacity() >= 123 );
	assert( buf2.isEmpty() );
	buf2.reserve(kPageMaxSize*2);
	assert( buf2.capacity() >= kPageMaxSize*2 );
	assert( buf2.isEmpty() );
	//
	HierBuffer<char> buf3;
	buf3.push_back('a');
	vector<char> vec3;
	vec3.push_back('a');
	const int SIZE = 1024;
	for (int i = 0; i < SIZE; ++i) {
		int ix = g_mt() % vec3.size();
		char ch = g_mt() % 26 + 'a';
		buf3.insert(ix, ch);
		vec3.insert(vec3.begin() + ix, ch);
	}
	assert( vec3.size() == buf3.size() );
	for (int i = 0; i < SIZE; ++i) {
		assert( vec3[i] == buf3[i] );
	}
	//
}
void time_gap_buffer()
{
	if( false ) {
		cout << "push_back():\n";
		gap_buffer<char> buf;
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < SZ; ++i)
			buf.push_back((char)i);
		auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
		auto dur = end - start;        // 要した時間を計算
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();	
		cout << msec << "msec\n\n";
	}
	if( false ) {
		cout << "push_front():\n";
		gap_buffer<char> buf;
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < SZ; ++i)
			buf.insert(0, (char)i);
		auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
		auto dur = end - start;        // 要した時間を計算
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();	
		cout << msec << "msec\n\n";
	}
	if( true ) {
		cout << "pop_back() 1024 times:\n";
		gap_buffer<char> buf;
		buf.resize(SZ);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < 1024; ++i)
			buf.pop_back();
		auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
		auto dur = end - start;        // 要した時間を計算
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();	
		cout << msec << "msec\n\n";
	}
	if( true ) {
		cout << "pop_back(), pop_front() 10 times:\n";
		gap_buffer<char> buf;
		buf.resize(SZ);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < 10; ++i) {
			buf.pop_back();
			buf.pop_front();
		}
		auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
		auto dur = end - start;        // 要した時間を計算
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();	
		cout << msec << "msec\n\n";
	}
}
