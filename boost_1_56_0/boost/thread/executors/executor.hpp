// Copyright (C) 2013,2014 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/09 Vicente J. Botet Escriba
//    Adapt to boost from CCIA C++11 implementation

#ifndef BOOST_THREAD_EXECUTORS_EXECUTOR_HPP
#define BOOST_THREAD_EXECUTORS_EXECUTOR_HPP

#include <boost/thread/detail/config.hpp>

#include <boost/thread/detail/delete.hpp>
#include <boost/thread/detail/move.hpp>
#include <boost/thread/executors/work.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{
  namespace executors
  {
  class executor
  {
  public:
    /// type-erasure to store the works to do
    typedef  executors::work work;

    /// executor is not copyable.
    BOOST_THREAD_NO_COPYABLE(executor)
    executor() {}

    /**
     * \b Effects: Destroys the executor.
     *
     * \b Synchronization: The completion of all the closures happen before the completion of the executor destructor.
     */
    virtual ~executor() {};

    /**
     * \b Effects: close the \c executor for submissions.
     * The worker threads will work until there is no more closures to run.
     */
    virtual void close() = 0;

    /**
     * \b Returns: whether the pool is closed for submissions.
     */
    virtual bool closed() = 0;

    /**
     * \b Effects: The specified closure will be scheduled for execution at some point in the future.
     * If invoked closure throws an exception the executor will call std::terminate, as is the case with threads.
     *
     * \b Synchronization: completion of closure on a particular thread happens before destruction of thread's thread local variables.
     *
     * \b Throws: \c sync_queue_is_closed if the thread pool is closed.
     * Whatever exception that can be throw while storing the closure.
     */
    virtual void submit(BOOST_THREAD_RV_REF(work) closure) = 0;

    /**
     * \b Requires: \c Closure is a model of Callable(void()) and a model of CopyConstructible/MoveConstructible.
     *
     * \b Effects: The specified closure will be scheduled for execution at some point in the future.
     * If invoked closure throws an exception the thread pool will call std::terminate, as is the case with threads.
     *
     * \b Synchronization: completion of closure on a particular thread happens before destruction of thread's thread local variables.
     *
     * \b Throws: \c sync_queue_is_closed if the thread pool is closed.
     * Whatever exception that can be throw while storing the closure.
     */

#if defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    template <typename Closure>
    void submit(Closure & closure)
    {
      work w ((closure));
      submit(boost::move(w));
    }
#endif
    void submit(void (*closure)())
    {
      work w ((closure));
      submit(boost::move(w));
    }

    template <typename Closure>
    void submit(BOOST_THREAD_RV_REF(Closure) closure)
    {
      work w = boost::move(closure);
      submit(boost::move(w));
    }

    /**
     * Effects: try to execute one task.
     * Returns: whether a task has been executed.
     * Throws: whatever the current task constructor throws or the task() throws.
     */
    virtual bool try_executing_one() = 0;

    /**
     * \b Requires: This must be called from an scheduled task.
     *
     * \b Effects: reschedule functions until pred()
     */
    template <typename Pred>
    bool reschedule_until(Pred const& pred)
    {
      do {
        //schedule_one_or_yield();
        if ( ! try_executing_one())
        {
          return false;
        }
      } while (! pred());
      return true;
    }
  };


  }
  using executors::executor;
}

#include <boost/config/abi_suffix.hpp>

#endif
