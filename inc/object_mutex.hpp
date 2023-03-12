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

class obj_mutex_base {
protected:
	static std::mutex assignment_operator_mtx_;
};
inline std::mutex obj_mutex_base::assignment_operator_mtx_;

template <typename T, typename MTX_T = std::mutex>
class obj_mutex : protected obj_mutex_base {
public:
	class single_accessor {
	public:
		/**
		 * @brief get a reference of a target object
		 *
		 * @return T&
		 */
		T& ref( void )
		{
			return *sp_target_obj_;
		}

		/**
		 * @brief move constructor of a new single accessor object
		 *
		 * @param orig
		 */
		single_accessor( single_accessor&& orig )
		  : sp_mtx_( std::move( orig.sp_mtx_ ) )
		  , sp_target_obj_( std::move( orig.sp_target_obj_ ) )
		  , lk_( std::move( orig.lk_ ) )
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
			lk_.unlock();                             // 「unlock -> メモリ参照先の開放」という順番となるようにする。
			lk_            = std::move( orig.lk_ );   // orig.lk_は、すでにlock済み
			sp_mtx_        = std::move( orig.sp_mtx_ );
			sp_target_obj_ = std::move( orig.sp_target_obj_ );
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
			return ( sp_target_obj_ != nullptr );
		}

		~single_accessor()
		{
			// メンバ変数定義と逆順にデストラクタが起動されるため、
			// 自動的に、unlock -> メモリ参照先の開放 という不正アクセスとはならない処理順になる。
		}

	private:
		single_accessor( std::shared_ptr<MTX_T> sp_mtx_arg, std::unique_lock<MTX_T> lk_arg, std::shared_ptr<T> sp_target_obj_arg )
		  : sp_mtx_( std::move( sp_mtx_arg ) )
		  , sp_target_obj_( std::move( sp_target_obj_arg ) )
		  , lk_( std::move( lk_arg ) )
		{
		}

		single_accessor( const single_accessor& )            = delete;
		single_accessor& operator=( const single_accessor& ) = delete;

		std::shared_ptr<MTX_T>  sp_mtx_;
		std::shared_ptr<T>      sp_target_obj_;
		std::unique_lock<MTX_T> lk_;

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
	template <typename U = T, typename std::enable_if<std::is_default_constructible<U>::value>::type* = nullptr>
	obj_mutex( void )
	  : sp_mtx_( std::make_shared<MTX_T>() )
	  , sp_target_obj_( std::make_shared<T>() )
	{
	}

	/**
	 * @brief Move constructor of a new obj mutex object
	 *
	 * This constructor support up-cast from type U to type T.
	 * This means T is base class of U, or T is same type to U.
	 * Or this means U is a derived class from T.
	 *
	 * @tparam U
	 * @tparam std::enable_if<std::is_base_of<T, U>::value>::type
	 * @param orig
	 */
	template <typename U, typename std::enable_if<std::is_base_of<T, U>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_mtx_()
	  , sp_target_obj_()
	{
		// 基底クラスへのアップキャスト
		std::lock_guard<MTX_T> lk( *( orig.sp_mtx_ ) );
		sp_target_obj_ = std::move( orig.sp_target_obj_ );
		sp_mtx_        = std::move( orig.sp_mtx_ );
	}

