/**
 * @file object_mutex.hpp
 * @author PFA03027@nifty.com
 * @brief A wrapper class that exclusively controls access to class instances
 * @version 0.1
 * @date 2023-02-04
 *
 * @copyright Copyright (c) 2023, PFA03027@nifty.com
 *
 */

#ifndef OBJECT_MUTEX_HPP_
#define OBJECT_MUTEX_HPP_

#include <memory>
#include <mutex>
#include <stdexcept>

template <typename MTX_T = std::mutex>
struct data_carrier_base_mtx {
	virtual ~data_carrier_base_mtx() {}

	MTX_T mtx_;
};

template <typename T, typename MTX_T = std::mutex>
struct data_carrier_non_class : public data_carrier_base_mtx<MTX_T> {
	template <typename... Args>
	data_carrier_non_class( Args&&... args )
	  : data_carrier_base_mtx<MTX_T>()
	  , data_( std::forward<Args>( args )... )
	{
	}

	T data_;

	template <typename U, typename MTX_U>
	friend class obj_mutex;
};

template <typename T, typename MTX_T = std::mutex>
struct data_carrier_class : public data_carrier_base_mtx<MTX_T>, public T {
	template <typename... Args>
	data_carrier_class( Args&&... args )
	  : data_carrier_base_mtx<MTX_T>()
	  , T( std::forward<Args>( args )... )
	{
	}

	template <typename U, typename MTX_U>
	friend class obj_mutex;
};

template <typename T, typename MTX_T = std::mutex>
class obj_mutex {
public:
	class single_accessor {
	public:
		/**
		 * @brief get a reference of a target object
		 *
		 * This member function is defined in case that T(=U) is base class of ACTUAL_T or same to ACTUAL_T.
		 *
		 * @return T&
		 */
		T& ref( void )
		{
			if ( sp_data_ == nullptr ) {
				throw std::logic_error( "single_accessor is empty. has been moved ?" );
			}
			return ref_to_data_;
		}

		/**
		 * @brief move constructor of a new single accessor object
		 *
		 * @param orig
		 */
		single_accessor( single_accessor&& orig )
		  : sp_data_( std::move( orig.sp_data_ ) )
		  , lk_( std::move( orig.lk_ ) )
		  , ref_to_data_( orig.ref_to_data_ )
		{
		}

		/**
		 * @brief move assignmment
		 *
		 * @param orig
		 * @return single_accessor&
		 */
		single_accessor& operator=( single_accessor&& orig )
		{
			lk_.unlock();                           // 「unlock -> メモリ参照先の開放」という順番となるようにする。
			lk_          = std::move( orig.lk_ );   // orig.lk_は、すでにlock済み
			sp_data_     = std::move( orig.sp_data_ );
			ref_to_data_ = orig.ref_to_data_;
			return *this;
		}

		/**
		 * @brief check the validity
		 *
		 * @return true this has a valid object
		 * @return false this does not have any valid object. e.g. this will happen after move
		 */
		bool valid( void ) const
		{
			return ( sp_data_ != nullptr );
		}

		~single_accessor()
		{
			// メンバ変数定義と逆順にデストラクタが起動されるため、
			// 自動的に、unlock -> メモリ参照先の開放 という不正アクセスとはならない処理順になる。
		}

	private:
		single_accessor( std::unique_lock<MTX_T> lk_arg, std::shared_ptr<data_carrier_base_mtx<MTX_T>> sp_data_arg, T& ref_to_data_arg )
		  : sp_data_( std::move( sp_data_arg ) )
		  , lk_( std::move( lk_arg ) )
		  , ref_to_data_( ref_to_data_arg )
		{
		}

		single_accessor( const single_accessor& )            = delete;
		single_accessor& operator=( const single_accessor& ) = delete;

		std::shared_ptr<data_carrier_base_mtx<MTX_T>> sp_data_;
		std::unique_lock<MTX_T>                       lk_;
		T&                                            ref_to_data_;

		template <typename U, typename MTX_U>
		friend class obj_mutex;
	};

	//////////////////////
	/**
	 * @brief Default constructor of a new obj mutex object
	 *
	 * If T has a default constructor, this default constructor is defined.
	 *
	 * @tparam U
	 */
	template <typename U = T, typename std::enable_if<std::is_default_constructible<U>::value && std::is_class<U>::value>::type* = nullptr>
	obj_mutex( void )
	  : sp_data_( std::make_shared<data_carrier_class<U, MTX_T>>() )
	{
	}

	/**
	 * @brief Default constructor of a new obj mutex object
	 *
	 * If T has a default constructor, this default constructor is defined.
	 *
	 * @tparam U
	 */
	template <typename U = T, typename std::enable_if<std::is_default_constructible<U>::value && !std::is_class<U>::value>::type* = nullptr>
	obj_mutex( void )
	  : sp_data_( std::make_shared<data_carrier_non_class<U, MTX_T>>() )
	{
	}

