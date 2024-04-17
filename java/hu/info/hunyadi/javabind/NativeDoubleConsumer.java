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

import java.util.function.DoubleConsumer;

/**
 * Represents an object that wraps a native callback function that accepts
 * a double-precision floating-point number.
 */
public final class NativeDoubleConsumer extends NativeCallback implements DoubleConsumer {
    protected NativeDoubleConsumer(long pointer) {
        super(pointer);
    }

    public native void accept(double value);
}
