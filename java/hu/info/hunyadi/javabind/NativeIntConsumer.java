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

import java.util.function.IntConsumer;

/**
 * Represents an object that wraps a native callback function that accepts
 * an integer.
 */
public final class NativeIntConsumer extends NativeCallback implements IntConsumer {
    protected NativeIntConsumer(long pointer) {
        super(pointer);
    }

    public native void accept(int value);
}
