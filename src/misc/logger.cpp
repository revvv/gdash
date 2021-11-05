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

#include "config.h"

#include <vector>
#include <glib.h>
#include <iostream>

#include "misc/logger.hpp"

std::vector<Logger *> Logger::loggers;

static char severity_char(ErrorMessage::Severity sev) {
    switch (sev) {
        case ErrorMessage::Error:
            return 'E';
        case ErrorMessage::Warning:
            return 'W';
        case ErrorMessage::Debug:
            return 'D';
        case ErrorMessage::Info:
            return 'I';
        case ErrorMessage::Message:
            return 'M';
        case ErrorMessage::Critical:
            return 'C';
    }
    return '?';
}

static ErrorMessage::Severity severity_glog(int flag) {
    if (flag & G_LOG_LEVEL_ERROR) return ErrorMessage::Error;
    if (flag & G_LOG_LEVEL_CRITICAL) return ErrorMessage::Critical;
    if (flag & G_LOG_LEVEL_WARNING) return ErrorMessage::Warning;
    if (flag & G_LOG_LEVEL_MESSAGE) return ErrorMessage::Message;
    if (flag & G_LOG_LEVEL_INFO) return ErrorMessage::Info;
    if (flag & G_LOG_LEVEL_DEBUG) return ErrorMessage::Debug;
    return ErrorMessage::Warning;
}

std::ostream &operator<<(std::ostream &os, const ErrorMessage &em) {
    os << severity_char(em.sev) << ':' << em.message;
    return os;
}

/// GLib log func. This is used to record the log messages from gtk and
/// glib as well.
static void log_func(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) {
    log(severity_glog(log_level), message);
    /* also call default handler to print to console; but with processed string */
    g_log_default_handler(log_domain, log_level, message, user_data);
}


/// Creates a new misc/logger.
/// Adds it to the static list of loggers.
Logger::Logger(bool ignore_)
    :
    ignore(ignore_) {
    /* if this is the first logger created */
    if (loggers.empty())
        g_log_set_default_handler(log_func, NULL);
    /* add this logger to list of loggers, so we always know which was last */
    loggers.push_back(this);
}

/// Destruct a misc/logger.
/// Removes it from the static list of loggers.
Logger::~Logger() {
    if (!empty()) {
        std::cerr << "Messages left in logger!" << std::endl;
        for (Container::const_iterator it = messages.begin(); it != messages.end(); ++it)
            std::cerr << "  " << *it << std::endl;
    }
    assert(loggers.back() == this);
    loggers.pop_back();
    if (loggers.empty())
        g_log_set_default_handler(g_log_default_handler, NULL);
}

/// Clears the logger to empty. (No messages.)
void Logger::clear() {
    messages.clear();
}

/// Returns if there are no error messages stored in the misc/logger.
/// @return true, if empty
bool Logger::empty() const {
    return messages.empty();
}

void Logger::set_context(std::string new_context) {
    context = std::move(new_context);
}

std::string const &Logger::get_context() const {
    return context;
}

/// Get the messages.
/// @return Container of messages.
Logger::Container const &Logger::get_messages() const {
    return messages;
}

/// Get messages in one string - many lines.
/// @return String of messages - each message on its own line.
std::string Logger::get_messages_in_one_string() const {
    std::string s;
    bool first = true;
    for (ConstIterator it = messages.begin(); it != messages.end(); ++it) {
        if (first) {
            s += '\n';
            first = false;
        }
        s += it->message;
    }
    return s;
}

/// Log a new message.
/// If a context is set, it will also be noted.
void Logger::log(ErrorMessage::Severity sev, std::string const &message) {
    // if ignoring messages, do nothing
    if (ignore)
        return;

    if (context == "")
        messages.push_back(ErrorMessage(sev, message));
    else
        messages.push_back(ErrorMessage(sev, context + ": " + message));
    std::cerr << messages.back() << std::endl;
}
