/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <variant>
#include <utility>

namespace skybolt {

template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
// Deduction guide (C++17)
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

/*!
    Visit items in an std::variant.
    Example usage:
    ```
        std::variant<int, double, std::string> v = "hello";

        skybolt::visit(v,
            [](int i) { std::cout << "int: " << i << '\n'; },
            [](double d) { std::cout << "double: " << d << '\n'; },
            [](const std::string& s) { std::cout << "string: " << s << '\n'; }
        );
    ```
*/
template <typename Variant, typename... Visitors>
decltype(auto) visit(Variant&& var, Visitors&&... visitors) {
    return std::visit(
        Overloaded{std::forward<Visitors>(visitors)...},
        std::forward<Variant>(var)
    );
}

} // namespace skybolt