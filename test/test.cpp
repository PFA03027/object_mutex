/**
 * @file test.cpp
 * @author Teruaki Ata (PFA03027@nifty.com)
 * @brief
 * @version 0.1
 * @date 2025-07-26
 *
 * @copyright Copyright (c) 2025, Teruaki Ata (PFA03027@nifty.com)
 *
 */

#include <future>
#include <memory>
#include <shared_mutex>

#include "object_mutex.hpp"

#include <gtest/gtest.h>

// ========================================================

TEST( TestObjectMutex, CanDefaultConstruct )
{
	// Arrange

	// Act
	obj_mutex<int> om;

	// Assert
}

TEST( TestObjectMutex, CanNativeHandle )
{
	// Arrange
	obj_mutex<int> om;

	// Act
	auto handle = om.native_handle();

	// Assert
	EXPECT_NE( handle, nullptr );
}

TEST( TestObjectMutex, CanTryLock )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> sut( 42 );

	// Act
	bool locked = sut.try_lock();

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	sut.unlock();
}

TEST( TestObjectMutex, CanTryLockFor )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> sut( 42 );

	// Act
	bool locked = sut.try_lock_for( std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	sut.unlock();
}

TEST( TestObjectMutex, CanTryLockUntil )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> sut( 42 );

	// Act
	bool locked = sut.try_lock_until( std::chrono::steady_clock::now() + std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	sut.unlock();
}

