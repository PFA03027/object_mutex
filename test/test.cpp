
#include <cstdlib>
#include <iostream>

#include "object_mutex.hpp"

class test_class1 {
public:
	test_class1( void )
	{
		a = 10;
		b = 11;
	}

	int a;

private:
	int b;
};

class test_class2 {
public:
	test_class2( int a_arg, int b_arg )
	  : a( a_arg )
	  , b( b_arg )
	{
	}

	int a;

private:
	int b;
};

void test1( void )
{
	obj_mutex<test_class1> tt;

	{
		auto locked_data = tt.lock_get();
		std::cout << "test log:" << std::to_string( locked_data.ref().a ) << std::endl;

		if ( tt.TEST_is_locked() ) {
			std::cout << "locked! OK" << std::endl;
		} else {
			std::cout << "not locked! ERROR" << std::endl;
		}
	}
	if ( tt.TEST_is_locked() ) {
		std::cout << "locked! ERROR" << std::endl;
	} else {
		std::cout << "not locked! OK" << std::endl;
	}
}

void test2( void )
{
	// constructor check
	obj_mutex<test_class2> tt2( 1, 2 );

	std::cout << "test log:" << std::to_string( tt2.lock_get().ref().a ) << std::endl;
}

void test3( void )
{
	obj_mutex<test_class1, std::recursive_mutex> tt3;

	{
		auto locked_data1 = tt3.lock_get();
		std::cout << "test log:" << std::to_string( locked_data1.ref().a ) << std::endl;

		locked_data1.ref().a = 20;

		if ( tt3.TEST_is_locked() ) {
			std::cout << "ERROR: should be possible to lock" << std::endl;
		} else {
			std::cout << "lockable additionally OK" << std::endl;
		}

		auto locked_data2 = tt3.lock_get();
		std::cout << "test log:" << std::to_string( locked_data2.ref().a ) << std::endl;
	}
	if ( tt3.TEST_is_locked() ) {
		std::cout << "ERROR: should be possible to lock" << std::endl;
	} else {
		std::cout << "lockable additionally OK" << std::endl;
	}
}

int main( void )
{
	test1();
	test2();
	test3();

	return EXIT_SUCCESS;
}
