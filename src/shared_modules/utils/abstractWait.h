/*
 * Utils abstract wait
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ABSTRACT_WAIT_HPP
#define _ABSTRACT_WAIT_HPP

#include <future>
#include <thread>

class IWait
{
    public:
        virtual ~IWait() = default;
        virtual void set_value() = 0;
        virtual void wait() = 0;
};

class PromiseWaiting final : public IWait
{
        std::promise<void> m_promise;

    public:
        explicit PromiseWaiting() {};

        virtual ~PromiseWaiting() = default;

        virtual void set_value() override
        {
            m_promise.set_value();
        }

        virtual void wait() override
        {
            m_promise.get_future().wait();
        }
};


class BusyWaiting final : public IWait
{
        std::atomic<bool> end;

    public:
        explicit BusyWaiting() : end {false} {};

        virtual ~BusyWaiting() = default;

        virtual void set_value() override
        {
            end = true;
        }

        virtual void wait() override
        {
            while (!end.load())
            {
                std::this_thread::sleep_for(std::chrono::seconds{1});
            }
        }
};


#endif // _ABSTRACT_WAIT_HPP
