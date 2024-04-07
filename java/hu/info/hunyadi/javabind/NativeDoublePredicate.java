/**
 * javabind: effective C++ and Java interoperability
 * @see https://github.com/hunyadi/javabind
 *
 * Copyright (c) 2024 Levente Hunyadi
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

package hu.info.hunyadi.javabind;

import java.util.function.DoublePredicate;

/**
 * Represents an object that wraps a native object-to-boolean callback function.
 */
public final class NativeDoublePredicate extends NativeCallback implements DoublePredicate {
    public native boolean test(double value);

    public native void close();
}
