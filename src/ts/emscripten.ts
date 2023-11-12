/**
 * Contains helpers for dealing with Emscripten module.
 */

export namespace EmHelper
{
    type pointer = number;

    const nullptr: pointer = 0;

    /**
     * Allocate space for and copy buffer into the Emscripten Module's heap
     * memory. Return a pointer to that memory, which must be freed by the user
     * when no longer necessary.
     */
    export
    function allocBuffer(em: EmscriptenModule, buf: ArrayBuffer): pointer
    {
        // allocate memory for buffer
        const ptr = em._malloc(buf.byteLength);
        try {
            // set memory
            em.HEAP8.set(new Uint8Array(buf), ptr);
        }
        catch (err)
        {
            // clean up if error
            em._free(ptr);

            console.error(err);
            return nullptr;
        }

        return ptr;
    }


    /**
     * Cleanup function for allocBuffer
     * @type {[type]}
     */
    export
    function free(em: EmscriptenModule, ptr: pointer): void
    {
        em._free(ptr);
    }


    /**
     * Download and allocate a file into Emscripten module heap memory.
     * The origin must have CORS permission to the endpoint.
     *
     * Please make sure to call `free` on the returned pointer when no longer
     * needing. Failing to do so will cause a memory leak.
     */
    export
    async function allocFile(em: EmscriptenModule, url: string): Promise<pointer>
    {
        try {
            const buf = await fetch(url)
                .then(res => res.arrayBuffer());

            return allocBuffer(em, buf);
        }
        catch(err)
        {
            console.error(err);
            return nullptr;
        }
    }
}
