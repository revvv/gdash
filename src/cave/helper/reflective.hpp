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

#ifndef REFLECTIVE_HPP_INCLUDED
#define REFLECTIVE_HPP_INCLUDED

#include "config.h"

#include <memory>
#include "cave/cavetypes.hpp"

/**
 * A GetterBase class, from which Getter-s for a particular
 * reflective class derive.
 *
 * A particular Getter will store a member pointer to a member
 * of a Reflective class.
 */
class GetterBase {
public:
    unsigned const count;       ///< The size of the array pointed by the prop, or 1

    template <class KLASS, class MEMBERTYPE>
    static std::unique_ptr<GetterBase> create_new(MEMBERTYPE KLASS::*ptr);
    template <class KLASS, class MEMBERTYPE, unsigned COUNT>
    static std::unique_ptr<GetterBase> create_new(MEMBERTYPE(KLASS::*ptr)[COUNT]);

    virtual ~GetterBase() = default;

protected:
    /** Create a GetterBase, which remembers the size or
     *  a member array, or 1. */
    explicit GetterBase(unsigned count) : count(count) {}
};

// pre-declaration of reflective class for GetForType::get(Reflective&)
class Reflective;

/**
 * A Getter for a given type.
 *
 * Has a virtual function, which will return the member
 * of a given reflective object. This will be implemented
 * by a real Getter, which knows the exact type of the
 * Reflective object, too.
 */
template <class MEMBERTYPE>
class GetterForType: public GetterBase {
public:
    /** Return the data member pointed to by the prop.
     * @param o The reflective object.
     * @return The data member, the pointer for which will be stored in the derived object.
     */
    virtual MEMBERTYPE &get(Reflective &o) const = 0;
protected:
    using GetterBase::GetterBase;
};


/**
 * A Getter for a given type of a given class.
 *
 * Stores a member pointer for the member of the class.
 * Implements the virtual function of its base class,
 * the GetterForType.
 *
 * The exact type of the Reflective object is not known;
 * it had to be dropped, as the virtual function is already
 * declared in the base class GetterForType<MEMBERTYPE>.
 * In the implemented get() function we downcast it to
 * its own type, KLASS again. This must be safe; as long
 * as the function is not given another object - some
 * other object that is Reflective, but not exactly
 * of type KLASS. As Getters will be associated to
 * different KLASSes and their member description
 * arrays, this is not likely.
 */
template <class KLASS, class MEMBERTYPE>
class Getter: public GetterForType<MEMBERTYPE> {
private:
    MEMBERTYPE KLASS::* const ptr;      ///< The pointer to the member of the given object (class)
public:
    /**
     * Create a prop, and remember the pointer given.
     * Also remember the count given - which will be the size of the array (if ptr points to a member array),
     * or 1 for single members.
     */
    explicit Getter(MEMBERTYPE KLASS::*ptr, unsigned count)
        :   GetterForType<MEMBERTYPE>(count),
            ptr(ptr) {
    }

    /**
     * The get function a returns with a given member of the reflective object o.
     * The dynamic cast checks that a correct object is given by the caller
     * (not another type of reflective object). It is cannot be static,
     * as the KLASS for which this Getter is instantiated may be a base
     * class of the object, which is not yet reflective (i.e. reflective
     * is inherited later in the hierarchy.)
     */
    MEMBERTYPE & get(Reflective &o) const {
        return dynamic_cast<KLASS &>(o).*ptr;
    }
};


/**
 * Create a new Getter for a class of type KLASS,
 * and its member of type MEMBERTYPE.
 * This function is provided for convenience to automatically guess
 * the class type and member type.
 * The KLASS and MEMBERTYPE are guessed from the member pointer parameter.
 * @param ptr A pointer to a member pointer (of any type) of a class (of any Reflective type).
 * @return A pointer to a newly allocated Getter, cast to GetterBase *.
 */
template <class KLASS, class MEMBERTYPE>
std::unique_ptr<GetterBase> GetterBase::create_new(MEMBERTYPE KLASS::*ptr) {
    return std::make_unique<Getter<KLASS, MEMBERTYPE>>(ptr, 1);
}

/**
 * Create a new Getter for a class of type KLASS,
 * and its member array of COUNT elements of type MEMBERTYPE.
 * This function is provided for convenience to automatically guess
 * the class type and member type.
 * The KLASS and MEMBERTYPE, also the size of the array are guessed from the member pointer parameter.
 * @param ptr A pointer to a member pointer (of any type) of a class (of any Reflective type).
 * @return A pointer to a newly allocated Getter, cast to GetterBase *.
 */
template <class KLASS, class MEMBERTYPE, unsigned COUNT>
std::unique_ptr<GetterBase> GetterBase::create_new(MEMBERTYPE(KLASS::*ptr)[COUNT]) {
    return std::make_unique<Getter<KLASS, MEMBERTYPE[COUNT]>>(ptr, COUNT);
}

/**
 * Properties which can be used in a description of a reflective class data item.
 */
enum ReflectivePropertyFlags {
    GD_ALWAYS_SAVE = 1 << 0,
    GD_DONT_SAVE = 1 << 1,
    GD_DONT_SHOW_IN_EDITOR = 1 << 2,
    GD_SHOW_LEVEL_LABEL = 1 << 3,
    GD_COMPATIBILITY_SETTING = 1 << 4,
    GD_BDCFF_RATIO_TO_CAVE_SIZE = 1 << 5,
    GD_BD_PROBABILITY = 1 << 6,
};

/**
 *  A data structure which describes (usually) one data member in a reflective class.
 */
struct PropertyDescription {
    /// BDCFF identifier
    const char *identifier;
    /// Type of data item
    GdType type;
    /// Flags used during saving and loading, and in the editor
    int flags;
    /// A Getter which can give this data item
    const char *name;
    /// Tooltip for data item, shown in editor.
    std::unique_ptr<GetterBase> prop;
    /// Name in the editor, shown to the user
    const char *tooltip;
    /// Minimum and maximum, for integers.
    int min, max;
};

/**
 * A base class for reflective classes.
 *
 * A requirement for reflective classes is to be able to
 * provide an array of PropertyDescriptions,
 * which describe their data members.
 */
class Reflective {
public:
    virtual PropertyDescription const *get_description_array() const = 0;
    virtual ~Reflective() = default;

    /**
     * Read a data member of type MEMBERTYPE from the given reflective object.
     * The type of the member in question has to be known by the caller,
     * by explicitly giving the template parameter.
     * The dynamic_cast checks if the correct type is given by the caller.
     * @param prop The prop which knows the data member
     */
    template <class MEMBERTYPE>
    MEMBERTYPE &get(std::unique_ptr<GetterBase> const & prop) {
        return dynamic_cast<GetterForType<MEMBERTYPE> const &>(*prop).get(*this);
    }

    /**
     * Const overload for get.
     */
    template <class MEMBERTYPE>
    MEMBERTYPE const & get(std::unique_ptr<GetterBase> const & prop) const {
        return const_cast<Reflective *>(this)->get<MEMBERTYPE>(prop);
    }
};

#endif /* GD_REFLECTIVE_H */
