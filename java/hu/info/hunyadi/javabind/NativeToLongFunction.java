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

import java.util.function.ToLongFunction;

/**
 * Represents an object that wraps a native object-to-long callback function.
 */
public final class NativeToLongFunction<T> extends NativeCallback implements ToLongFunction<T> {
    public native long applyAsLong(T value);

    public native void close();
}
