#ifndef THREADER_H
#define THREADER_H
#endif 
#include <iostream>
#include <vector>
#include <functional>
#include <list>
#include <mutex>
#include <condition_variable>
#include <bits/c++config.h>
#if defined(_GLIBCXX_HAS_GTHREADS)
#include <bits/gthr.h>
#include <chrono> 
#include <memory> 
#include <tuple> 
#if __cplusplus > 201703L
# include <compare>	
# include <stop_token>
#endif
#ifdef _GLIBCXX_USE_NANOSLEEP
# include <cerrno> 
# include <time.h> 
#endif
#include <bits/functional_hash.h> 
#include <bits/invoke.h>	  
namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION
  class thread
  {
  public:
    struct _State
    {
      virtual ~_State();
      virtual void _M_run() = 0;
    };
    using _State_ptr = unique_ptr<_State>;
    typedef __gthread_t			native_handle_type;
    class id
    {
      native_handle_type	_M_thread;

    public:
      id() noexcept : _M_thread() { }

      explicit
      id(native_handle_type __id) : _M_thread(__id) { }

    private:
      friend class thread;
      friend class hash<id>;

      friend bool
      operator==(id __x, id __y) noexcept;

#if __cpp_lib_three_way_comparison
      friend strong_ordering
      operator<=>(id __x, id __y) noexcept;
#else
      friend bool
      operator<(id __x, id __y) noexcept;
#endif

      template<class _CharT, class _Traits>
	friend basic_ostream<_CharT, _Traits>&
	operator<<(basic_ostream<_CharT, _Traits>& ___out, id __id);
    };

  private:
    id				_M_id;


    template<typename _Tp>
      using __not_same = __not_<is_same<__remove_cvref_t<_Tp>, thread>>;

  public:
    thread() noexcept = default;

    template<typename _Callable, typename... _Args,
	     typename = _Require<__not_same<_Callable>>>
      explicit
      thread(_Callable&& __f, _Args&&... __args)
      {
	static_assert( __is_invocable<typename decay<_Callable>::type,
				      typename decay<_Args>::type...>::value,
	  "std::thread arguments must be invocable after conversion to rvalues"
	  );

#ifdef GTHR_ACTIVE_PROXY
	auto __depend = reinterpret_cast<void(*)()>(&pthread_create);
#else
	auto __depend = nullptr;
#endif
	using _Invoker_type = _Invoker<__decayed_tuple<_Callable, _Args...>>;

	_M_start_thread(_S_make_state<_Invoker_type>(
	      std::forward<_Callable>(__f), std::forward<_Args>(__args)...),
	    __depend);
      }

    ~thread()
    {
      if (joinable())
	std::terminate();
    }

    thread(const thread&) = delete;

    thread(thread&& __t) noexcept
    { swap(__t); }

    thread& operator=(const thread&) = delete;

    thread& operator=(thread&& __t) noexcept
    {
      if (joinable())
	std::terminate();
      swap(__t);
      return *this;
    }

    void
    swap(thread& __t) noexcept
    { std::swap(_M_id, __t._M_id); }

    bool
    joinable() const noexcept
    { return !(_M_id == id()); }

    void
    join();

    void
    detach();

    id
    get_id() const noexcept
    { return _M_id; }

    native_handle_type
    native_handle()
    { return _M_id._M_thread; }
    static unsigned int
    hardware_concurrency() noexcept;

  private:
    template<typename _Callable>
      struct _State_impl : public _State
      {
	_Callable		_M_func;

	template<typename... _Args>
	  _State_impl(_Args&&... __args)
	  : _M_func{{std::forward<_Args>(__args)...}}
	  { }

	void
	_M_run() { _M_func(); }
      };

    void
    _M_start_thread(_State_ptr, void (*)());

    template<typename _Callable, typename... _Args>
      static _State_ptr
      _S_make_state(_Args&&... __args)
      {
	using _Impl = _State_impl<_Callable>;
	return _State_ptr{new _Impl{std::forward<_Args>(__args)...}};
      }
#if _GLIBCXX_THREAD_ABI_COMPAT
  public:
    struct _Impl_base;
    typedef shared_ptr<_Impl_base>	__shared_base_type;
    struct _Impl_base
    {
      __shared_base_type	_M_this_ptr;
      virtual ~_Impl_base() = default;
      virtual void _M_run() = 0;
    };

  private:
    void
    _M_start_thread(__shared_base_type, void (*)());

    void
    _M_start_thread(__shared_base_type);
#endif

  private:
    template<typename _Tuple>
      struct _Invoker
      {
	_Tuple _M_t;

	template<typename>
	  struct __result;
	template<typename _Fn, typename... _Args>
	  struct __result<tuple<_Fn, _Args...>>
	  : __invoke_result<_Fn, _Args...>
	  { };

	template<size_t... _Ind>
	  typename __result<_Tuple>::type
	  _M_invoke(_Index_tuple<_Ind...>)
	  { return std::__invoke(std::get<_Ind>(std::move(_M_t))...); }

	typename __result<_Tuple>::type
	operator()()
	{
	  using _Indices
	    = typename _Build_index_tuple<tuple_size<_Tuple>::value>::__type;
	  return _M_invoke(_Indices());
	}
      };

    template<typename... _Tp>
      using __decayed_tuple = tuple<typename decay<_Tp>::type...>;

  public:

    template<typename _Callable, typename... _Args>
      static _Invoker<__decayed_tuple<_Callable, _Args...>>
      __make_invoker(_Callable&& __callable, _Args&&... __args)
      {
	return { __decayed_tuple<_Callable, _Args...>{
	    std::forward<_Callable>(__callable), std::forward<_Args>(__args)...
	} };
      }
  };

  inline void
  swap(thread& __x, thread& __y) noexcept
  { __x.swap(__y); }

  inline bool
  operator==(thread::id __x, thread::id __y) noexcept
  {

    return __x._M_thread == __y._M_thread;
  }

