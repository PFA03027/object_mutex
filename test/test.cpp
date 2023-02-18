
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
	test_class1( int a_arg, int b_arg )
	  : a( a_arg )
	  , b( b_arg )
	{
	}

	int a;
	int b;
};

bool test1( void )
{
	obj_mutex<test_class1> tt;

	{
		auto locked_data = tt.lock_get();

		if ( tt.TEST_is_locked() ) {
			// std::cout << "locked! OK -> this is expected. so, " << std::endl;
		} else {
			std::cout << "not locked! ERROR" << std::endl;
			return false;
		}
	}
	if ( tt.TEST_is_locked() ) {
		std::cout << "locked! ERROR" << std::endl;
		return false;
	} else {
		// std::cout << "not locked! -> this is expected. so, OK" << std::endl;
	}

	return true;
}

bool test2_default_constructor( void )
{
	// constructor check
	obj_mutex<test_class1> tt1;

	if ( tt1.lock_get().ref().a != 10 ) {
		std::cout << "ERROR: constructor check for member variable a" << std::endl;
		return false;
	}
	if ( tt1.lock_get().ref().b != 11 ) {
		std::cout << "ERROR: constructor check for member variable b" << std::endl;
		return false;
	}

	return true;
}

bool test2_constructor_check( void )
{
	// constructor check
	obj_mutex<test_class1> tt2( 1, 2 );

	if ( tt2.lock_get().ref().a != 1 ) {
		std::cout << "ERROR: constructor check for member variable a" << std::endl;
		return false;
	}
	if ( tt2.lock_get().ref().b != 2 ) {
		std::cout << "ERROR: constructor check for member variable b" << std::endl;
		return false;
	}

	return true;
}

bool test3( void )
{
	obj_mutex<test_class1, std::recursive_mutex> tt3;

	{
		auto locked_data1 = tt3.lock_get();
		// std::cout << "test log:" << std::to_string( locked_data1.ref().a ) << std::endl;

		locked_data1.ref().a = 20;

		if ( tt3.TEST_is_locked() ) {
			std::cout << "ERROR: should be possible to lock" << std::endl;
			return false;
		} else {
			// std::cout << "lockable additionally  -> this is expected. so, OK" << std::endl;
		}

		auto locked_data2 = tt3.lock_get();
		if ( locked_data2.ref().a != 20 ) {
			std::cout << "ERROR: should be same value. expected= 20, actual=" << std::to_string( locked_data2.ref().a ) << std::endl;
			return false;
		} else {
			// std::cout << "lockable additionally  -> this is expected. so, OK" << std::endl;
		}
	}
	if ( tt3.TEST_is_locked() ) {
		std::cout << "ERROR: should be possible to lock" << std::endl;
		return false;
	} else {
		// std::cout << "lockable additionally -> this is expected. so,  OK" << std::endl;
	}

	return true;
}

int main( void )
{
	int exit_code = EXIT_SUCCESS;
	if ( !test1() ) {
		exit_code = EXIT_FAILURE;
	}
	if ( !test2_default_constructor() ) {
		exit_code = EXIT_FAILURE;
	}
	if ( !test2_constructor_check() ) {
		exit_code = EXIT_FAILURE;
	}
	if ( !test3() ) {
		exit_code = EXIT_FAILURE;
	}

	if ( exit_code == EXIT_SUCCESS ) {
		std::cout << "All tests are OK" << std::endl;
	} else {
		std::cout << "Any test/s are NG" << std::endl;
	}
	return exit_code;
}
