# A wrapper class that exclusively controls access to class instances

It implements a mutual exclusion control method similar to Mutex in Rust.

It provides a mutual exclusion method similar to Rust's Mutex.
It provides lock/unlock using the Scoped Locking Pattern, similar to C++'s std::unique_lock.
As such, it can be used in combination with condition variables such as std::condition_variable.

Mutilicly controlled objects are referenced via the lock object.
This means that the class instance can only be accessed while the lock object is valid, i.e. only while it is under mutual exclusion.

## obj_mutex\<T, MTX_T\>
`obj_mutex<T, MTX_T>` is a wrapper class for exclusive control of instances of class T.
The mutex used for exclusive control is specified with `MTX_T`.

## Lock classes
The following lock class provides member functions ref(), operator*(), and operator->() to obtain a reference to the exclusively controlled object.
These are the only ways to access an instance of object_mutex\<T, MTX_T\>.
This provides safe access to the class instance, as it is only accessible during the lifetime of the lock object, which is the exclusive control period.

### obj_lock_guard\<OM\>
`obj_lock_guard<OM>` is a lock class for exclusive control of instances of `obj_mutex<...>`.
This class locks an instance of `obj_mutex<...>` and automatically unlocks it when it goes out of scope.
Implemented as a derived class of std::lock_guard.

### obj_unique_lock\<OM\>
`obj_unique_lock<OM>` is a lock class for exclusive control of `obj_mutex<...>` instances.
This class locks an `obj_mutex<...>` instance and automatically unlocks it when it goes out of scope.
You can also change the lock state.
Implemented as a derived class of std::unique_lock.
Because it is derived from std::unique_lock, it can be used in combination with condition variables such as std::condition_variable.

### obj_shared_lock\<OM\>
`obj_shared_lock<OM>` is a lock class for shared locking of `obj_mutex<...>` instances.
This class locks an `obj_mutex<...>` instance and automatically unlocks it when it goes out of scope.
You can also change the state of a shared lock.

Implemented as a derived class of std::shared_lock.

## Usage example
### Example of using obj_lock_guard\<OM\>
```cpp
struct test_t {
    int value;
    test_t( int v )
      : value( v ) {}
};
obj_mutex<test_t> om( test_t(42) ); // Create an obj_mutex that exclusively controls the instance of test_t

int func1( void )
    obj_lock_guard lock( om ); // Create an obj_lock_guard that locks the instance of obj_mutex
    // Since lock locks the instance of obj_mutex, you can access the instance of test_t through om.ref()

    return lock.ref().value; // Access the value of test_t in the exclusively controlled om
}
```
### Example of use in combination with std::condition_variable
```cpp
#include <condition_variable>
#include "object_mutex.hpp" // include object_mutex.hpp
struct test_t {
    int value;
    test_t( int v )
        : value( v ) {}
};
obj_mutex<test_t> om( test_t(42) ); // Create obj_mutex to exclusively control the instance of test_t
std::condition_variable cv; // Condition Variables

void func1( void )
{
    obj_unique_lock locked_obj( om );
    // Creates an obj_unique_lock that locks an instance of obj_mutex equivalent to std::unique_lock<std::mutex>.

    cv.wait( lock, [&locked_obj] -> bool {
        return locked_obj.ref().value == 0; // Condition variable wait condition
    } ); // Waiting on a condition variable
    // Since we are using obj_unique_lock, we access the instance of test_t in the variable om via om.ref() and check the state.
}

void func2( void )
{
    obj_lock_guard locked_obj( om );
    // Creates an obj_lock_guard that locks an instance of obj_mutex equivalent to std::lock_guard<std::mutex>.

    locked_obj.ref().value = 0; // Change the value of an instance of test_t in om
    cv.notify_all(); // Signal to a condition variable
}
```


## Restrictions, etc.
### Uses features from C++17 or later
This code uses features from C++17 or later.
Please build with a compiler for C++17 or later.

