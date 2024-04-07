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

import java.util.function.DoubleFunction;

/**
 * Represents an object that wraps a native object-to-double callback function.
 */
public final class NativeDoubleFunction<R> extends NativeCallback implements DoubleFunction<R> {
    public native R apply(double value);

    public native void close();
}
