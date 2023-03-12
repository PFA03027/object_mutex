
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>

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

TEST( ObjectMutex, Locked )
{
	obj_mutex<test_class1> tt;

	{
		auto locked_data = tt.lock_get();
		EXPECT_TRUE( tt.is_locked() );
	}
	EXPECT_FALSE( tt.is_locked() );

	return;
}

TEST( ObjectMutex, Locked_move1 )
{
	obj_mutex<test_class1> tt11( 11, 12 );

	auto locked_data1 = tt11.lock_get();
	auto locked_data2 = std::move( locked_data1 );

	EXPECT_FALSE( locked_data1.valid() );
	EXPECT_TRUE( locked_data2.valid() );
	EXPECT_EQ( 11, locked_data2.ref().a );

	return;
}

TEST( ObjectMutex, Locked_move2 )
{
	obj_mutex<test_class1> tt11( 11, 12 );
	obj_mutex<test_class1> tt12( 21, 22 );

	auto locked_data1 = tt11.lock_get();
	auto locked_data2 = tt12.lock_get();

	EXPECT_TRUE( locked_data1.valid() );
	EXPECT_TRUE( locked_data2.valid() );
	EXPECT_EQ( 21, locked_data2.ref().a );

	locked_data2 = std::move( locked_data1 );

	EXPECT_FALSE( locked_data1.valid() );
	EXPECT_TRUE( locked_data2.valid() );
	EXPECT_EQ( 11, locked_data2.ref().a );

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

		ASSERT_FALSE( tt3.is_locked() );

		auto locked_data2 = tt3.lock_get();
		EXPECT_EQ( 20, locked_data2.ref().a );
	}
	ASSERT_FALSE( tt3.is_locked() );

	return;
}

typename obj_mutex<test_class1>::single_accessor tttt( void )
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

class test_class2 {
public:
	test_class2( int a_arg, int b_arg )
	  : a( a_arg )
	  , b( b_arg )
	{
	}

private:
	int a;
	int b;
};

TEST( ObjectMutex, CouldNotCall_default_constructor )
{
	// could not call default constructor
	static_assert( !std::is_default_constructible<obj_mutex<test_class2>>::value );

	return;
}

TEST( ObjectMutex, CanCall_move_constructor )
{
	static_assert( std::is_move_constructible<obj_mutex<test_class1>>::value );

	obj_mutex<test_class1> tt1;
	tt1.lock_get().ref().a     = 20;
	obj_mutex<test_class1> tt2 = std::move( tt1 );

	EXPECT_FALSE( tt1.valid() );
	EXPECT_TRUE( tt2.valid() );
	EXPECT_EQ( 20, tt2.lock_get().ref().a );
	return;
}

class test_classA {
public:
	test_classA( int a_arg = 0 )
	  : a( a_arg )
	{
	}

	virtual ~test_classA() = default;

	int a;
};

class test_classB : public test_classA {
public:
	test_classB( int b_arg = 0 )
	  : test_classA()
	  , b( b_arg )
	{
	}

	int b;
};

TEST( ObjectMutex, CanCall_up_cast_move_constructor )
{
	obj_mutex<test_classB> ttB;
	ttB.lock_get().ref().a = 20;
	ASSERT_NO_THROW( {
		obj_mutex<test_classA> ttA = std::move( ttB );
		EXPECT_EQ( 20, ttA.lock_get().ref().a );
	} );
	EXPECT_FALSE( ttB.valid() );
	return;
}