* C++17 or later is required because std::scoped_lock is used to implement the assignment operator. It is possible to achieve the function with compilers for C++11 or later using std::lock().
* Inference assistance introduced in C++17 is used to make it easier to declare when using lock classes. If you are using a compiler environment earlier than C++17, explicitly specify the type in the declaration of the lock class.
* It is assumed that you can use std::shared_lock from C++14. If you are compatible with C++11, disable the code related to std::shared_mutex.

### Adaptation to std::scoped_lock is not yet.
When performing multiple mutual exclusion controls simultaneously, `std::scoped_lock` is usefull. But `obj_mutex<T, MTX_T>` is not yet adapted.
After obtaining the lock status of all mutexes using std::lock(), use the adopt function of obj_unique_ptr and/or std::unique_lock to manage the unlock for each mutex.

## License
No license notice is required to use this object_mutex.hpp.
There are no restrictions on modifying, redistributing, or redistributing it after modification.

---------------

# クラスインスタンに対するアクセスを排他制御するラッパークラス

RustのMutexに似た排他制御方法を実現します。

排他制御されたオブジェクトの参照は、lockオブジェクトを通じて行います。
これにより、lockオブジェクトが有効な期間のみ＝排他制御されている期間のみ、クラスインスタンスへのアクセスが可能となります。

C++のstd::unique_lock等の同様にScoped Locking Patternを用いたlock/unlockを実現しています。
そのため、std::condition_variable等の条件変数と組み合わせて使用することができます。

## obj_mutex\<T, MTX_T\>
`obj_mutex<T, MTX_T>`は、クラスTのインスタンスを排他制御するためのラッパークラスです。
排他制御に使用するミューテックスは、`MTX_T`で指定します。

## ロッククラス
下記のロッククラスは、メンバ関数ref()、operator*()、operator->()を提供し、排他制御されたオブジェクトの参照を取得します。
これらが、object_mutex\<T, MTX_T\>のインスタンスへの唯一のアクセス方法です。
これにより、ロックオブジェクトの生存期間＝排他制御されている期間のみアクセス可能となり、クラスインスタンスへの安全なアクセスが提供されます。

### obj_lock_guard\<OM\>
`obj_lock_guard<OM>`は、`obj_mutex<...>`のインスタンスを排他制御するためのロッククラスです。
このクラスは、`obj_mutex<...>`のインスタンスをロックし、スコープを抜けると自動的にアンロックします。
std::lock_guardの派生クラスとして実装されています。

### obj_unique_lock\<OM\>
`obj_unique_lock<OM>`は、`obj_mutex<...>`のインスタンスを排他制御するためのロッククラスです。
このクラスは、`obj_mutex<...>`のインスタンスをロックし、スコープを抜けると自動的にアンロックします。
また、ロックの状態を変更することもできます。
std::unique_lockの派生クラスとして実装されています。
std::unique_lockからの派生蔵であることにより、std::condition_variable等の条件変数と組み合わせて使用することができます。

### obj_shared_lock\<OM\>
`obj_shared_lock<OM>`は、`obj_mutex<...>`のインスタンスを共有ロックするためのロッククラスです。
このクラスは、`obj_mutex<...>`のインスタンスを共有ロックし、スコープを抜けると自動的にアンロックします。
また、共有ロックの状態を変更することもできます。
std::shared_lockの派生クラスとして実装されています。

