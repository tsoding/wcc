// NOTE: to run this test just do `make test_rot13_char` in the root of the project.
// For more information do `make help`.

const fs = require("fs");
const assert = require("assert");

WebAssembly
    .instantiate(fs.readFileSync(`${__dirname}/rot13_char.wasm`))
    .then((wasm) => {
        let rot13_char = wasm.instance.exports.rot13_char;
        // values below the range
        for (let i = 0; i < 65; ++i) {
            assert.equal(rot13_char(i), i);
        }
        // values within the range
        for (let i = 65; i <= 90; ++i) {
            // TODO: rot13_char only works with capital letters
            assert.equal(rot13_char(rot13_char(i)), i);
        }
        // values above the range
        for (let i = 91; i < 256; ++i) {
            assert.equal(rot13_char(i), i);
        }
    });
