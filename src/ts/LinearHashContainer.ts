// Somewhat space-inefficient but convenient way to hash groups of objects
// into buckets by key
export
class LinearHashContainer<T>
{
    private buckets: T[][];

    // Flattened source of truth
    readonly items: T[];

    constructor(bucketCount: number)
    {
        const buckets = [];
        for (let i = 0; i < bucketCount; ++i)
        {
            buckets.push([]);
        }

        this.buckets = buckets;
        this.items = [];
    }

    /**
     * Put an item into the container.
     *
     * @param item - the item to push
     * @param key  - the key, a percentage (0-1) which will indicate where item
     *               will be placed.
     */
    push(item: T, key: number)
    {
        const buckets = this.buckets;

        this.items.push(item);
        buckets[keyToBucketIndex(key, buckets.length)].push(item);
    }

    remove(item: T, key: number)
    {
        const items = this.items;
        const itemLength = items.length;
        for (let i = 0; i < itemLength; ++i)
        {
            if (items[i] === item)
            {
                items.splice(i, 1);
                break;
            }
        }

        const bucket = this.buckets[keyToBucketIndex(key, this.buckets.length)];
        const bucketLength = bucket.length;
        for (let i = 0; i < bucketLength; ++i)
        {
            if (bucket[i] === item)
            {
                bucket.splice(i, 1);
                break;
            }
        }
    }

    getBucket(key: number)
    {
        return this.buckets[keyToBucketIndex(key, this.buckets.length)];
    }

    /**
     * Clear the container
     */
    clear(): void
    {
        const buckets = this.buckets;
        const bucketsLength = buckets.length;
        for (let i = 0; i < bucketsLength; ++i)
            buckets[i].length = 0;

        this.items.length = 0;
    }


}

// helpers

function keyToBucketIndex(key: number, length: number)
{
    const index = Math.floor(key / (1 / length));
    return Math.min(Math.max(index, 0), length-1);
}
