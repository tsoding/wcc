// NOTE: to run this test just do `make test_add` in the root of the project.
// For more information do `make help`.

const fs = require("fs");
const assert = require("assert");

WebAssembly
    .instantiate(fs.readFileSync(`${__dirname}/add.wasm`))
    .then((wasm) => {
        assert.equal(wasm.instance.exports.add_u8(10, 20), 30);
        assert.equal(wasm.instance.exports.add_u32(10, 20), 30);
        assert.equal(wasm.instance.exports.add_u64(10n, 20n), 30n);
        console.log("test_add ok");
    });
