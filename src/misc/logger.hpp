/*
 * Copyright (c) 2007-2018, GDash Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include "config.h"

#include "settings.hpp"
#include "misc/printf.hpp"
#include <string>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>

/// A simple class to store an error message.
/// Each message has a string and a severity level.
struct ErrorMessage {
    enum Severity {
        Debug,
        Info,
        Message,
        Warning,
        Critical,
        Error,
    };

    Severity sev;
    std::string message;

    ErrorMessage(Severity sev_, std::string msg_)
        : sev(sev_), message(std::move(msg_)) {
    }
};

/**
 * A logger class, which is able to record error messages.
 *
 * This class records error messages, stores them in a list.
 * One can get the list by calling get_messages(). A const_iterator
 * is provided to allow the user iterating through the messages.
 *
 * Each logger object remembers if it stores an error message;
 * upon deletion, the destructor will check if there were any
 * messages still unread. If that is the case, an error
 * message is printed to the console. Currently, a GLib
 * log handler is also installed by the misc/logger.
 *
 * The Logger class keeps track of all Logger objects in
 * existence, using the loggers static variable.
 * Global error logging functions are provided for simple
 * usage - they allow callers to use the logging facility
 * without the need of passing the references to a logger
 * object.
 *
 * The global log functions always log errors to the most recently
 * created Logger object. The scheme to use this thing is:
 * @code
 * {             // a code block for the logger object
 *   Logger l;   // create a logger
 *
 *   ...
 *   ... // do things that use the global gd_message() etc
 *   ...
 *
 *   if (!l.empty()) {
 *     // there were errors reported
 *     for (Logger::ConstIterator it=l.begin(); it!=l.end(); ++it)
 *       std::cout << it->message << std::endl;
 *     l.clear();
 *   }
 * }             // logger is deleted here
 * @endcode
 *
 * It is up to the caller to create the logger objects.
 * It is recommended to create a "global" logger object int the
 * main() function, which will receive all log messages, when
 * no other logger objects exist.
 * @code
 * int main()
 * {
 *    Logger global_logger;
 *    ...
 *    ...
 *    ...
 *    global_misc/logger.clear();
 * }
 * @endcode
 */
class Logger {
private:
    static std::vector<Logger *> loggers;
public:
    typedef std::vector<ErrorMessage> Container;
    typedef Container::const_iterator ConstIterator;

private:
    bool ignore;            ///< if true, all errors reported are ignored
    Container messages;     ///< list of messages
    std::string context;    ///< context which is added to all messages


    void set_context(std::string new_context = std::string());
    std::string const &get_context() const;

public:
    Logger(bool ignore_ = false);
    Logger(Logger const &) = delete;
    Logger& operator=(Logger const &) = delete;
    ~Logger();
    void clear();
    bool empty() const;
    Container const &get_messages() const;
    std::string get_messages_in_one_string() const;
    void log(ErrorMessage::Severity sev, std::string const &message);
    
    static Logger &get_active_logger() {
        assert(!Logger::loggers.empty());
        return *Logger::loggers.back();
    }

    template <typename ... ARGS>
    friend void log(ErrorMessage::Severity sev, std::string const &message, ARGS const & ... args);

    friend class SetLoggerContextForFunction;
};


template <typename ... ARGS>
void log(ErrorMessage::Severity sev, std::string const &message, ARGS const & ... args) {
    /* check if at least one logger exists */
    if (Logger::loggers.empty()) {
        std::cerr << message;
        return;
    }
    
    if (sizeof...(args) == 0) {
        Logger::get_active_logger().log(sev, message);
    } else {
        Logger::get_active_logger().log(sev, Printf(message, args...));
    }
}


/**
 * @brief Convenience function to log a critical message.
 */
template <typename ... ARGS>
void gd_critical(const char *message, ARGS const & ... args) {
    log(ErrorMessage::Critical, message, args...);
}

/**
 * @brief Convenience function to log a warning message.
 */
template <typename ... ARGS>
void gd_warning(const char *message, ARGS const & ... args) {
    log(ErrorMessage::Warning, message, args...);
}

/**
 * @brief Convenience function to log a normal message.
 */
template <typename ... ARGS>
void gd_message(const char *message, ARGS const & ... args) {
    log(ErrorMessage::Message, message, args...);
}

template <typename ... ARGS>
void gd_debug(const char *message, ARGS const & ... args) {
    if (gd_param_debug)
        log(ErrorMessage::Debug, message, args...);
}

/** Set the logger context for the lifetime of the object.
 * To be used to set the context while inside a function or a statement block:
 *
 * @code
 * {
 *    SetLoggerContextForFunction slc(get_active_logger(), "Reading file");
 *
 *    // Log messages generated here will have the context
 *
 * } // slc object goes out of scope here, context is reset to original value
 * @endcode
*/
class SetLoggerContextForFunction {
    Logger &l;
    std::string orig_context;
public:
    SetLoggerContextForFunction(std::string const &context, Logger &l = Logger::get_active_logger())
        : l(l) {
        orig_context = l.get_context();
        l.set_context(orig_context.empty() ? context : (orig_context + ", " + context));
    }
    ~SetLoggerContextForFunction() {
        l.set_context(orig_context);
    }
};


#endif /* GD_LOGGER */
