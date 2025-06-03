#ifndef SPECTER_THREAD_INVOKE_H
#define SPECTER_THREAD_INVOKE_H

/* --------------------------------- Standard ------------------------------- */
#include <QMetaObject>
#include <QThread>
#include <functional>
#include <future>
/* -------------------------------------------------------------------------- */

namespace specter {

/* --------------------------- invokeInObjectThread ------------------------- */

template<typename Obj, typename Method, typename... Args>
auto InvokeInObjectThread(Obj *object, Method method, Args &&...args)
  -> decltype((object->*method)(std::forward<Args>(args)...)) {
  using CleanObj = std::remove_const_t<Obj>;
  using ReturnType = decltype((object->*method)(std::forward<Args>(args)...));

  if (QThread::currentThread() == object->thread()) {
    return (object->*method)(std::forward<Args>(args)...);
  }

  std::promise<ReturnType> promise;
  auto future = promise.get_future();

  auto *invoke_target = const_cast<CleanObj *>(object);

  QMetaObject::invokeMethod(
    invoke_target,
    [object, method, promise = std::move(promise),
     ... args = std::forward<Args>(args)]() mutable {
      try {
        if constexpr (std::is_void_v<ReturnType>) {
          (object->*method)(std::move(args)...);
          promise.set_value();
        } else {
          promise.set_value((object->*method)(std::move(args)...));
        }
      } catch (...) { promise.set_exception(std::current_exception()); }
    },
    Qt::BlockingQueuedConnection);

  return future.get();
}

template<typename Obj, typename Func>
auto InvokeInObjectThread(Obj *object, Func &&func) -> decltype(func()) {
  using CleanObj = std::remove_const_t<Obj>;
  using ReturnType = decltype(func());

  if (QThread::currentThread() == object->thread()) { return func(); }

  std::promise<ReturnType> promise;
  auto future = promise.get_future();

  auto *invoke_target = const_cast<CleanObj *>(object);

  QMetaObject::invokeMethod(
    invoke_target,
    [promise = std::move(promise), func = std::forward<Func>(func)]() mutable {
      try {
        if constexpr (std::is_void_v<ReturnType>) {
          func();
          promise.set_value();
        } else {
          promise.set_value(func());
        }
      } catch (...) { promise.set_exception(std::current_exception()); }
    },
    Qt::BlockingQueuedConnection);

  return future.get();
}

}// namespace specter

#endif// SPECTER_THREAD_INVOKE_H