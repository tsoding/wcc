// NOTE: to run this test just do `make test_prime` in the root of the project.
// For more information do `make help`.

const fs = require("fs");
const assert = require("assert");

WebAssembly
    .instantiate(fs.readFileSync(`${__dirname}/prime.wasm`))
    .then((wasm) => {
        const prime_numbers = [2, 3, 5, 7, 11, 13, 17, 19, 23,
                               29, 31, 37, 41, 43, 47, 53, 59,
                               61, 67, 71, 73, 79, 83, 89, 97];
        prime_numbers.forEach((x) => {
            assert(wasm.instance.exports.is_prime(x));
            assert(!wasm.instance.exports.is_prime(x * x));
        });
    });
