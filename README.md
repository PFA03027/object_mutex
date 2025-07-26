# A wrapper class that exclusively controls access to class instances

It implements a mutual exclusion control method similar to Mutex in Rust.

It provides a mutual exclusion method similar to Rust's Mutex.
It provides lock/unlock using the Scoped Locking Pattern, similar to C++'s std::unique_lock.
As such, it can be used in combination with condition variables such as std::condition_variable.

Mutilicly controlled objects are referenced via the lock object.
This means that the class instance can only be accessed while the lock object is valid, i.e. only while it is under mutual exclusion.

### License
No license notice is required to use this object_mutex.hpp.
There are no restrictions on modifying, redistributing, or redistributing it after modification.

---------------

# クラスインスタンに対するアクセスを排他制御するラッパークラス

RustのMutexに似た排他制御方法を実現します。

排他制御されたオブジェクトの参照は、lockオブジェクトを通じて行います。
これにより、lockオブジェクトが有効な期間のみ＝排他制御されている期間のみ、クラスインスタンスへのアクセスが可能となります。

C++のstd::unique_lock等の同様にScoped Locking Patternを用いたlock/unlockを実現しています。
そのため、std::condition_variable等の条件変数と組み合わせて使用することができます。



### ライセンスについて
この object_mutex.hpp を使用するために、ライセンス通知は必要ありません。
変更や再配布、変更後の再配布に対する制約事項はありません。
