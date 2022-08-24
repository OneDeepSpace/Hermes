

#define BOOST_ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS

#include <chrono>
#include <memory>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <exception>

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include "object_pool.h"

/*
 * ======================================================================
 */

// Functor: wrapped user's task into try-catch exec
template <typename callback>
struct task_t
{
private:
    callback task_;

public:
    explicit task_t(const callback& task)
        : task_(task)
    {}

    ~task_t() { /*std::cout << "~task_t\n";*/ };

    void operator()() const
    {
        try {
            // flush interruption point
            boost::this_thread::interruption_point();
        } catch (const boost::thread_interrupted&){}

        try {
            task_();
        }

        catch (const std::exception& e) {
            std::cerr << "[ERROR][task_t] exception::task_(): " << e.what() << '\n';
        }
        catch (const boost::thread_interrupted&) {
            std::cerr << "[ERROR][task_t] exception::task_(): thread interrupted\n";
        }
        catch (...) {
            std::cerr << "[ERROR][task_t] exception::task_(): unknown exception\n";
        }
    }
}; // task_t

// Functor:  wrapped user's task into task_t that executed by timer with giving delay
template <typename callback>
struct timer_task_t
{
private:
    std::unique_ptr<boost::asio::steady_timer> timer_ {nullptr};
    task_t<callback> task_;

public:
    explicit timer_task_t(
            std::unique_ptr<boost::asio::steady_timer> timer,
            const callback& raw_task)
        : timer_(std::move(timer))
        , task_(raw_task)
    {}

    void operator()(const boost::system::error_code& ec) const
    {
        if (!ec)
            task_();
        else
            std::cerr << "[ERROR][timer_task_t] error while (): " << ec.message() << "\n";
    }
}; // timer_task_t

// function that make wrapped users' task with user's Function object
template <typename callback>
task_t<callback> makeTask(const callback& func)
{
    return task_t<callback>(func);
}

// TaskManager as io_context wrapper
class TaskManager : private boost::noncopyable
{
protected:
    static boost::asio::io_context& get_context()
    {
        static boost::asio::io_context ioc;
        static boost::asio::io_context::work work(ioc); // for debug proposes
        return ioc;
    }

public:

    template <typename Time, typename Callback>
    static void run_delayed(Time duration, const Callback& task)
    {
        std::unique_ptr<boost::asio::steady_timer> timer(
            new boost::asio::steady_timer(
                get_context(), duration
            )
        );

        boost::asio::steady_timer& timer_ref = *timer;

        timer_ref.async_wait(
            timer_task_t<Callback>(
                std::move(timer),
                task
            )
        );
    }

    template <typename Callback>
    static void push_task(const Callback& task)
    {
        get_context().post(makeTask(task));
    }

    static void start()
    {
        get_context().run();
    }

    static void stop()
    {
        get_context().stop();
    }

}; // TaskManager

/*
 * ======================================================================
 */

int func_test()
{
    static int counter = 0;
    ++counter;
    boost::this_thread::interruption_point();
    switch (counter)
    {
    case 3:
        throw std::logic_error("just checking std::logic_error");
    case 10:
        // Эмуляция прерывания потока.
        // Перехват внутри task_wrapped. Выполнение следующих задач продолжится.
        throw boost::thread_interrupted();
    case 90:
        // Остановка task_processor.
        TaskManager::stop();
    }
    return counter;
}


void test_struct_task_t()
{
    for (std::size_t i = 0; i < 100; ++i) {
        TaskManager::push_task(&func_test);
    }
    // Обработка не была начата.
    assert(func_test() == 1);
    // Мы также можем использовать лямбда-выражение в качестве задачи.
    // Асинхронно считаем 2 + 2.
    int sum = 0;
    TaskManager::push_task(
            [&sum]() { sum = 2 + 2; }
    );
    // Обработка не была начата.
    assert(sum == 0);
    assert(sum != 4);
    // Не выбрасывает исключение, но блокирует текущий поток выполнения до тех пор,
    // пока одна из задач не вызовет tasks_processor::stop().
    TaskManager::start();
    assert(func_test() == 91);
}

// some user's raw task as functor
struct test_functor
{
    int &i_;

    explicit test_functor(int &i)
        : i_(i)
    {}

    void operator()() const
    {
        i_ = 1;
        TaskManager::stop();
    }
};

void test_struct_timer_task_t()
{
    const int delay_sec {3000};
    int i {0};

    TaskManager::run_delayed(
        std::chrono::milliseconds(delay_sec),
        test_functor(i)
    );

    TaskManager::run_delayed(
        std::chrono::milliseconds(delay_sec),
        &func_test
    );

    assert(i==0);

    TaskManager::start();
}

/*
 * ======================================================================
 */

void usage()
{
    std::cerr << "\nUsage: ./Timers <num>\n\n"
              << "\t 1 - test pool\n"
              << "\t 2 - test task_t struct\n"
              << "\t 3 - test timer_task_t struct\n\n";
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        usage();
        return 1;
    }

    const auto choise_num = (int)atoi(argv[1]);
    switch(choise_num)
    {
        case 1:
        {
            test_pool();
            break;
        }
        case 2:
        {
            test_struct_task_t();
            break;
        }
        case 3:
        {
            test_struct_timer_task_t();
            break;
        }
        default:
            usage();
            return 1;
    }

    std::cout << "success\n";
    return 0;
}

