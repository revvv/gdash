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
#ifndef POLYMORPHIC_HPP_INCLUDED
#define POLYMORPHIC_HPP_INCLUDED

#include "config.h"

#include <memory>
#include <utility>
#include <type_traits>

/**
 * Polymorphic value container for one object of any class T,
 * having a clone() member function.
 */
template <typename T>
class Polymorphic {
  private:
    std::unique_ptr<T> object;
    
  public:
    Polymorphic() = default;

    ~Polymorphic() = default;
    
    template <typename U, typename = typename std::enable_if<std::is_base_of<T, U>::value>::type>
    Polymorphic(std::unique_ptr<U> new_object) {
        object = std::move(new_object);
    }
        
    template <typename U, typename = typename std::enable_if<std::is_base_of<T, U>::value>::type>
    Polymorphic(U new_object) {
        object = std::make_unique<U>(std::move(new_object));
    }
    
    Polymorphic(Polymorphic const & other) {
        if (other.object != nullptr)
            object = other.object->clone();
    }
    
    Polymorphic(Polymorphic && other) = default;
    
    Polymorphic & operator=(Polymorphic other) noexcept {
        object.swap(other.object);
        return *this;
    }
    
    operator T & () {
        return *object;
    }
    
    operator T const & () const {
        return *object;
    }
    
    T & get() {
        return *object;
    }
    
    T const & get() const {
        return *object;
    }
};

#endif
