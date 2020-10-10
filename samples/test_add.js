// NOTE: to run this test just do `make test_add` in the root of the project.
// To run all the tests do `make test`

const fs = require("fs");
const assert = require("assert");

WebAssembly
    .instantiate(fs.readFileSync(`${__dirname}/add.wasm`))
    .then((wasm) => {
        assert(wasm.instance.exports.add_u32(10, 20) == 30);
        assert(wasm.instance.exports.add_u64(BigInt(10), BigInt(20)) == BigInt(30));
        console.log("test_add ok");
    });
