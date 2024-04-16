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

import java.util.function.ToDoubleFunction;

/**
 * Represents an object that wraps a native object-to-double callback function.
 */
public final class NativeToDoubleFunction<T> extends NativeCallback implements ToDoubleFunction<T> {
    protected NativeToDoubleFunction(long pointer) {
        super(pointer);
    }

    public native double applyAsDouble(T value);
}