TEST( TestObjectMutex, CanLock_ThenCheckByTryLock_ThenUnlock )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> sut( 42 );

	// Act
	sut.lock();

	// Assert
	std::packaged_task<bool()> task( [&sut]() {
		return sut.try_lock();
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	bool ret = future.get();

	EXPECT_FALSE( ret );

	// Clean up
	sut.unlock();
}

TEST( TestObjectMutex, CanLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );

	// Act
	om.lock_shared();

	// Assert
	std::packaged_task<bool()> task( [&om]() {
		obj_unique_lock lock( om, std::try_to_lock );
		return lock.owns_lock();
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	bool ret = future.get();
	EXPECT_FALSE( ret );

	// Clean up
	om.unlock_shared();
}

TEST( TestObjectMutex, CanTryLockShared_ThenLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );

	// Act
	bool locked = om.try_lock_shared();

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	om.unlock_shared();
}

TEST( TestObjectMutex, CanTryLockShared_ThenNotLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	om.lock();

	// Act
	std::packaged_task<bool()> task( [&om]() {
		return om.try_lock_shared();
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	bool ret = future.get();

	// Assert
	EXPECT_FALSE( ret );

	// Clean up
	om.unlock();
}

TEST( TestObjectMutex, CanTryLockSharedFor_ThenLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );

	// Act
	bool locked = om.try_lock_shared_for( std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	om.unlock_shared();
}

TEST( TestObjectMutex, CanTryLockSharedFor_ThenNotLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	om.lock();

	// Act
	std::packaged_task<bool()> task( [&om]() {
		return om.try_lock_shared_for( std::chrono::milliseconds( 1 ) );
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	bool ret = future.get();

	// Assert
	EXPECT_FALSE( ret );

	// Clean up
	om.unlock();
}

TEST( TestObjectMutex, CanTryLockSharedUntil_ThenLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );

	// Act
	bool locked = om.try_lock_shared_until( std::chrono::steady_clock::now() + std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );

	// Clean up
	om.unlock_shared();
}

TEST( TestObjectMutex, CanTryLockSharedUntil_ThenNotLockShared )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	om.lock();

	// Act
	std::packaged_task<bool()> task( [&om]() {
		return om.try_lock_shared_until( std::chrono::steady_clock::now() + std::chrono::milliseconds( 1 ) );
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	bool ret = future.get();

	// Assert
	EXPECT_FALSE( ret );

	// Clean up
	om.unlock();
}

TEST( TestObjectMutex, CanScopedLockPattern1 )
{
	// Arrange
	obj_mutex<int> om( 42 );

	// Act
	obj_lock_guard sut( om );

	// Assert
	EXPECT_EQ( sut.ref(), 42 );
	EXPECT_EQ( *sut, 42 );
	EXPECT_EQ( static_cast<const obj_lock_guard<obj_mutex<int>>&>( sut ).ref(), 42 );
	EXPECT_EQ( *static_cast<const obj_lock_guard<obj_mutex<int>>&>( sut ), 42 );
}

TEST( TestObjectMutex, CanScopedLockPattern2 )
{
	// Arrange
	struct test_t {
		int value;
		test_t( int v )
		  : value( v ) {}
	};
	obj_mutex<test_t> om( 42 );

	// Act
	obj_lock_guard sut( om );

	// Assert
	EXPECT_EQ( sut.ref().value, 42 );
	EXPECT_EQ( ( *sut ).value, 42 );
	EXPECT_EQ( sut->value, 42 );
	EXPECT_EQ( static_cast<const obj_lock_guard<obj_mutex<test_t>>&>( sut ).ref().value, 42 );
	EXPECT_EQ( ( *static_cast<const obj_lock_guard<obj_mutex<test_t>>&>( sut ) ).value, 42 );
	EXPECT_EQ( static_cast<const obj_lock_guard<obj_mutex<test_t>>&>( sut )->value, 42 );
}

TEST( TestObjectMutex, CanCopyConstruct )
{
	// Arrange
	obj_mutex<int> src( 42 );

	// Act
	obj_mutex<int> sut( src );

	// Assert
	obj_lock_guard lock( sut );
	EXPECT_EQ( lock.ref(), 42 );
}

TEST( TestObjectMutex, CanMoveConstruct )
{
	// Arrange
	obj_mutex<std::unique_ptr<int>> src( std::make_unique<int>( 42 ) );

	// Act
	obj_mutex<std::unique_ptr<int>> sut( std::move( src ) );

	// Assert
	obj_lock_guard lock( sut );
	EXPECT_EQ( *( lock.ref() ), 42 );
}

TEST( TestObjectMutex, CanCopyAssign )
{
	// Arrange
	obj_mutex<int> src( 42 );
	obj_mutex<int> sut( 0 );

	// Act
	sut = src;

	// Assert
	obj_lock_guard<obj_mutex<int>> lock( sut );
	EXPECT_EQ( lock.ref(), 42 );
}

TEST( TestObjectMutex, CanCopyConvertByConstructor )
{
	// Arrange
	obj_mutex<int> src( 42 );

	// Act
	obj_mutex<double> sut( src );

	// Assert
	obj_lock_guard lock( sut );
	EXPECT_EQ( lock.ref(), 42.0 );
}

TEST( TestObjectMutex, CanMoveConvertByConstructor )
{
	// Arrange
	obj_mutex<int> src( 42 );

	// Act
	obj_mutex<double> sut( std::move( src ) );

	// Assert
	obj_lock_guard lock( sut );
	EXPECT_EQ( lock.ref(), 42.0 );
}

TEST( TestObjectMutex, CanCopyConvertByAssing )
{
	// Arrange
	obj_mutex<int>    src( 42 );
	obj_mutex<double> sut( 0.0 );

	// Act
	sut = src;

	// Assert
	obj_lock_guard<obj_mutex<double>> lock( sut );
	EXPECT_EQ( lock.ref(), 42.0 );
}

TEST( TestObjectMutex, CanMoveConvertByAssing )
{
	// Arrange
	obj_mutex<int>    src( 42 );
	obj_mutex<double> sut( 0.0 );

	// Act
	sut = std::move( src );

	// Assert
	obj_lock_guard<obj_mutex<double>> lock( sut );
	EXPECT_EQ( lock.ref(), 42.0 );
}

TEST( TestObjectMutex, CanMoveAssign )
{
	// Arrange
	obj_mutex<std::unique_ptr<int>> src( std::make_unique<int>( 42 ) );
	obj_mutex<std::unique_ptr<int>> sut( std::make_unique<int>( 0 ) );

	// Act
	sut = std::move( src );

	// Assert
	obj_lock_guard lock( sut );
	EXPECT_EQ( *( lock.ref() ), 42 );
}

// ========================================================

TEST( TestObjUniqueLock, CanConstruct )
{
	// Arrange
	struct test_t {
		int value;
		test_t( int v )
		  : value( v ) {}
	};
	obj_mutex<test_t> om( 42 );

	// Act
	obj_unique_lock sut( om );

	// Assert
	EXPECT_EQ( sut.ref().value, 42 );
	EXPECT_EQ( ( *sut ).value, 42 );
	EXPECT_EQ( sut->value, 42 );
	EXPECT_EQ( static_cast<const obj_unique_lock<obj_mutex<test_t>>&>( sut ).ref().value, 42 );
	EXPECT_EQ( ( *static_cast<const obj_unique_lock<obj_mutex<test_t>>&>( sut ) ).value, 42 );
	EXPECT_EQ( static_cast<const obj_unique_lock<obj_mutex<test_t>>&>( sut )->value, 42 );
}

TEST( TestObjUniqueLock, CanConstructWithDeferLock )
{
	// Arrange
	obj_mutex<int> om( 42 );

	// Act
	obj_unique_lock sut( om, std::defer_lock );

	// Assert
	EXPECT_FALSE( sut.owns_lock() );
	EXPECT_ANY_THROW( sut.ref() );
	sut.lock();
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjUniqueLock, CanConstructkWithTryToLock_ThenLocked )
{
	// Arrange
	obj_mutex<int> om( 42 );

	// Act
	obj_unique_lock sut( om, std::try_to_lock );

	// Assert
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjUniqueLock, CanConstructkWithTryToLock_ThenNotLocked )
{
	// Arrange
	obj_mutex<int>  om( 42 );
	obj_unique_lock lock( om, std::try_to_lock );
	EXPECT_TRUE( lock.owns_lock() );

	// Act
	obj_unique_lock sut( om, std::try_to_lock );

	// Assert
	EXPECT_FALSE( sut.owns_lock() );
}

TEST( TestObjUniqueLock, CanConstructkWithTryToLockFor_ThenLocked )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> om( 42 );
	obj_unique_lock                  lock( om, std::defer_lock );
	EXPECT_FALSE( lock.owns_lock() );

	// Act
	bool ret = lock.try_lock_for( std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( ret );
	EXPECT_EQ( lock.ref(), 42 );
}

TEST( TestObjUniqueLock, CanConstructWithTryToLockFor_ThenNotLocked )
{
	// Arrange
	obj_mutex<int, std::timed_mutex> om( 42 );
	obj_unique_lock                  lock( om );
	EXPECT_TRUE( lock.owns_lock() );
	std::packaged_task<bool()> task( [&om]() {
		obj_unique_lock lock( om, std::defer_lock );
		return lock.try_lock_for( std::chrono::milliseconds( 1 ) );
	} );
	std::future<bool>          future = task.get_future();

	// Act
	std::thread t( std::move( task ) );
	t.detach();

	// Assert
	bool ret = future.get();
	EXPECT_FALSE( ret );
}

TEST( TestObjUniqueLock, CanConstructWithAdoptLock )
{
	// Arrange
	obj_mutex<int> om( 42 );
	om.lock();   // lock the mutex before adopting

	// Act
	obj_unique_lock sut( om, std::adopt_lock );

	// Assert
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjUniqueLock, CanMoveConstruct )
{
	// Arrange
	obj_mutex<int>  om( 42 );
	obj_unique_lock src( om );

	// Act
	obj_unique_lock sut( std::move( src ) );

	// Assert
	EXPECT_EQ( sut.ref(), 42 );
	EXPECT_FALSE( src.owns_lock() );   // src should not own the lock anymore
}

TEST( TestObjUniqueLock, CanMoveAssign )
{
	// Arrange
	obj_mutex<int>                  om( 42 );
	obj_unique_lock                 src( om );
	obj_unique_lock<obj_mutex<int>> sut;

	// Act
	sut = std::move( src );

	// Assert
	EXPECT_EQ( sut.ref(), 42 );
	EXPECT_FALSE( src.owns_lock() );   // src should not own the lock anymore
}

// ========================================================

TEST( TestObjSharedLock, CanConstruct )
{
	// Arrange
	struct test_t {
		int value;
		test_t( int v )
		  : value( v ) {}
	};
	obj_mutex<test_t, std::shared_mutex> om( 42 );

	// Act
	obj_shared_lock sut( om );

	// Assert
	EXPECT_EQ( sut.ref().value, 42 );
	EXPECT_EQ( ( *sut ).value, 42 );
	EXPECT_EQ( sut->value, 42 );
}

TEST( TestObjSharedLock, CanMoveConstruct )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	obj_shared_lock                   src( om );

	// Act
	obj_shared_lock sut( std::move( src ) );

	// Assert
	EXPECT_EQ( sut.ref(), 42 );
	EXPECT_FALSE( src.owns_lock() );   // src should not own the lock anymore
}

TEST( TestObjSharedLock, CanMoveAssign )
{
	// Arrange
	obj_mutex<int, std::shared_mutex>                  om( 42 );
	obj_shared_lock                                    src( om );
	obj_shared_lock<obj_mutex<int, std::shared_mutex>> sut;

	// Act
	sut = std::move( src );

	// Assert
	EXPECT_EQ( sut.ref(), 42 );
	EXPECT_FALSE( src.owns_lock() );   // src should not own the lock anymore
}

TEST( TestObjSharedLock, CanConstructWithDeferLock )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );

	// Act
	obj_shared_lock sut( om, std::defer_lock );

	// Assert
	EXPECT_FALSE( sut.owns_lock() );
	EXPECT_ANY_THROW( sut.ref() );
	sut.lock();
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjSharedLock, CanConstructkWithTryToLock_ThenLocked )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );

	// Act
	obj_shared_lock sut( om, std::try_to_lock );

	// Assert
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjSharedLock, CanConstructkWithTryToLock_ThenNotLocked )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	obj_unique_lock                   writer_lock( om );
	EXPECT_TRUE( writer_lock.owns_lock() );

	// Act
	std::packaged_task<bool()> task( [&om]() {
		obj_shared_lock sut( om, std::try_to_lock );
		return sut.owns_lock();
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();

	// Assert
	bool ret = future.get();
	EXPECT_FALSE( ret );
}

TEST( TestObjSharedLock, CanConstructkWithAdoptLock )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	om.lock_shared();   // lock the mutex before adopting

	// Act
	obj_shared_lock sut( om, std::adopt_lock );

	// Assert
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );
}