	/**
	 * @brief Move constructor of a new obj mutex object with up-cast or no cast
	 *
	 * @tparam U original type of move
	 * @tparam std::enable_if<std::is_base_of<T, U>::value>::type
	 * @param orig
	 */
	template <typename U = T, typename std::enable_if<std::is_base_of<T, U>::value || std::is_same<T, U>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_data_( std::move( orig.sp_data_ ) )
	{
	}

	/**
	 * @brief Move constructor of a new obj mutex object with down cast
	 *
	 * @tparam U original type of move
	 * @tparam std::enable_if<std::is_base_of<T, U>::value>::type
	 * @param orig
	 *
	 * @exception std::bad_cast fail to down-cast from U to T. This means T is not derived class of U
	 */
	template <typename U = T, typename std::enable_if<std::is_base_of<U, T>::value && !std::is_same<T, U>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_data_( std::move( orig.sp_data_ ) )
	{
		T* p_T_tmp = dynamic_cast<T*>( sp_data_.get() );
		if ( p_T_tmp == nullptr ) {
			orig.sp_data_ = std::move( sp_data_ );
			throw std::bad_cast();   // fail to down cast or could not be convertible
		}
	}

	/**
	 * @brief Construct a new obj mutex object
	 *
	 * Construct with using T(headarg, args...)
	 *
	 * @tparam HEADArg this is a type of 1st argument, and is not type of obj_mutex<U>
	 * @tparam Args these are the types of 2nd and more arguments
	 * @param headarg 1st argument of T's constructor
	 * @param args 2nd and more arguments of T's constructor
	 */
	template <typename HEADArg, typename U = T, typename MTX_U = MTX_T, typename... Args,
	          typename std::enable_if<
				  !std::is_same<
					  obj_mutex<U, MTX_U>,
					  typename std::remove_cv<
						  typename std::remove_reference<HEADArg>::type>::type>::value &&
				  std::is_class<U>::value>::type* = nullptr>
	obj_mutex( HEADArg&& headarg, Args&&... args )
	  : sp_data_( std::make_shared<data_carrier_class<U, MTX_U>>( std::forward<HEADArg>( headarg ), std::forward<Args>( args )... ) )
	{
	}

	/**
	 * @brief Construct a new obj mutex object
	 *
	 * Construct with using T(headarg, args...)
	 *
	 * @tparam HEADArg this is a type of 1st argument, and is not type of obj_mutex<U>
	 * @tparam Args these are the types of 2nd and more arguments
	 * @param headarg 1st argument of T's constructor
	 * @param args 2nd and more arguments of T's constructor
	 */
	template <typename HEADArg, typename U = T, typename MTX_U = MTX_T, typename... Args,
	          typename std::enable_if<
				  !std::is_same<
					  obj_mutex<U, MTX_U>,
					  typename std::remove_cv<
						  typename std::remove_reference<HEADArg>::type>::type>::value &&
				  !std::is_class<U>::value>::type* = nullptr>
	obj_mutex( HEADArg&& headarg, Args&&... args )
	  : sp_data_( std::make_shared<data_carrier_non_class<U, MTX_U>>( std::forward<HEADArg>( headarg ), std::forward<Args>( args )... ) )
	{
	}

	/**
	 * @brief move assignmment with up-cast or no cast
	 *
	 * @param orig
	 * @return obj_mutex&
	 */
	template <typename U, typename std::enable_if<std::is_base_of<T, U>::value || std::is_same<T, U>::value>::type* = nullptr>
	obj_mutex& operator=( obj_mutex<U, MTX_T>&& orig )
	{
		sp_data_ = std::move( orig.sp_data_ );
		return *this;
	}

	/**
	 * @brief move assignmment
	 *
	 * @param orig
	 * @return obj_mutex&
	 *
	 * @exception std::bad_cast fail to down-cast from U to T. This means T is not derived class of U
	 */
	template <typename U, typename std::enable_if<std::is_base_of<U, T>::value && !std::is_same<T, U>::value>::type* = nullptr>
	obj_mutex& operator=( obj_mutex<U, MTX_T>&& orig )
	{
		T* p_T_tmp = dynamic_cast<T*>( orig.sp_data_.get() );
		if ( p_T_tmp == nullptr ) {
			throw std::bad_cast();   // fail to down cast or could not be convertible
		}
		sp_data_ = std::move( orig.sp_data_ );
		return *this;
	}

	/**
	 * @brief check the validity
	 *
	 * @return true this has a valid object
	 * @return false this does not have any valid object. e.g. this will happen after move
	 */
	bool valid( void ) const
	{
		return ( sp_data_ != nullptr );
	}

	/**
	 * @brief get single accessor object with up-cast
	 *
	 * if this object is not valid( valid() is flase ), this throws std::logic_error
	 *
	 * @tparam U this U is base type of T
	 * @return obj_mutex<U, MTX_T>::single_accessor
	 */
	template <typename U = T>
	typename obj_mutex<U, MTX_T>::single_accessor lock_get( void )
	{
		// 型Tの基底クラスUへのアップキャストされたsingle_accessorを得る
		if ( sp_data_ == nullptr ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}

		std::unique_lock<MTX_T> lk_my( sp_data_->mtx_ );
		return typename obj_mutex<U, MTX_T>::single_accessor( std::move( lk_my ), sp_data_, get_ref<U>() );
	}
	template <typename U = T>
	typename obj_mutex<const U, MTX_T>::single_accessor lock_get( void ) const
	{
		// 型Tの基底クラスUへのアップキャストされたsingle_accessorを得る
		if ( sp_data_ == nullptr ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}

		std::unique_lock<MTX_T> lk_my( sp_data_->mtx_ );
		return typename obj_mutex<const U, MTX_T>::single_accessor( std::move( lk_my ), sp_data_, get_ref<U>() );
	}

