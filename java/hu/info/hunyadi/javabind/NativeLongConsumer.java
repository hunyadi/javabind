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

import java.util.function.LongConsumer;

/**
 * Represents an object that wraps a native callback function that accepts
 * a long integer.
 */
public final class NativeLongConsumer extends NativeCallback implements LongConsumer {
    protected NativeLongConsumer(long pointer) {
        super(pointer);
    }

    public native void accept(long value);
}
