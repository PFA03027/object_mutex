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

template <typename T, typename MTX_T = std::mutex>
class obj_mutex {
public:
	class single_accessor {
	public:
		T& ref( void )
		{
			return *sp_target_obj_;
		}

		single_accessor( single_accessor&& orig )
		  : sp_mtx_( std::move( orig.sp_mtx_ ) )
		  , sp_target_obj_( std::move( orig.sp_target_obj_ ) )
		  , lk_( std::move( orig.lk_ ) )
		{
		}

		single_accessor& operator=( single_accessor&& orig )
		{
			lk_.unlock();   // 「unlock -> メモリ参照先の開放」という順番となるようにする。
			lk_            = std::move( orig.lk_ );
			sp_mtx_        = std::move( orig.sp_mtx_ );
			sp_target_obj_ = std::move( orig.sp_target_obj_ );
			return *this;
		}

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
		single_accessor( std::shared_ptr<MTX_T> sp_mtx_arg, std::shared_ptr<T> sp_target_obj_arg )
		  : sp_mtx_( std::move( sp_mtx_arg ) )
		  , sp_target_obj_( std::move( sp_target_obj_arg ) )
		  , lk_( *sp_mtx_ )
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
	template <typename U = T, typename std::enable_if<std::is_default_constructible<U>::value>::type* = nullptr>
	obj_mutex( void )
	  : sp_mtx_( std::make_shared<MTX_T>() )
	  , sp_target_obj_( std::make_shared<T>() )
	{
	}

	template <typename U, typename std::enable_if<std::is_base_of<T, U>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_mtx_( std::move( orig.sp_mtx_ ) )
	  , sp_target_obj_( std::move( orig.sp_target_obj_ ) )
	{
		// 基底クラスへのアップキャスト
	}

	template <typename U, typename std::enable_if<std::is_base_of<U, T>::value && !std::is_same<U, T>::value>::type* = nullptr>
	obj_mutex( obj_mutex<U, MTX_T>&& orig )
	  : sp_mtx_( std::move( orig.sp_mtx_ ) )
	  , sp_target_obj_( std::dynamic_pointer_cast<T>( orig.sp_target_obj_ ) )
	{
		// 派生クラスへのダウンキャスト
		// ダウンキャスト失敗した場合は、move元に情報が残る仕様とする。
		if ( ( orig.sp_target_obj_ != nullptr ) && ( sp_target_obj_ == nullptr ) ) {
			// ダウンキャスト失敗
			throw std::bad_cast();
		}
		orig.sp_target_obj_.reset();   // moveコンストラクタであるため、move元を解放する。
	}

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

	bool valid( void ) const
	{
		return ( sp_target_obj_ != nullptr );
	}

	template <typename U = T>
	typename obj_mutex<U, MTX_T>::single_accessor lock_get( void ) const
	{
		if ( sp_target_obj_ == nullptr ) {
			throw std::logic_error( "obj_mutex is empty. has been moved ?" );
		}
		std::shared_ptr<U> sp_u = std::dynamic_pointer_cast<U>( sp_target_obj_ );
		if ( sp_u == nullptr ) {
			throw std::bad_cast();
		}
		return typename obj_mutex<U, MTX_T>::single_accessor( sp_mtx_, sp_u );
	}

	template <typename U = T, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
	obj_mutex<U, MTX_T> clone( void ) const
	{
		return obj_mutex<U, MTX_T>( lock_get().ref() );
	}

	obj_mutex shared_clone( void ) const
	{
		return obj_mutex( sp_mtx_, sp_target_obj_ );
	}

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
