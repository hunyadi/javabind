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

import hu.info.hunyadi.javabind.NativeCallback;
import java.util.function.Function;

/**
 * Represents an object that wraps a native object-to-object callback function.
 */
public final class NativeFunction<T, R> extends NativeCallback implements Function<T, R> {
    public native R apply(T t);
    public native void close();
}