TEST( TestObjSharedLock, CanLock )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	obj_shared_lock                   sut( om, std::defer_lock );
	EXPECT_FALSE( sut.owns_lock() );

	// Act
	sut.lock();

	// Assert
	EXPECT_TRUE( sut.owns_lock() );
	EXPECT_EQ( sut.ref(), 42 );

	// Clean up
	sut.unlock();
}

TEST( TestObjSharedLock, CanTryLock )
{
	// Arrange
	obj_mutex<int, std::shared_mutex> om( 42 );
	obj_shared_lock                   sut( om, std::defer_lock );
	EXPECT_FALSE( sut.owns_lock() );

	// Act
	bool locked = sut.try_lock();

	// Assert
	EXPECT_TRUE( locked );
	EXPECT_EQ( sut.ref(), 42 );

	// Clean up
	sut.unlock();
}

TEST( TestObjSharedLock, CanTryLockFor_ThenLocked )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	obj_shared_lock                         sut( om, std::defer_lock );
	EXPECT_FALSE( sut.owns_lock() );

	// Act
	bool locked = sut.try_lock_for( std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );
	EXPECT_EQ( sut.ref(), 42 );

	// Clean up
	sut.unlock();
}

TEST( TestObjSharedLock, CanTryLockFor_ThenNotLocked )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	obj_unique_lock                         sut( om );
	EXPECT_TRUE( sut.owns_lock() );

	// Act
	std::packaged_task<bool()> task( [&om]() {
		obj_shared_lock lock( om, std::defer_lock );
		return lock.try_lock_for( std::chrono::milliseconds( 1 ) );
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();

	// Assert
	bool ret = future.get();
	EXPECT_FALSE( ret );
}

TEST( TestObjSharedLock, CanTryLockUntil_ThenLocked )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	obj_shared_lock                         sut( om, std::defer_lock );
	EXPECT_FALSE( sut.owns_lock() );

	// Act
	bool locked = sut.try_lock_until( std::chrono::steady_clock::now() + std::chrono::milliseconds( 100 ) );

	// Assert
	EXPECT_TRUE( locked );
	EXPECT_EQ( sut.ref(), 42 );

	// Clean up
	sut.unlock();
}