## 利用例
### obj_lock_guard\<OM\>を利用する例
```cpp
#include "object_mutex.hpp" // object_mutex.hppをインクルード
struct test_t {
    int value;
    test_t( int v )
        : value( v ) {}
};
obj_mutex<test_t> om( test_t(42) ); // test_tのインスタンスを排他制御するobj_mutexを生成

int func1( void )
	obj_lock_guard sut( om ); // obj_mutexのインスタンスをロックするobj_lock_guardを生成
    // sutはobj_mutexのインスタンスをロックしているので、om.ref()を通じて
    // test_tのインスタンスにアクセスできる。

	return sut.ref().value; // 排他制御されているom内のtest_tのvalueにアクセス
}
```
### std::condition_variableと組み合わせて利用する例
```cpp
#include <condition_variable>
#include "object_mutex.hpp" // object_mutex.hppをインクルード
struct test_t {
    int value;
    test_t( int v )
        : value( v ) {}
};
obj_mutex<test_t> om( test_t(42) ); // test_tのインスタンスを排他制御するobj_mutexを生成
std::condition_variable cv; // 条件変数

void func1( void )
{
    obj_unique_lock locked_obj( om );
    // std::unique_lock<std::mutex>に相当するobj_mutexの
    /// インスタンスをロックするobj_unique_lockを生成

    cv.wait( lock, [&locked_obj] -> bool {
        return locked_obj.ref().value == 0; // 条件変数の待機条件
    } ); // 条件変数を待機
    // obj_unique_lockを使用しているので、om.ref()を通じて
    // 変数om内のtest_tのインスタンスにアクセスし、状態を検査する。
}

void func2( void )
{
    obj_lock_guard locked_obj( om );
    // std::lock_guard<std::mutex>に相当するobj_mutexの
    /// インスタンスをロックするobj_lock_guardを生成

    locked_obj.ref().value = 0; // test_tのインスタンスの値を変更
    cv.notify_all(); // 条件変数を通知
}
```

## 制約など
### C++17以降の機能を使用
このコードはC++17以降の機能を使用しています。
C++17以降のコンパイラでビルドしてください。

* 代入演算子の実装にstd::scoped_lockを使用しているため、C++17以降を必要としています。std::lock()を使用して、C++11以降のコンパイラでも機能実現することは可能です。
* ロッククラスを利用する際の宣言が楽になるよう、C++17から導入された推論補助を利用しています。もしC++17以前のコンパイラ環境で利用する場合は、ロッククラスの宣言に型を明示的に指定してください。
* C++14のstd::shared_lockを利用することができる前提となっています。C++11に対応する場合は、std::shared_mutex関連のコードを無効化してください。

### std::scoped_lockへの適応
複数の排他制御を同時に行う場合、`std::scoped_lock`が便利です。しかしながら、`obj_mutex<T, MTX_T>`はまだ、これに対応していません。
代替方法は、std::lock()の利用です。
std::lock()を使ってすべてのmutexのロック状態を取得した後、std::unique_lockやobj_unique_ptrのadopt機能の利用して、それぞれのmutexに対するunlockの管理をしてください。

### std::lock_guard、 std::unique_lock、std::shared_lockの拡張
本質的には、`obj_lock_guard`、`obj_unique_lock`、`obj_shared_lock`は、std::lock_guard、std::unique_lock、std::shared_lockの拡張です。
そのため、std::lock_guard等を特殊化することで、std::lock_guard等を`obj_mutex<T, MTX_T>`適応させることが可能です。
例えば、標準ライブラリのlock_guardクラスのPrimaryテンプレートが、下記のような定義だったとして、
```cpp
namespace std {
    template <typename Mutex>
    class lock_guard {...};
}
```
下記のように、obj_mutex用に特殊化したテンプレートクラスを定義できます。
```cpp
namespace std {
    template <typename T, typename MTX_T>
    class lock_guard<obj_mutex<T, MTX_T>> : public lock_guard<MTX_T> {
    public:
        lock_guard( obj_mutex<T, MTX_T>& om )
        : lock_guard<MTX_T>( *om.mutex() )
        , p_om_( &om )
        {
        }

        T& ref( void ) noexcept
        {
            return p_om_->ref();
        }

    private:
        obj_mutex<T, MTX_T>* p_om_;   //!< pointer to the object mutex being locked
    };
}
```
現在の実装は、ロッククラスを独立したクラスとして用意し、std名前空間での特殊化を行っていませんが、
上記のように、標準ライブラリへの適用が容易な点もobj_mutexの特徴になります。

## ライセンスについて
この object_mutex.hpp を使用するために、ライセンス通知は必要ありません。
変更や再配布、変更後の再配布に対する制約事項はありません。