#if __cpp_lib_three_way_comparison
  inline strong_ordering
  operator<=>(thread::id __x, thread::id __y) noexcept
  { return __x._M_thread <=> __y._M_thread; }
#else
  inline bool
  operator!=(thread::id __x, thread::id __y) noexcept
  { return !(__x == __y); }

  inline bool
  operator<(thread::id __x, thread::id __y) noexcept
  {

    return __x._M_thread < __y._M_thread;
  }

  inline bool
  operator<=(thread::id __x, thread::id __y) noexcept
  { return !(__y < __x); }

  inline bool
  operator>(thread::id __x, thread::id __y) noexcept
  { return __y < __x; }

  inline bool
  operator>=(thread::id __x, thread::id __y) noexcept
  { return !(__x < __y); }
#endif 
  template<>
    struct hash<thread::id>
    : public __hash_base<size_t, thread::id>
    {
      size_t
      operator()(const thread::id& __id) const noexcept
      { return std::_Hash_impl::hash(__id._M_thread); }
    };

  template<class _CharT, class _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& ___out, thread::id __id)
    {
      if (__id == thread::id())
	return ___out << "thread::id of a non-executing thread";
      else
	return ___out << __id._M_thread;
    }


  namespace this_thread
  {
    /// get_id
    inline thread::id
    get_id() noexcept
    {
#ifdef __GLIBC__

      if (!__gthread_active_p())
	return thread::id(1);
#endif
      return thread::id(__gthread_self());
    }
    inline void
    yield() noexcept
    {
#ifdef _GLIBCXX_USE_SCHED_YIELD
      __gthread_yield();
#endif
    }

    void
    __sleep_for(chrono::seconds, chrono::nanoseconds);
    template<typename _Rep, typename _Period>
      inline void
      sleep_for(const chrono::duration<_Rep, _Period>& __rtime)
      {
	if (__rtime <= __rtime.zero())
	  return;
	auto __s = chrono::duration_cast<chrono::seconds>(__rtime);
	auto __ns = chrono::duration_cast<chrono::nanoseconds>(__rtime - __s);
#ifdef _GLIBCXX_USE_NANOSLEEP
	__gthread_time_t __ts =
	  {
	    static_cast<std::time_t>(__s.count()),
	    static_cast<long>(__ns.count())
	  };
	while (::nanosleep(&__ts, &__ts) == -1 && errno == EINTR)
	  { }
#else
	__sleep_for(__s, __ns);
#endif
      }

    template<typename _Clock, typename _Duration>
      inline void
      sleep_until(const chrono::time_point<_Clock, _Duration>& __atime)
      {
#if __cplusplus > 201703L
	static_assert(chrono::is_clock_v<_Clock>);
#endif
	auto __now = _Clock::now();
	if (_Clock::is_steady)
	  {
	    if (__now < __atime)
	      sleep_for(__atime - __now);
	    return;
	  }
	while (__now < __atime)
	  {
	    sleep_for(__atime - __now);
	    __now = _Clock::now();
	  }
      }
  }
