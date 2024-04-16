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

import java.util.function.LongFunction;

/**
 * Represents an object that wraps a native long-to-object callback function.
 */
public final class NativeLongFunction<R> extends NativeCallback implements LongFunction<R> {
    protected NativeLongFunction(long pointer) {
        super(pointer);
    }

    public native R apply(long value);
}