TEST( ObjectMutex, CanCall_down_cast_move_constructor )
{
	obj_mutex<test_classB> ttB;
	ttB.lock_get().ref().a     = 20;
	obj_mutex<test_classA> ttA = std::move( ttB );
	ASSERT_NO_THROW( {
		obj_mutex<test_classB> ttB2 = std::move( ttA );
		EXPECT_EQ( 20, ttB2.lock_get().ref().a );
	} );
	EXPECT_FALSE( ttA.valid() );
	EXPECT_FALSE( ttB.valid() );
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

class test_class3 {
public:
	test_class3( int b_arg = 0 )
	  : b( b_arg )
	{
	}

	test_class3( const test_class3& orig )
	  : b( orig.b )
	{
	}

	int b;
};

TEST( ObjectMutex, CanCall_base_copy_constructor )
{
	// constructor check
	test_class3            ttorig( 10 );
	obj_mutex<test_class3> tt3( ttorig );

	EXPECT_EQ( 10, tt3.lock_get().ref().b );

	return;
}

TEST( ObjectMutex, CanCall_move_assignment )
{
	// constructor check
	test_class3            ttorig( 10 );
	obj_mutex<test_class3> tt1( 1 );
	obj_mutex<test_class3> tt2( 2 );

	EXPECT_EQ( 1, tt1.lock_get().ref().b );
	EXPECT_EQ( 2, tt2.lock_get().ref().b );

	tt2 = std::move( tt1 );

	EXPECT_EQ( 1, tt2.lock_get().ref().b );

	return;
}

TEST( ObjectMutex, CanCall_lock_get_upcast )
{
	obj_mutex<test_classB> ttB;
	ttB.lock_get().ref().a = 20;
	ASSERT_NO_THROW( {
		EXPECT_EQ( 20, ttB.lock_get<test_classA>().ref().a );
	} );
	return;
}

TEST( ObjectMutex, CanCall_lock_get_downcast )
{
	obj_mutex<test_classA> ttA = obj_mutex<test_classB>( 21 );

	ASSERT_NO_THROW( {
		EXPECT_EQ( 21, ttA.lock_get<test_classB>().ref().b );
	} );
	return;
}

TEST( ObjectMutex, Invalid_obj_mutex_throw )
{
	obj_mutex<test_class1> tt1;
	obj_mutex<test_class1> tt2 = std::move( tt1 );
	EXPECT_THROW( tt1.lock_get(), std::logic_error );
	EXPECT_EQ( 10, tt2.lock_get().ref().a );
	return;
}

TEST( ObjectMutex, obj_mutex_throw_by_fail_dynamic_cast )
{
	obj_mutex<test_classA> ttA;

	EXPECT_THROW( ttA.lock_get<test_classB>(), std::bad_cast );

	return;
}

TEST( ObjectMutex, clone_has_no_relationship )
{
	obj_mutex<test_class1> tt1( 1, 2 );
	obj_mutex<test_class1> tt2 = tt1.clone();

	EXPECT_TRUE( tt1.valid() );
	auto locked_acc1 = tt1.lock_get();
	EXPECT_FALSE( tt2.is_locked() );

	locked_acc1.ref().a = 10;

	auto locked_acc2 = tt2.lock_get();
	EXPECT_EQ( 1, locked_acc2.ref().a );

	return;
}

TEST( ObjectMutex, clone_to_convertible1 )
{
	obj_mutex<char> tt1( 1 );
	obj_mutex<int>  tt2 = tt1.clone<int>();

	EXPECT_EQ( 1, tt2.lock_get().ref() );

	return;
}

#if 0
struct is_callable_clone_impl {
	template <typename T, typename U>
	static auto check( T*, U* ) -> decltype( std::declval<obj_mutex<T>*>()->clone<U>(), std::true_type() );

	template <typename T, typename U>
	static auto check( ... ) -> std::false_type;
};

template <typename T, typename U>
struct is_callable_clone : decltype( is_callable_clone_impl::check<T, U>( nullptr, nullptr ) ) {};

TEST( ObjectMutex, clone_to_convertible2 )
{
	static_assert( is_callable_clone<char, int>::value, "should be convertible from char to int" );
	static_assert( !is_callable_clone<int, test_class1>::value, "should not be convertible from int to test_class1" );
	return;
}
#endif

TEST( ObjectMutex, shared_clone_has_the_relationship )
{
	obj_mutex<test_class1> tt1( 1, 2 );
	obj_mutex<test_class1> tt2 = tt1.shared_clone();

	EXPECT_TRUE( tt1.valid() );

	{
		auto locked_acc2 = tt2.lock_get();
		EXPECT_EQ( 1, locked_acc2.ref().a );
	}
	{
		auto locked_acc1 = tt1.lock_get();
		EXPECT_TRUE( tt2.is_locked() );
		EXPECT_EQ( 1, locked_acc1.ref().a );

		locked_acc1.ref().a = 10;
	}

	{
		auto locked_acc2 = tt2.lock_get();
		EXPECT_TRUE( tt1.is_locked() );
		EXPECT_EQ( 10, locked_acc2.ref().a );
	}

	return;
}

class test_class4 {
public:
	test_class4( std::unique_ptr<int> up_b_arg )
	  : a( 0 )
	  , up_b( std::move( up_b_arg ) )
	{
	}

	test_class4( int a_arg, std::unique_ptr<int> up_b_arg )
	  : a( a_arg )
	  , up_b( std::move( up_b_arg ) )
	{
	}

	int                  a;
	std::unique_ptr<int> up_b;
};

TEST( ObjectMutex, T_is_construct_by_move_only_argument1 )
{
	obj_mutex<test_class4> ttA( std::unique_ptr<int>( new int( 11 ) ) );

	EXPECT_EQ( 11, *( ttA.lock_get().ref().up_b ) );

	return;
}

TEST( ObjectMutex, T_is_construct_by_move_only_argument2 )
{
	obj_mutex<test_class4> ttA( 11, std::unique_ptr<int>( new int( 12 ) ) );

	EXPECT_EQ( 11, ttA.lock_get().ref().a );
	EXPECT_EQ( 12, *( ttA.lock_get().ref().up_b ) );

	return;
}
