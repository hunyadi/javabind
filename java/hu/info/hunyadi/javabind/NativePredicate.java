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

import java.util.function.Predicate;

/**
 * Represents an object that wraps a native object-to-boolean callback function.
 */
public final class NativePredicate<T> extends NativeCallback implements Predicate<T> {
    protected NativePredicate(long pointer) {
        super(pointer);
    }

    public native boolean test(T value);
}
