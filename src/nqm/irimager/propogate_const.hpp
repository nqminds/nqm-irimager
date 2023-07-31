// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2017 Michael Graham Johnson

#include <type_traits>

template<class Tp_>
class propagate_const : public Tp_
{
public:
    propagate_const() {}

    template<typename ...Args>
    propagate_const(Args... args)
        : Tp_(std::forward<Args...>(args...)) {}

    inline typename Tp_::element_type& operator *()
    {
        return Tp_::operator*();
    }

    inline const typename Tp_::element_type& operator *() const
    {
        return Tp_::operator*();
    }

    inline typename Tp_::element_type* operator ->()
    {
        return Tp_::operator->();
    }

    inline const typename Tp_::element_type* operator ->() const
    {
        return Tp_::operator->();
    }

    inline typename Tp_::element_type* get()
    {
        return Tp_::get();
    }

    inline const typename Tp_::element_type* get() const
    {
        return Tp_::get();
    }
};
