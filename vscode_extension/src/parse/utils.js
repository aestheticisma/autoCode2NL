"use strict";

function getIndentation(line) {
    const whiteSpaceMatches = line.match(/^[^\S\r]+/);
    if (whiteSpaceMatches == undefined) {
        return "";
    }
    return whiteSpaceMatches[0];
}

function blankLine(line) {
    return line.match(/[^\s]/) == undefined;
}

function preprocessLines(lines) {
    return lines
        .map(line => line.trim())
        .filter((line) => !line.startsWith("#"));
}

function indentationOf(line) {
    return getIndentation(line).length;
}

exports.getIndentation = getIndentation;
exports.blankLine = blankLine;
exports.preprocessLines = preprocessLines;
exports.indentationOf = indentationOf;