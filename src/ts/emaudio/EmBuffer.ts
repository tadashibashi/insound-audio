/**
 * Wrapper around allocated memory in an Emscripten module.
 */
class EmPointer
{
    // EmscriptenModule object, saved for call to free
    private m_mod: EmscriptenModule | null;
    // Pointer to module memory
    private m_ptr: number;
    // Size of the data allocated
    private m_size: number;

    /**
     * Allocate memory and fill it with buffer data.
     *
     * Make sure to free the memory by calling `EmPointer#free` when you are
     * done with it.
     *
     * @param {ArrayBuffer}      buffer [description]
     * @param {EmscriptenModule} mod    [description]
     */
    allocBuffer(buffer: ArrayBuffer, mod: EmscriptenModule)
    {
        if (buffer.byteLength <= 0)
            throw Error("Cannot allocate 0 bytes");

        const ptr = mod._malloc(buffer.byteLength);
        try {
            mod.HEAP8.set(new Uint8Array(buffer), ptr);
            this.free();
        }
        catch(e)
        {
            mod._free(ptr);
            throw e;
        }

        this.m_ptr = ptr;
        this.m_size = buffer.byteLength;
        this.m_mod = mod;
    }

    /**
     * Allocate memory without filling it. Manually fill it by setting the
     * EmscriptenModule heap object.
     *
     * Make sure to free the memory by calling `EmPointer#free` when you are
     * done with it.
     *
     * @param bytes - number of bytes to allocate
     * @param mod   - the Emscripten module to allocate memory in
     */
    malloc(bytes: number, mod: EmscriptenModule)
    {
        if (bytes <= 0)
            throw Error("Cannot allocate 0 bytes");
        const ptr = mod._malloc(bytes);
        try {
            this.free();
        }
        catch(e)
        {
            mod._free(ptr);
            throw e;
        }

        this.m_ptr = ptr;
        this.m_size = bytes;
        this.m_mod = mod;
    }

    /**
     * Free the allocated data. Safe to call if none was allocated, or if
     * already freed.
     */
    free()
    {
        if (this.m_ptr !== 0 && this.m_mod)
        {
            this.m_mod._free(this.m_ptr);
            this.m_size = 0;
            this.m_mod = null;
            this.m_ptr = 0;
        }
    }

    /**
     * Whether data has been allocated.
     */
    get isNull(): boolean
    {
        return !this.m_ptr;
    }

    /**
     * Size of currently allocated data in bytes. `0` if no memory was
     * allocated.
     */
    get size(): number
    {
        return this.m_size;
    }

    /**
     * Pointer to currently allocatd data. `0` if no memory was allocated.
     */
    get ptr(): number
    {
        return this.m_ptr;
    }

    /**
     * Emscripten module associated with allocated memory. `null` if none was
     * allocated.
     */
    get mod(): EmscriptenModule
    {
        return this.m_mod;
    }
}

const registry = new FinalizationRegistry<EmPointer>((heldValue) => {
    if (heldValue instanceof EmPointer)
    {
        heldValue.free();
    }
    else
    {
        console.error("Failed to finalize MultiLayeredTrack: Wrong type was " +
            "passed to the FinalizationRegistry.");
    }
});

/**
 * Wrapper around a pointer to EmscriptenModule memory.
 *
 * Automatically cleans up data via a FinalizationRegistry, but since it is
 * not guaranteed to be called, an explicit call to `EmBuffer#free` is highly
 * recommended.
 */
export
class EmBuffer
{
    // Internal pointer object
    private readonly data: EmPointer;

    constructor()
    {
        this.data = new EmPointer;
        registry.register(this, this.data, this);
    }

    /**
     * Allocate ArrayBuffer into Emscripten module memory
     *
     * @param buffer   - data buffer to copy
     * @param emModule - module to allocate the memory into
     */
    alloc(buffer: ArrayBuffer, emModule: EmscriptenModule): void
    {
        this.data.allocBuffer(buffer, emModule);
    }

    /**
     * Free allocated data. Safe to call if no data is currently allocated.
     */
    free(): void
    {
        this.data.free();
    }

    /**
     * Whether pointer is pointing to null
     */
    get isNull(): boolean
    {
        return this.data.isNull;
    }

    /**
     * The raw pointer.
     */
    get ptr(): number {
        return this.data.ptr;
    }

    /**
     * Size of allocated memory.
     */
    get size() {
        return this.data.size;
    }

    /**
     * The emscripten module object data is allocated in.
     */
    get mod() {
        return this.data.mod;
    }
}
