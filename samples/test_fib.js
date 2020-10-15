// NOTE: to run this test just do `make test_fib` in the root of the project.
// For more information do `make help`.

const fs = require("fs");
const assert = require("assert");

WebAssembly
    .instantiate(fs.readFileSync(`${__dirname}/fib.wasm`))
    .then((wasm) => {
        const fibs = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55];
        for (let i = 0; i < fibs.length; ++i) {
            assert.equal(wasm.instance.exports.fib_u32(i), fibs[i]);
            assert.equal(wasm.instance.exports.fib_u64(BigInt(i)), fibs[i]);
        }
        console.log("test_fib ok");
    });
