// NOTE: to run the tests just do `make test`

const fs = require("fs");
const assert = require("assert");

async function test_add() {
    const wasm = await WebAssembly.instantiate(fs.readFileSync("./samples/add.wasm"));
    assert(wasm.instance.exports.add_u32(10, 20) == 30);
    assert(wasm.instance.exports.add_u64(BigInt(10), BigInt(20)) == BigInt(30));
}

async function test_fib() {
    const fibs = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55];
    const wasm = await WebAssembly.instantiate(fs.readFileSync("./samples/fib.wasm"));
    for (let i = 0; i < fibs.length; ++i) {
        assert.equal(wasm.instance.exports.fib_u32(i), fibs[i]);
        assert.equal(wasm.instance.exports.fib_u64(BigInt(i)), fibs[i]);
    }
}

test_add().then(() => console.log("test_add() ok"));
test_fib().then(() => console.log("test_fib() ok"));