	/**
	 * @brief make a clone object
	 *
	 * there is no exclusive control b/w the cloned object and this object.
	 *
	 * @tparam U type that will clone to
	 * @tparam MTX_U type of mutex
	 * @return obj_mutex<U, MTX_U> a cloned object
	 */
	template <typename U = T, typename MTX_U = MTX_T, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
	obj_mutex<U, MTX_U> clone( void ) const
	{
		return obj_mutex<U, MTX_U>( lock_get().ref() );
	}

	/**
	 * @brief make a shered clone object
	 *
	 * there is the exclusive control b/w the cloned object and this object.
	 *
	 * @return obj_mutex  a cloned object that shares a mutex
	 */
	template <typename U = T, typename std::enable_if<std::is_same<U, T>::value>::type* = nullptr>
	obj_mutex<U, MTX_T> shared_clone( void ) const
	{
		return obj_mutex<U, MTX_T>( sp_data_ );
	}

	/**
	 * @brief make a shered clone object
	 *
	 * there is the exclusive control b/w the cloned object and this object.
	 *
	 * @return obj_mutex  a cloned object that shares a mutex
	 *
	 * @exception std::bad_cast fail to down-cast from T/U to U/T. This means U/T is not derived class of T/U
	 */
	template <typename U = T, typename std::enable_if<!std::is_same<U, T>::value && ( std::is_base_of<U, T>::value || std::is_base_of<T, U>::value )>::type* = nullptr>
	obj_mutex<U, MTX_T> shared_clone( void ) const
	{
		U* p_U_tmp = dynamic_cast<U*>( sp_data_.get() );
		if ( p_U_tmp == nullptr ) {
			throw std::bad_cast();
		}
		return obj_mutex<U, MTX_T>( sp_data_ );
	}

	/**
	 * @brief check this is locked or not
	 *
	 * @return true
	 * @return false
	 */
	bool is_locked( void ) const
	{
		bool ans = sp_data_->mtx_.try_lock();
		if ( ans ) {
			sp_data_->mtx_.unlock();
		}
		return !ans;
	}

private:
	obj_mutex( const std::shared_ptr<data_carrier_base_mtx<MTX_T>>& sp_data_arg )
	  : sp_data_( sp_data_arg )
	{
	}

	// コピーコンストラクタを定義した場合、文脈上、2つの振る舞いが利用者の期待値として存在しうる。
	// (1)保持している型Tの実体のコピー=複製。いわゆる深いコピー。
	//    既存のインスタントと異なるオブジェクトが生成される＝排他制御は分離される。
	// (2)排他制御を行うためのアクセスポイントとしての複製。いわゆる浅いコピー。
	//
	// obj_mutexの趣旨として、データ実体はアクセスは、single_accessor経由でアクセスすべきである。
	// この点を踏まえると、(1)を実装すべきと考えられるが、一方でコードの見た目として(2)のと区別が不明である。
	// よって、コピーコンストラクタ、copy assignmentは定義しない。
	// (1)相当の機能は、clone()で提供する。
	// (2)相当の機能は、shared_clone()で提供する。
	obj_mutex( const obj_mutex& orig )            = delete;
	obj_mutex& operator=( const obj_mutex& orig ) = delete;

	template <typename U, typename X = T, typename std::enable_if<std::is_class<X>::value>::type* = nullptr>
	U& get_ref( void )
	{
		return dynamic_cast<U&>( *sp_data_ );
	}

	template <typename U, typename X = T, typename std::enable_if<std::is_class<X>::value>::type* = nullptr>
	const U& get_ref( void ) const
	{
		return dynamic_cast<U&>( *sp_data_ );
	}

	template <typename U, typename X = T, typename std::enable_if<!std::is_class<X>::value>::type* = nullptr>
	U& get_ref( void )
	{
		auto sp_tt = std::dynamic_pointer_cast<data_carrier_non_class<T, MTX_T>>( sp_data_ );
		if ( sp_tt == nullptr ) {
			throw std::bad_cast();
		}
		return sp_tt->data_;
	}

	template <typename U, typename X = T, typename std::enable_if<!std::is_class<X>::value>::type* = nullptr>
	const U& get_ref( void ) const
	{
		auto sp_tt = std::dynamic_pointer_cast<data_carrier_non_class<T, MTX_T>>( sp_data_ );
		if ( sp_tt == nullptr ) {
			throw std::bad_cast();
		}
		return sp_tt->data_;
	}

	std::shared_ptr<data_carrier_base_mtx<MTX_T>> sp_data_;

	template <typename U, typename MTX_U>
	friend class obj_mutex;
};

#endif