#ifdef __cpp_lib_jthread

  class jthread
  {
  public:
    using id = thread::id;
    using native_handle_type = thread::native_handle_type;

    jthread() noexcept
    : _M_stop_source{nostopstate}
    { }

    template<typename _Callable, typename... _Args,
	     typename = enable_if_t<!is_same_v<remove_cvref_t<_Callable>,
					       jthread>>>
      explicit
      jthread(_Callable&& __f, _Args&&... __args)
      : _M_thread{_S_create(_M_stop_source, std::forward<_Callable>(__f),
			    std::forward<_Args>(__args)...)}
      { }

    jthread(const jthread&) = delete;
    jthread(jthread&&) noexcept = default;

    ~jthread()
    {
      if (joinable())
        {
          request_stop();
          join();
        }
    }

    jthread&
    operator=(const jthread&) = delete;

    jthread&
    operator=(jthread&&) noexcept = default;

    void
    swap(jthread& __other) noexcept
    {
      std::swap(_M_stop_source, __other._M_stop_source);
      std::swap(_M_thread, __other._M_thread);
    }

    [[nodiscard]] bool
    joinable() const noexcept
    {
      return _M_thread.joinable();
    }

    void
    join()
    {
      _M_thread.join();
    }

    void
    detach()
    {
      _M_thread.detach();
    }

    [[nodiscard]] id
    get_id() const noexcept
    {
      return _M_thread.get_id();
    }

    [[nodiscard]] native_handle_type
    native_handle()
    {
      return _M_thread.native_handle();
    }

    [[nodiscard]] static unsigned
    hardware_concurrency() noexcept
    {
      return thread::hardware_concurrency();
    }

    [[nodiscard]] stop_source
    get_stop_source() noexcept
    {
      return _M_stop_source;
    }

    [[nodiscard]] stop_token
    get_stop_token() const noexcept
    {
      return _M_stop_source.get_token();
    }

    bool request_stop() noexcept
    {
      return _M_stop_source.request_stop();
    }

    friend void swap(jthread& __lhs, jthread& __rhs) noexcept
    {
      __lhs.swap(__rhs);
    }

  private:
    template<typename _Callable, typename... _Args>
      static thread
      _S_create(stop_source& __ssrc, _Callable&& __f, _Args&&... __args)
      {
	if constexpr(is_invocable_v<decay_t<_Callable>, stop_token,
				    decay_t<_Args>...>)
	  return thread{std::forward<_Callable>(__f), __ssrc.get_token(),
			std::forward<_Args>(__args)...};
	else
	  {
	    static_assert(is_invocable_v<decay_t<_Callable>,
					 decay_t<_Args>...>,
			  "std::thread arguments must be invocable after"
			  " conversion to rvalues");
	    return thread{std::forward<_Callable>(__f),
			  std::forward<_Args>(__args)...};
	  }
      }

    stop_source _M_stop_source;
    thread _M_thread;
  };
#endif
_GLIBCXX_END_NAMESPACE_VERSION
} 
#endif 
class SimpleThreadPool {
public:
    SimpleThreadPool(int numThreads) : numThreads_(numThreads) {
        for (int i = 0; i < numThreads_; ++i) {
            threads_.emplace_back(&SimpleThreadPool::workerThread, this);
        }
    }

    ~SimpleThreadPool() {
        stop();
    }

    void addTask(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.push_back(task);
        }
        condition_.notify_one();
    }

    void stop() {
        stop_ = true;
        condition_.notify_all();
        for (std::thread &thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    void wait() {
        for (std::thread &thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    void workerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = tasks_.front();
                tasks_.pop_front();
            }
            task();
        }
    }

    int numThreads_;
    std::vector<std::thread> threads_;
    std::list<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_ = false;
};
