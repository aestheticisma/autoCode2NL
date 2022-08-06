"use strict";
const blankLine = require("./utils").blankLine
const preprocessLines = require("./utils").preprocessLines

function getDefinition(document, linePosition) {
    const precedingLines = getPrecedingLines(document, linePosition);
    const precedingText = precedingLines.join(" ");
    // Don't parse if the preceding line is blank
    const precedingLine = precedingLines[precedingLines.length - 1];
    if (precedingLine == undefined || blankLine(precedingLine)) {
        return "";
    }
    const pattern = /\b(((async\s+)?\s*def)|\s*class)\b/g;
    // Get starting index of last def match in the preceding text
    let index;
    while (pattern.test(precedingText)) {
        index = pattern.lastIndex - RegExp.lastMatch.length;
    }
    if (index == undefined) {
        return "";
    }
    const lastFunctionDef = precedingText.slice(index);
    return lastFunctionDef.trim();
}

function getPrecedingLines(document, linePosition) {
    const lines = document.split("\n");
    const rawPrecedingLines = lines.slice(0, linePosition);
    const precedingLines = preprocessLines(rawPrecedingLines);
    return precedingLines;
}

exports.getDefinition = getDefinition;
