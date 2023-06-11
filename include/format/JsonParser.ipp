/*
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-2023 https://github.com/klappdev
 *
 * Permission is hereby  granted, free of charge, to any  person obtaining a copy
 * of this software and associated  documentation files (the "Software"), to deal
 * in the Software  without restriction, including without  limitation the rights
 * to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
 * copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
 * FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
 * AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
 * LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

template<class T,
		class D1 = boost::describe::describe_members<T,
				   boost::describe::mod_public | boost::describe::mod_protected>,
		class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
		class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value && !std::is_union<T>::value>
>	
inline void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const T& t) {
	auto& object = jsonValue.emplace_object();

	boost::mp11::mp_for_each<D1>([&](auto D) {
		object[D.name] = boost::json::value_from(t.*D.pointer);
	});
}


template<class T,
		class D1 = boost::describe::describe_members<T,
				   boost::describe::mod_public | boost::describe::mod_protected>,
		class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
		class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value && !std::is_union<T>::value>
>
inline auto tag_invoke(boost::json::value_to_tag<T> const&, const boost::json::value& jsonValue) -> T {
	const auto& object = jsonValue.as_object();
	T t{};

	auto extractor = []<class E>(const boost::json::object& object, const char* name, E& value) {
		value = boost::json::value_to<E>(object.at(name));
	};

	boost::mp11::mp_for_each<D1>([&](auto D) {
		extractor(object, D.name, t.*D.pointer);
	});

	return t;
}