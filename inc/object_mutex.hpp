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

template <typename T, typename MTX_T = std::mutex>
class obj_mutex {
private:
	struct obj_cnt {
		mutable MTX_T mtx_;
		T             target_obj_;

		template <typename... Args>
		obj_cnt( Args... args )
		  : mtx_()
		  , target_obj_( std::forward<Args>( args )... )
		{
		}
	};

public:
	class single_accessor {
	public:
		T& ref( void )
		{
			return sp_target_obj_->target_obj_;
		}

		single_accessor( single_accessor&& orig )
		  : sp_target_obj_( std::move( orig.sp_target_obj_ ) )
		  , lk_( std::move( orig.lk_ ) )
		{
		}

		single_accessor& operator=( single_accessor&& orig )
		{
			lk_.unlock();   // unlock -> メモリ参照先の開放という順番となるようにする。
			sp_target_obj_ = std::move( orig.sp_target_obj_ );
			lk_            = std::move( orig.lk_ );
			return *this;
		}

		~single_accessor()
		{
			// メンバ変数定義と逆順にデストラクタが起動されるため、
			// 自動的に、unlock -> メモリ参照先の開放 という不正アクセスとはならない処理順になる。
		}

	private:
		single_accessor( std::shared_ptr<obj_cnt> sp_target_obj_arg )
		  : sp_target_obj_( sp_target_obj_arg )
		  , lk_( sp_target_obj_->mtx_ )
		{
		}

		single_accessor( const single_accessor& )            = delete;
		single_accessor& operator=( const single_accessor& ) = delete;

		std::shared_ptr<obj_cnt> sp_target_obj_;
		std::unique_lock<MTX_T>  lk_;

		friend obj_mutex;
	};

	template <typename... Args>
	obj_mutex( Args... args )
	  : sp_target_obj_( std::make_shared<obj_cnt>( std::forward<Args>( args )... ) )
	{
	}

	single_accessor lock_get( void )
	{
		return single_accessor( sp_target_obj_ );
	}

	bool TEST_is_locked( void )
	{
		bool ans = sp_target_obj_->mtx_.try_lock();
		if ( ans ) {
			sp_target_obj_->mtx_.unlock();
		}
		return !ans;
	}

private:
	std::shared_ptr<obj_cnt> sp_target_obj_;
};

#endif
