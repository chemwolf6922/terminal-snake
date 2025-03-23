#pragma once

#include <functional>

namespace Snake
{
    /**
     * @brief It is too much work to have void I and void O. 
     *      Just use int if you don't need any parameters or results.
     * 
     * @tparam I The parameter type when activating the session.
     * @tparam O The result type when the session completes.
     */
    template <typename I, typename O>
    class Session
    {
    public:
        virtual ~Session() = default;

        /**
         * @brief Called when switched from another session.
         * 
         * @param params 
         * @param callback
         */
        void Activate(const I& params, const std::function<void(const O&)>& callback)
        {
            _callback = callback;
            Activate(params);
        }
        virtual void Activate(const I& params) = 0;
        /**
         * @brief Called before switching back to the previous session.
         * 
         */
        virtual void Deactivate() = 0;
        /**
         * @brief Called before switching to another session.
         * @note There is no resume method as the resume operation should 
         *     happen in the callback of the next session.
         * 
         */
        virtual void Pause() = 0;
        /**
         * @note This should only be propagated if this session 
         *      holds the ownership of the sub-sessions.
         * @note This can be called when the session is paused or deactivated.
         * 
         */
        virtual void Close() = 0;
        
    protected:
        template <typename IN, typename ON>
        void SwitchTo(const Session<IN, ON>& next, const IN& params, const std::function<void(const ON&)>& callback)
        {
            Pause();
            next.Activate(params, callback);
        }

        void SwitchBack(const O& result)
        {
            Deactivate();
            _callback(result);
        }

    private:
        std::function<void(const O&)> _callback{nullptr};
    };
}
