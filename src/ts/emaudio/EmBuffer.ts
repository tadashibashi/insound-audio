import { Audio } from "./AudioModule";

class EmPointer
{
    private m_mod: EmscriptenModule | null;
    private m_ptr: number;
    private m_size: number;

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

    malloc(bytes: number, mod: EmscriptenModule = Audio)
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

    get isLoaded(): boolean
    {
        return !!this.m_ptr;
    }

    get size(): number
    {
        return this.m_size;
    }

    get ptr(): number
    {
        return this.m_ptr;
    }

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
 * Wrapper for WebAssembly memory pointer of a buffer
 */
export
class EmBuffer
{
    private readonly data: EmPointer;

    constructor(buffer?: ArrayBuffer, emModule: EmscriptenModule = Audio)
    {
        this.data = new EmPointer;

        if (buffer && emModule)
            this.alloc(buffer, emModule);

        registry.register(this, this.data, this);
    }

    alloc(buffer: ArrayBuffer, emModule: EmscriptenModule = Audio)
    {
        this.data.allocBuffer(buffer, emModule);
    }

    free()
    {
        this.data.free();
    }

    get isLoaded(): boolean
    {
        return this.data.isLoaded;
    }

    get ptr() {
        return this.data.ptr;
    }

    get size() {
        return this.data.size;
    }
}
