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

import java.util.function.IntFunction;

/**
 * Represents an object that wraps a native int-to-object callback function.
 */
public final class NativeIntFunction<R> extends NativeCallback implements IntFunction<R> {
    protected NativeIntFunction(long pointer) {
        super(pointer);
    }

    public native R apply(int value);
}
