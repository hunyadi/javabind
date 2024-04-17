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

import java.util.function.Consumer;

/**
 * Represents an object that wraps a native callback function that accepts
 * an object.
 */
public final class NativeConsumer<T> extends NativeCallback implements Consumer<T> {
    protected NativeConsumer(long pointer) {
        super(pointer);
    }

    public native void accept(T value);
}
