// calculate md5 hash of a file or buffer
export function fileMD5(input: string | Buffer, callback: (err: Error, result: string) => void, progress?: (progress: number, total: number) => void): void;

