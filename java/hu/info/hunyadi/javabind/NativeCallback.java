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

import java.lang.ref.Cleaner;

/**
 * Represents an object that wraps a native callback function.
 */
public abstract class NativeCallback implements AutoCloseable {
    /**
     * Holds an opaque reference to an object that exists in the native code
     * execution context.
     */
    private final long nativePointer;

    /**
     * Deallocates native resources associated with the Java host object.
     */
    private final Cleaner.Cleanable cleanable;

    /**
     * Deallocates native resources when the Java host object becomes phantom
     * reachable.
     */
    private static final Cleaner cleaner = Cleaner.create();

    private static class Deallocator implements Runnable {
        private long pointer;

        public Deallocator(long pointer) {
            this.pointer = pointer;
        }

        @Override
        public void run() {
            NativeCallback.deallocate(this.pointer);
        }
    }

    /**
     * Creates this callback object by wrapping a native callback pointer.
     */
    protected NativeCallback(long pointer) {
        nativePointer = pointer;
        cleanable = cleaner.register(this, new Deallocator(pointer));
    }

    @Override
    public void close() {
        cleanable.clean();
    }

    /**
     * Disposes of objects allocated in the native code execution context.
     */
    private static native void deallocate(long pointer);
}
