// NOTE: to run the tests just do `make test`

const fs = require("fs");
const assert = require("assert");

async function test_add() {
    const wasm = await WebAssembly.instantiate(fs.readFileSync("./samples/add.wasm"));
    assert(wasm.instance.exports.add(10, 20) == 30);
}

async function test_fib() {
    const wasm = await WebAssembly.instantiate(fs.readFileSync("./samples/fib.wasm"));
    const fibs = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55];
    for (let i = 0; i < fibs.length; ++i) {
        assert.equal(wasm.instance.exports.fib(i), fibs[i]);
    }
}

test_add().then(() => console.log("test_add() ok"));
test_fib().then(() => console.log("test_fib() ok"));