TEST( TestObjSharedLock, CanTryLockUntil_ThenNotLocked )
{
	// Arrange
	obj_mutex<int, std::shared_timed_mutex> om( 42 );
	obj_unique_lock                         sut( om );
	EXPECT_TRUE( sut.owns_lock() );

	// Act
	std::packaged_task<bool()> task( [&om]() {
		obj_shared_lock lock( om, std::defer_lock );
		return lock.try_lock_until( std::chrono::steady_clock::now() + std::chrono::milliseconds( 1 ) );
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();

	// Assert
	bool ret = future.get();
	EXPECT_FALSE( ret );

	// Clean up
	sut.unlock();
}

// ========================================================

TEST( TestObjMutex, CanHandleWithStdConditionVariable )
{
	// Arrange
	obj_mutex<int>             om( 42 );
	std::condition_variable    cv;
	std::packaged_task<bool()> task( [&om, &cv]() {
		obj_unique_lock lock( om );
		cv.wait( lock, [&lock]() { return lock.ref() == 0; } );
		lock.ref() = 1;   // Change the value to trigger the condition variable
		return true;
	} );
	std::future<bool>          future = task.get_future();
	std::thread                t( std::move( task ) );
	t.detach();
	{
		obj_lock_guard lock( om );
		lock.ref() = 0;   // Change the value to trigger the condition variable
	}

	// Act
	cv.notify_all();

	// Assert
	bool ret = future.get();
	EXPECT_TRUE( ret );
	obj_lock_guard lock2( om );
	EXPECT_EQ( lock2.ref(), 1 );   // Check if the value was changed by the thread waiting on the condition variable
}