	/**
	 * @brief Move constructor of a new obj mutex object
	 *
	 * This constructor support down-cast from type U to type T.
	 * This means U is base class of T.
	 * Or this means T is a derived class from U.
	 *
	 * If T is not derived class from U, this throws std::bad_cast exception.
	 *
	 * @tparam U
	 * @tparam std::enable_if<std::is_base_of<U, T>::value && !std::is_same<U, T>::value>::type
	 * @param orig
	 */
	template <typename U, typename std::enable_if<std::is_base_of<U, T>::value && !std::is_same<U, T>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_mtx_()
	  , sp_target_obj_()
	{
		// 派生クラスへのダウンキャスト
		// ダウンキャスト失敗した場合は、move元に情報が残る仕様とする。
		std::lock_guard<MTX_T> lk( *( orig.sp_mtx_ ) );
		sp_target_obj_ = std::dynamic_pointer_cast<T>( orig.sp_target_obj_ );
		if ( ( orig.sp_target_obj_ != nullptr ) && ( sp_target_obj_ == nullptr ) ) {
			// ダウンキャスト失敗
			throw std::bad_cast();
		}
		orig.sp_target_obj_.reset();   // moveコンストラクタであるため、move元を解放する。
		sp_mtx_ = std::move( orig.sp_mtx_ );
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
	template <typename HEADArg, typename U = T, typename... Args,
	          typename std::enable_if<
				  !std::is_same<
					  obj_mutex<U, MTX_T>,
					  typename std::remove_cv<
						  typename std::remove_reference<HEADArg>::type>::type>::value>::type* = nullptr>
	obj_mutex( HEADArg&& headarg, Args&&... args )
	  : sp_mtx_( std::make_shared<MTX_T>() )
	  , sp_target_obj_( std::make_shared<T>( std::forward<HEADArg>( headarg ), std::forward<Args>( args )... ) )
	{
	}

	/**
	 * @brief move assignmment
	 *
	 * @param orig
	 * @return obj_mutex&
	 */
	obj_mutex& operator=( obj_mutex&& orig )
	{
		std::lock_guard<std::mutex> assignment_operator_lk( assignment_operator_mtx_ );
		std::shared_ptr<MTX_T>      sp_tmp_mtx_;
		std::shared_ptr<T>          sp_tmp_target_obj;
		{
			std::lock_guard<MTX_T> orig_side_lk( *( orig.sp_mtx_ ) );
			sp_tmp_target_obj = std::move( orig.sp_target_obj_ );
			sp_tmp_mtx_       = std::move( orig.sp_mtx_ );
		}
		{
			std::lock_guard<MTX_T> this_side_lk( *( sp_mtx_ ) );
			sp_target_obj_ = std::move( sp_tmp_target_obj );
			sp_mtx_        = std::move( sp_tmp_mtx_ );
		}
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
		std::lock_guard<std::mutex> assignment_operator_lk( assignment_operator_mtx_ );
		return ( ( sp_target_obj_ != nullptr ) && ( sp_mtx_ != nullptr ) );
	}

	/**
	 * @brief get single accessor object with up-cast
	 *
	 * if this object is not valid( valid() is flase ), this throws std::logic_error
	 *
	 * @tparam U this U is base type of T
	 * @return obj_mutex<U, MTX_T>::single_accessor
	 */
	template <typename U = T, typename std::enable_if<std::is_base_of<U, T>::value || std::is_same<T, U>::value>::type* = nullptr>
	typename obj_mutex<U, MTX_T>::single_accessor lock_get( void ) const
	{
		// 型Tの基底クラスUへのアップキャストされたsingle_accessorを得る
		if ( ( sp_target_obj_ == nullptr ) || ( sp_mtx_ == nullptr ) ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}

		std::shared_ptr<MTX_T>  sp_mtx_tmp = sp_mtx_;
		std::unique_lock<MTX_T> lk_my( *sp_mtx_tmp );
		if ( sp_mtx_tmp != sp_mtx_ ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}
		std::shared_ptr<U> sp_u( sp_target_obj_ );
		return typename obj_mutex<U, MTX_T>::single_accessor( sp_mtx_, std::move( lk_my ), sp_u );
	}

	/**
	 * @brief get single accessor object with down-cast
	 *
	 * if U is not derived class of internal keeping object, this throws std::bad_cast
	 * if this object is not valid( valid() is flase ), this throws std::logic_error
	 *
	 * @tparam U this U is derived class from T
	 * @tparam std::enable_if<std::is_base_of<T, U>::value && !std::is_same<T, U>::value>::type
	 * @return obj_mutex<U, MTX_T>::single_accessor
	 */
	template <typename U = T, typename std::enable_if<std::is_base_of<T, U>::value && !std::is_same<T, U>::value>::type* = nullptr>
	typename obj_mutex<U, MTX_T>::single_accessor lock_get( void ) const
	{
		// 型Tの派生クラスUへのダウンキャストされたsingle_accessorを得る
		if ( ( sp_target_obj_ == nullptr ) || ( sp_mtx_ == nullptr ) ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}

		std::shared_ptr<MTX_T>  sp_mtx_tmp = sp_mtx_;
		std::unique_lock<MTX_T> lk_my( *sp_mtx_tmp );
		if ( sp_mtx_tmp != sp_mtx_ ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}
		std::shared_ptr<U> sp_u = std::dynamic_pointer_cast<U>( sp_target_obj_ );
		if ( sp_u == nullptr ) {
			throw std::bad_cast();
		}
		return typename obj_mutex<U, MTX_T>::single_accessor( sp_mtx_, std::move( lk_my ), sp_u );
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
	obj_mutex shared_clone( void ) const
	{
		return obj_mutex( sp_mtx_, sp_target_obj_ );
	}

	/**
	 * @brief check this is locked or not
	 *
	 * @return true
	 * @return false
	 */
	bool is_locked( void ) const
	{
		bool ans = sp_mtx_->try_lock();
		if ( ans ) {
			sp_mtx_->unlock();
		}
		return !ans;
	}

private:
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

	obj_mutex( std::shared_ptr<MTX_T> sp_mtx_arg, std::shared_ptr<T> sp_target_obj_arg )
	  : sp_mtx_( std::move( sp_mtx_arg ) )
	  , sp_target_obj_( std::move( sp_target_obj_arg ) )
	{
	}

	std::shared_ptr<MTX_T> sp_mtx_;
	std::shared_ptr<T>     sp_target_obj_;

	template <typename U, typename MTX_U>
	friend class obj_mutex;
};

#endif
