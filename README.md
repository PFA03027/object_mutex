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

## Lock class
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
```cpp
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

## 制約など
### std::scoped_lockへの適応
複数の排他制御を同時に行う場合、`std::scoped_lock`に対してはまだ適応していません。
std::lock()を使ってすべてのmutexのロック状態を取得した後、std::unique_lockや、obj_unique_ptrのadopt機能の利用して、それぞれのmutexに対するunlockの管理をしてください。

## ライセンスについて
この object_mutex.hpp を使用するために、ライセンス通知は必要ありません。
変更や再配布、変更後の再配布に対する制約事項はありません。
