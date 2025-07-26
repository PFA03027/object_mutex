/**
 * @file object_mutex.hpp
 * @author PFA03027@nifty.com
 * @brief A wrapper class that exclusively controls access to class instances
 * @version 0.1
 * @date 2023-02-04
 *
 * @copyright Copyright (c) 2023, PFA03027@nifty.com
 *
 * @pre C++17 is required by std::scope_lock. If you use on C++11/C++14, alternative implementation of scope_lock is required with using std::lock()
 */

#ifndef OBJECT_MUTEX_HPP_
#define OBJECT_MUTEX_HPP_

#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>

/**
 * @brief A wrapper class that exclusively controls access to class instances
 *
 * @tparam T type of object to be controlled exclusively
 * @tparam MTX_T type of mutex to be used for exclusive control
 */
template <typename T, typename MTX_T = std::mutex>
class obj_mutex {
public:
	using value_type = T;
	using mutex_type = MTX_T;

	struct is_callable_try_lock_for_impl {
		template <typename U, typename Rep, typename Period>
		static auto test( U* u, const std::chrono::duration<Rep, Period>& rel_time )
			-> decltype( u->try_lock_for( rel_time ), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX, class Rep, class Period>
	struct is_callable_try_lock_for
	  : decltype( is_callable_try_lock_for_impl::test( std::declval<MTX*>(), std::declval<const std::chrono::duration<Rep, Period>&>() ) ) {};

	struct is_callable_try_lock_until_impl {
		template <typename U, typename Clock, typename Duration>
		static auto test( U* u, const std::chrono::time_point<Clock, Duration>& abs_time )
			-> decltype( u->try_lock_until( abs_time ), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX, class Clock, class Duration>
	struct is_callable_try_lock_until
	  : decltype( is_callable_try_lock_until_impl::test( std::declval<MTX*>(), std::declval<const std::chrono::time_point<Clock, Duration>&>() ) ) {};

	struct is_callable_native_handle_impl {
		template <typename U>
		static auto test( U* u )
			-> decltype( u->native_handle(), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX>
	struct is_callable_native_handle
	  : decltype( is_callable_native_handle_impl::test( std::declval<MTX*>() ) ) {};

	struct is_callable_lock_shared_impl {
		template <typename U>
		static auto test( U* u )
			-> decltype( u->lock_shared(), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX>
	struct is_callable_lock_shared
	  : decltype( is_callable_lock_shared_impl::test( std::declval<MTX*>() ) ) {};

	struct is_callable_try_lock_shared_for_impl {
		template <typename U, typename Rep, typename Period>
		static auto test( U* u, const std::chrono::duration<Rep, Period>& rel_time )
			-> decltype( u->try_lock_shared_for( rel_time ), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX, class Rep, class Period>
	struct is_callable_try_lock_shared_for
	  : decltype( is_callable_try_lock_shared_for_impl::test( std::declval<MTX*>(), std::declval<const std::chrono::duration<Rep, Period>&>() ) ) {};

	struct is_callable_try_lock_shared_until_impl {
		template <typename U, typename Clock, typename Duration>
		static auto test( U* u, const std::chrono::time_point<Clock, Duration>& abs_time )
			-> decltype( u->try_lock_shared_until( abs_time ), std::true_type() );
		static std::false_type test( ... );
	};
	template <class MTX, class Clock, class Duration>
	struct is_callable_try_lock_shared_until
	  : decltype( is_callable_try_lock_shared_until_impl::test( std::declval<MTX*>(), std::declval<const std::chrono::time_point<Clock, Duration>&>() ) ) {};

	~obj_mutex()      = default;
	obj_mutex( void ) = default;

	/**
	 * @brief Construct a new obj mutex object
	 *
	 * @pre src must not be locked.
	 * @param src
	 */
	obj_mutex( const obj_mutex& src )
	  : mtx_()
	{
		std::lock_guard<MTX_T> lock( src.mtx_ );
		v_ = src.v_;
	}

	/**
	 * @brief Assign a new value to this object mutex
	 *
	 * @pre this and src must not be locked.
	 *
	 * @param src
	 * @return obj_mutex&
	 */
	obj_mutex& operator=( const obj_mutex& src )
	{
		if ( this == &src ) {
			return *this;
		}
		std::scoped_lock lock( mtx_, src.mtx_ );
		v_ = src.v_;
		return *this;
	}

	/**
	 * @brief Construct a new obj mutex object
	 *
	 * @pre src must not be locked.
	 *
	 * @param src
	 */
	obj_mutex( obj_mutex&& src )
	  : mtx_()
	{
		std::lock_guard<MTX_T> lock( src.mtx_ );
		v_ = std::move( src.v_ );
	}

	/**
	 * @brief Assign a new value to this object mutex
	 *
	 * @pre this and src must not be locked.
	 *
	 * @param src
	 * @return obj_mutex&
	 */
	obj_mutex& operator=( obj_mutex&& src )
	{
		if ( this == &src ) {
			return *this;
		}
		std::scoped_lock lock( mtx_, src.mtx_ );
		v_ = std::move( src.v_ );
		return *this;
	}

	/**
	 * @brief Construct a new obj mutex object from convertible type
	 *
	 * @pre src must not be locked.
	 *
	 * @tparam U type of object to be controlled exclusively
	 * @param src
	 */
	template <typename U, typename std::enable_if<std::is_constructible<T, const U&>::value>::type* = nullptr>
	obj_mutex( const obj_mutex<U, MTX_T>& src )
	  : mtx_()
	{
		std::lock_guard<MTX_T> lock( src.mtx_ );
		v_ = src.v_;
	}

	/**
	 * @brief Assign a new value to this object mutex from convertible type
	 *
	 * @pre this and src must not be locked.
	 *
	 * @tparam U type of object to be controlled exclusively
	 * @param src
	 * @return obj_mutex&
	 */
	template <typename U, typename std::enable_if<std::is_convertible<const U, T>::value>::type* = nullptr>
	obj_mutex& operator=( const obj_mutex<U, MTX_T>& src )
	{
		std::scoped_lock lock( mtx_, src.mtx_ );
		v_ = src.v_;
		return *this;
	}

	/**
	 * @brief Construct a new obj mutex object from convertible type
	 *
	 * @pre src must not be locked.
	 *
	 * @tparam U type of object to be controlled exclusively
	 * @param src
	 */
	template <typename U, typename std::enable_if<std::is_constructible<T, U&&>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& src )
	  : mtx_()
	{
		std::lock_guard<MTX_T> lock( src.mtx_ );
		v_ = std::move( src.v_ );
	}

	/**
	 * @brief Assign a new value to this object mutex from convertible type
	 *
	 * @pre this and src must not be locked.
	 *
	 * @tparam U type of object to be controlled exclusively
	 * @param src
	 * @return obj_mutex&
	 */
	template <typename U, typename std::enable_if<std::is_convertible<U&&, T>::value>::type* = nullptr>
	obj_mutex& operator=( obj_mutex<U, MTX_T>&& src )
	{
		std::scoped_lock lock( mtx_, src.mtx_ );
		v_ = std::move( src.v_ );
		return *this;
	}

	/**
	 * @brief Construct a new obj mutex object from convertible type
	 *
	 * @tparam U type of object to be controlled exclusively
	 * @param src
	 */
	template <typename U, typename std::enable_if<std::is_constructible<T, U>::value>::type* = nullptr>
	obj_mutex( U&& src )
	  : mtx_()
	  , v_( std::forward<U>( src ) )
	{
	}

	void lock( void )
	{
		mtx_.lock();
	}

	bool try_lock( void )
	{
		return mtx_.try_lock();
	}

	template <class Rep, class Period, typename std::enable_if<is_callable_try_lock_for<mutex_type, Rep, Period>::value>::type* = nullptr>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& rel_time )
	{
		return mtx_.try_lock_for( rel_time );
	}

	template <class Clock, class Duration, typename std::enable_if<is_callable_try_lock_until<mutex_type, Clock, Duration>::value>::type* = nullptr>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& abs_time )
	{
		return mtx_.try_lock_until( abs_time );
	}

	void unlock( void )
	{
		mtx_.unlock();
	}

	template <typename U = MTX_T, typename std::enable_if<is_callable_lock_shared<U>::value>::type* = nullptr>
	void lock_shared( void )
	{
		mtx_.lock_shared();
	}

	template <typename U = MTX_T, typename std::enable_if<is_callable_lock_shared<U>::value>::type* = nullptr>
	bool try_lock_shared( void )
	{
		return mtx_.try_lock_shared();
	}

	template <class Rep, class Period, typename std::enable_if<is_callable_try_lock_shared_for<mutex_type, Rep, Period>::value>::type* = nullptr>
	bool try_lock_shared_for( const std::chrono::duration<Rep, Period>& rel_time )
	{
		return mtx_.try_lock_shared_for( rel_time );
	}

	template <class Clock, class Duration, typename std::enable_if<is_callable_try_lock_shared_until<mutex_type, Clock, Duration>::value>::type* = nullptr>
	bool try_lock_shared_until( const std::chrono::time_point<Clock, Duration>& abs_time )
	{
		return mtx_.try_lock_shared_until( abs_time );
	}

	template <typename U = MTX_T, typename std::enable_if<is_callable_lock_shared<U>::value>::type* = nullptr>
	void unlock_shared( void )
	{
		mtx_.unlock_shared();
	}

	template <typename U = MTX_T, typename std::enable_if<is_callable_native_handle<U>::value>::type* = nullptr>
	auto native_handle( void )
	{
		return mtx_.native_handle();
	}

	mutex_type* mutex( void ) const noexcept
	{
		return &mtx_;
	}

private:
	value_type& ref( void ) noexcept
	{
		return v_;
	}
	const value_type& ref( void ) const noexcept
	{
		return v_;
	}

	mutable mutex_type mtx_;   //!< mutex for this object
	value_type         v_;     //!< value of type T that is controlled exclusively

	template <typename U, typename MTX_U>
	friend class obj_mutex;

	template <typename OM>
	friend class obj_lock_guard;

	template <typename OM>
	friend class obj_unique_lock;

	template <typename OM>
	friend class obj_shared_lock;
};

template <typename OM>
class obj_lock_guard : public std::lock_guard<typename OM::mutex_type> {
public:
	using mutex_type       = typename OM::mutex_type;
	using value_type       = typename OM::value_type;
	using const_value_type = typename std::add_const<typename OM::value_type>::type;

	~obj_lock_guard( void )                            = default;
	obj_lock_guard( void )                             = delete;
	obj_lock_guard( const obj_lock_guard& )            = delete;
	obj_lock_guard& operator=( const obj_lock_guard& ) = delete;
	obj_lock_guard( obj_lock_guard&& )                 = delete;
	obj_lock_guard& operator=( obj_lock_guard&& )      = delete;

	obj_lock_guard( OM& om )
	  : std::lock_guard<mutex_type>( *om.mutex() )
	  , p_om_( &om )
	{
	}

	value_type& ref( void ) noexcept
	{
		return p_om_->ref();
	}
	const_value_type& ref( void ) const noexcept
	{
		return p_om_->ref();
	}

	value_type& operator*( void ) noexcept
	{
		return p_om_->ref();
	}
	const_value_type& operator*( void ) const noexcept
	{
		return p_om_->ref();
	}
	value_type* operator->( void ) noexcept
	{
		return &p_om_->ref();
	}
	const_value_type* operator->( void ) const noexcept
	{
		return &p_om_->ref();
	}

private:
	OM* p_om_;   //!< pointer to the object mutex being locked
};

#if __cpp_deduction_guides >= 201611L
// Deduction guide for obj_lock_guard
template <typename OM>
obj_lock_guard( OM& ) -> obj_lock_guard<OM>;
#endif

template <typename OM>
class obj_unique_lock : public std::unique_lock<typename OM::mutex_type> {
public:
	using mutex_type       = typename OM::mutex_type;
	using value_type       = typename OM::value_type;
	using const_value_type = typename std::add_const<typename OM::value_type>::type;

	~obj_unique_lock( void ) = default;
	obj_unique_lock( void )
	  : std::unique_lock<mutex_type>()
	  , p_om_( nullptr )
	{
	}
	obj_unique_lock( const obj_unique_lock& )            = delete;
	obj_unique_lock& operator=( const obj_unique_lock& ) = delete;
	obj_unique_lock( obj_unique_lock&& src )
	  : std::unique_lock<mutex_type>( std::move( src ) )
	  , p_om_( src.p_om_ )
	{
		src.p_om_ = nullptr;   // transfer ownership
	}
	obj_unique_lock& operator=( obj_unique_lock&& src )
	{
		if ( this == &src ) {
			return *this;
		}

		obj_unique_lock( std::move( src ) ).swap( *this );
		return *this;
	}
	void swap( obj_unique_lock& other ) noexcept
	{
		std::unique_lock<mutex_type>::swap( other );
		std::swap( p_om_, other.p_om_ );
	}

	obj_unique_lock( OM& om )
	  : std::unique_lock<mutex_type>( *om.mutex() )
	  , p_om_( &om )
	{
	}
	obj_unique_lock( OM& om, std::defer_lock_t )
	  : std::unique_lock<mutex_type>( *om.mutex(), std::defer_lock )
	  , p_om_( &om )
	{
	}
	obj_unique_lock( OM& om, std::try_to_lock_t )
	  : std::unique_lock<mutex_type>( *om.mutex(), std::try_to_lock )
	  , p_om_( &om )
	{
	}
	obj_unique_lock( OM& om, std::adopt_lock_t )
	  : std::unique_lock<mutex_type>( *om.mutex(), std::adopt_lock )
	  , p_om_( &om )
	{
	}

	value_type& ref( void )
	{
		if ( !this->owns_lock() ) {
			throw std::runtime_error( "Cannot access ref() without owning the lock." );
		}
		if ( p_om_ == nullptr ) {
			throw std::runtime_error( "Cannot access ref() without object." );
		}
		return p_om_->ref();
	}

	const_value_type& ref( void ) const
	{
		if ( !this->owns_lock() ) {
			throw std::runtime_error( "Cannot access ref() without owning the lock." );
		}
		if ( p_om_ == nullptr ) {
			throw std::runtime_error( "Cannot access ref() without object." );
		}
		return p_om_->ref();
	}

	value_type& operator*( void ) noexcept
	{
		return p_om_->ref();
	}
	const_value_type& operator*( void ) const noexcept
	{
		return p_om_->ref();
	}
	value_type* operator->( void ) noexcept
	{
		return &p_om_->ref();
	}
	const_value_type* operator->( void ) const noexcept
	{
		return &p_om_->ref();
	}

private:
	OM* p_om_;   //!< pointer to the object mutex being locked
};

#if __cpp_deduction_guides >= 201611L
// Deduction guide for obj_lock_guard
template <typename OM>
obj_unique_lock( OM& ) -> obj_unique_lock<OM>;
template <typename OM>
obj_unique_lock( OM&, std::defer_lock_t ) -> obj_unique_lock<OM>;
template <typename OM>
obj_unique_lock( OM&, std::try_to_lock_t ) -> obj_unique_lock<OM>;
template <typename OM>
obj_unique_lock( OM&, std::adopt_lock_t ) -> obj_unique_lock<OM>;
#endif

template <typename OM>
class obj_shared_lock : public std::shared_lock<typename OM::mutex_type> {
public:
	using mutex_type       = typename OM::mutex_type;
	using const_value_type = typename std::add_const<typename OM::value_type>::type;

	~obj_shared_lock( void ) = default;
	obj_shared_lock( void )
	  : std::shared_lock<mutex_type>()
	  , p_om_( nullptr )
	{
	}
	obj_shared_lock( const obj_shared_lock& )            = delete;
	obj_shared_lock& operator=( const obj_shared_lock& ) = delete;
	obj_shared_lock( obj_shared_lock&& src )
	  : std::shared_lock<mutex_type>( std::move( src ) )
	  , p_om_( src.p_om_ )
	{
		src.p_om_ = nullptr;   // transfer ownership
	}
	obj_shared_lock& operator=( obj_shared_lock&& src )
	{
		if ( this == &src ) {
			return *this;
		}
		obj_shared_lock( std::move( src ) ).swap( *this );
		return *this;
	}

	obj_shared_lock( OM& om )
	  : std::shared_lock<mutex_type>( *om.mutex() )
	  , p_om_( &om )
	{
	}
	obj_shared_lock( OM& om, std::defer_lock_t )
	  : std::shared_lock<mutex_type>( *om.mutex(), std::defer_lock )
	  , p_om_( &om )
	{
	}
	obj_shared_lock( OM& om, std::try_to_lock_t )
	  : std::shared_lock<mutex_type>( *om.mutex(), std::try_to_lock )
	  , p_om_( &om )
	{
	}
	obj_shared_lock( OM& om, std::adopt_lock_t )
	  : std::shared_lock<mutex_type>( *om.mutex(), std::adopt_lock )
	  , p_om_( &om )
	{
	}

	void swap( obj_shared_lock& other ) noexcept
	{
		std::shared_lock<mutex_type>::swap( other );
		std::swap( p_om_, other.p_om_ );
	}

	const_value_type& ref( void ) const
	{
		if ( !this->owns_lock() ) {
			throw std::runtime_error( "Cannot access ref() without owning the lock." );
		}
		if ( p_om_ == nullptr ) {
			throw std::runtime_error( "Cannot access ref() without object." );
		}
		return p_om_->ref();
	}

	const_value_type& operator*( void ) const noexcept
	{
		return p_om_->ref();
	}
	const_value_type* operator->( void ) const noexcept
	{
		return &p_om_->ref();
	}

private:
	const OM* p_om_;   //!< pointer to the object mutex being locked
};

#if __cpp_deduction_guides >= 201611L
// Deduction guide for obj_lock_guard
template <typename OM>
obj_shared_lock( OM& ) -> obj_shared_lock<OM>;
template <typename OM>
obj_shared_lock( OM&, std::defer_lock_t ) -> obj_shared_lock<OM>;
template <typename OM>
obj_shared_lock( OM&, std::try_to_lock_t ) -> obj_shared_lock<OM>;
template <typename OM>
obj_shared_lock( OM&, std::adopt_lock_t ) -> obj_shared_lock<OM>;
#endif

#endif
