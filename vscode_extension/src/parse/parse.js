"use strict";
const getBody = require("./get_body").getBody;
const getDefinition = require("./get_definition").getDefinition;
function parse(document, positionLine) {
    const definition = getDefinition(document, positionLine);
    const body = getBody(document, positionLine);
    const code = [definition].concat(body);
    return code;
}

exports.parse = parse;