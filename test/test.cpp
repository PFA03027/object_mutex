
#include <cstdlib>
#include <iostream>

#include "object_mutex.hpp"

#include "gtest/gtest.h"

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

TEST( ObjectMutex, CanCall_default_constructor )
{
	// constructor check
	obj_mutex<test_class1> tt1;

	EXPECT_EQ( 10, tt1.lock_get().ref().a );
	EXPECT_EQ( 11, tt1.lock_get().ref().b );

	return;
}

TEST( ObjectMutex, CanCall_translate_constructor )
{
	// constructor check
	obj_mutex<test_class1> tt1( 1, 2 );

	EXPECT_EQ( 1, tt1.lock_get().ref().a );
	EXPECT_EQ( 2, tt1.lock_get().ref().b );

	return;
}

TEST( ObjectMutex, Locked )
{
	obj_mutex<test_class1> tt;

	{
		auto locked_data = tt.lock_get();
		EXPECT_TRUE( tt.TEST_is_locked() );
	}
	EXPECT_FALSE( tt.TEST_is_locked() );

	return;
}

TEST( ObjectMutex, RecusiveMutexLocked )
{
	obj_mutex<test_class1, std::recursive_mutex> tt3;

	{
		auto locked_data1 = tt3.lock_get();
		// std::cout << "test log:" << std::to_string( locked_data1.ref().a ) << std::endl;

		EXPECT_NE( 20, locked_data1.ref().a );
		locked_data1.ref().a = 20;

		ASSERT_FALSE( tt3.TEST_is_locked() );

		auto locked_data2 = tt3.lock_get();
		EXPECT_EQ( 20, locked_data2.ref().a );
	}
	ASSERT_FALSE( tt3.TEST_is_locked() );

	return;
}

obj_mutex<test_class1>::single_accessor tttt( void )
{
	static obj_mutex<test_class1> tt1;
	return tt1.lock_get();
}

TEST( ObjectMutex, move_constructor_single_accessor )
{
	auto locked_data1 = tttt();
	EXPECT_EQ( 10, locked_data1.ref().a );
	return;
}
